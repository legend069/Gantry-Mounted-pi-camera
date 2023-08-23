/**
 * Copyright (c) 2022 Paul-Louis Ageneau
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "pollservice.hpp"
#include "internals.hpp"

#if RTC_ENABLE_WEBSOCKET

#include <algorithm>
#include <cassert>

namespace rtc::impl {

using namespace std::chrono_literals;
using std::chrono::duration_cast;
using std::chrono::milliseconds;

PollService &PollService::Instance() {
	static PollService *instance = new PollService;
	return *instance;
}

PollService::PollService() : mStopped(true) {}

PollService::~PollService() {}

void PollService::start() {
	mSocks = std::make_unique<SocketMap>();
	mInterrupter = std::make_unique<PollInterrupter>();
	mStopped = false;
	mThread = std::thread(&PollService::runLoop, this);
}

void PollService::join() {
	std::unique_lock lock(mMutex);
	if (std::exchange(mStopped, true))
		return;

	lock.unlock();

	mInterrupter->interrupt();
	mThread.join();

	mSocks.reset();
	mInterrupter.reset();
}

void PollService::add(socket_t sock, Params params) {
	std::unique_lock lock(mMutex);
	assert(params.callback);

	PLOG_VERBOSE << "Registering socket in poll service, direction=" << params.direction;
	auto until = params.timeout ? std::make_optional(clock::now() + *params.timeout) : nullopt;
	assert(mSocks);
	mSocks->insert_or_assign(sock, SocketEntry{std::move(params), std::move(until)});

	assert(mInterrupter);
	mInterrupter->interrupt();
}

void PollService::remove(socket_t sock) {
	std::unique_lock lock(mMutex);

	PLOG_VERBOSE << "Unregistering socket in poll service";
	assert(mSocks);
	mSocks->erase(sock);

	assert(mInterrupter);
	mInterrupter->interrupt();
}

void PollService::prepare(std::vector<struct pollfd> &pfds, optional<clock::time_point> &next) {
	std::unique_lock lock(mMutex);
	pfds.resize(1 + mSocks->size());
	next.reset();

	auto it = pfds.begin();
	mInterrupter->prepare(*it++);
	for (const auto &[sock, entry] : *mSocks) {
		it->fd = sock;
		switch (entry.params.direction) {
		case Direction::In:
			it->events = POLLIN;
			break;
		case Direction::Out:
			it->events = POLLOUT;
			break;
		default:
			it->events = POLLIN | POLLOUT;
			break;
		}
		if (entry.until)
			next = next ? std::min(*next, *entry.until) : *entry.until;

		++it;
	}
}

void PollService::process(std::vector<struct pollfd> &pfds) {
	std::unique_lock lock(mMutex);

	auto it = pfds.begin();
	mInterrupter->process(*it++);
	while (it != pfds.end()) {
		socket_t sock = it->fd;
		auto jt = mSocks->find(sock);
		if (jt != mSocks->end()) {
			try {
				auto &entry = jt->second;
				const auto &params = entry.params;

				if (it->revents & POLLNVAL || it->revents & POLLERR) {
					PLOG_VERBOSE << "Poll error event";
					auto callback = std::move(params.callback);
					mSocks->erase(sock);
					callback(Event::Error);

				} else if (it->revents & POLLIN || it->revents & POLLOUT) {
					entry.until = params.timeout
					                  ? std::make_optional(clock::now() + *params.timeout)
					                  : nullopt;

					auto callback = params.callback;
					if (it->revents & POLLIN) {
						PLOG_VERBOSE << "Poll in event";
						callback(Event::In);
					}
					if (it->revents & POLLOUT) {
						PLOG_VERBOSE << "Poll out event";
						callback(Event::Out);
					}

				} else if (entry.until && clock::now() >= *entry.until) {
					PLOG_VERBOSE << "Poll timeout event";
					auto callback = std::move(params.callback);
					mSocks->erase(sock);
					callback(Event::Timeout);
				}

			} catch (const std::exception &e) {
				PLOG_WARNING << e.what();
				mSocks->erase(sock);
			}
		}

		++it;
	}
}

void PollService::runLoop() {
	try {
		PLOG_DEBUG << "Poll service started";
		assert(mSocks);

		std::vector<struct pollfd> pfds;
		optional<clock::time_point> next;
		while (!mStopped) {
			prepare(pfds, next);

			int ret;
			do {
				int timeout;
				if (next) {
					auto msecs = duration_cast<milliseconds>(
					    std::max(clock::duration::zero(), *next - clock::now() + 1ms));
					PLOG_VERBOSE << "Entering poll, timeout=" << msecs.count() << "ms";
					timeout = static_cast<int>(msecs.count());
				} else {
					PLOG_VERBOSE << "Entering poll";
					timeout = -1;
				}

				ret = ::poll(pfds.data(), static_cast<nfds_t>(pfds.size()), timeout);

				PLOG_VERBOSE << "Exiting poll";

			} while (ret < 0 && (sockerrno == SEINTR || sockerrno == SEAGAIN));

			if (ret < 0)
				throw std::runtime_error("poll failed, errno=" + std::to_string(sockerrno));

			process(pfds);
		}
	} catch (const std::exception &e) {
		PLOG_FATAL << "Poll service failed: " << e.what();
	}

	PLOG_DEBUG << "Poll service stopped";
}

std::ostream &operator<<(std::ostream &out, PollService::Direction direction) {
	const char *str;
	switch (direction) {
	case PollService::Direction::In:
		str = "in";
		break;
	case PollService::Direction::Out:
		str = "out";
		break;
	case PollService::Direction::Both:
		str = "both";
		break;
	default:
		str = "unknown";
		break;
	}
	return out << str;
}

} // namespace rtc::impl

#endif
