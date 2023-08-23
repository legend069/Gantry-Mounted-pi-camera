/**
 * Copyright (c) 2020-2022 Paul-Louis Ageneau
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

#ifndef RTC_IMPL_INIT_H
#define RTC_IMPL_INIT_H

#include "common.hpp"
#include "global.hpp" // for SctpSettings

#include <chrono>
#include <future>
#include <mutex>

namespace rtc::impl {

using init_token = shared_ptr<void>;

class Init {
public:
	static Init &Instance();

	Init(const Init &) = delete;
	Init &operator=(const Init &) = delete;
	Init(Init &&) = delete;
	Init &operator=(Init &&) = delete;

	init_token token();
	void preload();
	std::shared_future<void> cleanup();
	void setSctpSettings(SctpSettings s);

private:
	Init();
	~Init();

	void doInit();
	void doCleanup();

	std::optional<shared_ptr<void>> mGlobal;
	weak_ptr<void> mWeak;
	bool mInitialized = false;
	SctpSettings mCurrentSctpSettings = {};
	std::mutex mMutex;
	std::shared_future<void> mCleanupFuture;

	struct TokenPayload;
};

} // namespace rtc::impl

#endif
