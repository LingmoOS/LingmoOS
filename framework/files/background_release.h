// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>
#include <utility>

#include "base/thread_pool.h"

namespace Lingmo::Files {

// Drops value on a pool thread: freeing a listing of 50,000 items or its URL index
// takes the UI thread tens of milliseconds. value must own nothing tied to the
// calling thread.
template<typename T>
void releaseInBackground(T value)
{
    auto owned = std::make_shared<T>(std::move(value));
    base::ThreadPool::global().post([owned]() mutable {
        owned.reset();
    });
}

} // namespace Lingmo::Files
