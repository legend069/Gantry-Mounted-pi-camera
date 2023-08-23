/**
 * Copyright (c) 2019 Paul-Louis Ageneau
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

#ifndef RTC_IMPL_QUEUE_H
#define RTC_IMPL_QUEUE_H

#include "common.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <queue>

namespace rtc::impl {

template <typename T> class Queue {
public:
	using amount_function = std::function<size_t(const T &element)>;

	Queue(size_t limit = 0, amount_function func = nullptr);
	~Queue();

	void stop();
	bool running() const;
	bool empty() const;
	bool full() const;
	size_t size() const;   // elements
	size_t amount() const; // amount
	void push(T element);
	optional<T> pop();
	optional<T> tryPop();
	optional<T> peek();
	optional<T> exchange(T element);
	bool wait(const optional<std::chrono::milliseconds> &duration = nullopt);

private:
	void pushImpl(T element);
	optional<T> popImpl();

	const size_t mLimit;
	size_t mAmount;
	std::queue<T> mQueue;
	std::condition_variable mPopCondition, mPushCondition;
	amount_function mAmountFunction;
	bool mStopping = false;

	mutable std::mutex mMutex;
};

template <typename T>
Queue<T>::Queue(size_t limit, amount_function func) : mLimit(limit), mAmount(0) {
	mAmountFunction = func ? func : [](const T &element) -> size_t {
		static_cast<void>(element);
		return 1;
	};
}

template <typename T> Queue<T>::~Queue() { stop(); }

template <typename T> void Queue<T>::stop() {
	std::lock_guard lock(mMutex);
	mStopping = true;
	mPopCondition.notify_all();
	mPushCondition.notify_all();
}

template <typename T> bool Queue<T>::running() const {
	std::lock_guard lock(mMutex);
	return !mQueue.empty() || !mStopping;
}

template <typename T> bool Queue<T>::empty() const {
	std::lock_guard lock(mMutex);
	return mQueue.empty();
}

template <typename T> bool Queue<T>::full() const {
	std::lock_guard lock(mMutex);
	return mQueue.size() >= mLimit;
}

template <typename T> size_t Queue<T>::size() const {
	std::lock_guard lock(mMutex);
	return mQueue.size();
}

template <typename T> size_t Queue<T>::amount() const {
	std::lock_guard lock(mMutex);
	return mAmount;
}

template <typename T> void Queue<T>::push(T element) {
	std::unique_lock lock(mMutex);
	mPushCondition.wait(lock, [this]() { return !mLimit || mQueue.size() < mLimit || mStopping; });
	pushImpl(std::move(element));
}

template <typename T> optional<T> Queue<T>::pop() {
	std::unique_lock lock(mMutex);
	mPopCondition.wait(lock, [this]() { return !mQueue.empty() || mStopping; });
	return popImpl();
}

template <typename T> optional<T> Queue<T>::tryPop() {
	std::unique_lock lock(mMutex);
	return popImpl();
}

template <typename T> optional<T> Queue<T>::peek() {
	std::unique_lock lock(mMutex);
	return !mQueue.empty() ? std::make_optional(mQueue.front()) : nullopt;
}

template <typename T> optional<T> Queue<T>::exchange(T element) {
	std::unique_lock lock(mMutex);
	if (mQueue.empty())
		return nullopt;

	std::swap(mQueue.front(), element);
	return std::make_optional(std::move(element));
}

template <typename T> bool Queue<T>::wait(const optional<std::chrono::milliseconds> &duration) {
	std::unique_lock lock(mMutex);
	if (duration) {
		return mPopCondition.wait_for(lock, *duration,
		                              [this]() { return !mQueue.empty() || mStopping; });
	} else {
		mPopCondition.wait(lock, [this]() { return !mQueue.empty() || mStopping; });
		return true;
	}
}

template <typename T> void Queue<T>::pushImpl(T element) {
	if (mStopping)
		return;

	mAmount += mAmountFunction(element);
	mQueue.emplace(std::move(element));
	mPopCondition.notify_one();
}

template <typename T> optional<T> Queue<T>::popImpl() {
	if (mQueue.empty())
		return nullopt;

	mAmount -= mAmountFunction(mQueue.front());
	optional<T> element{std::move(mQueue.front())};
	mQueue.pop();
	return element;
}

} // namespace rtc::impl

#endif
