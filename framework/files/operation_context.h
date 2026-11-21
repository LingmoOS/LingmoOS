// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <string>

#include "lingmo/files/error.h"

namespace Lingmo::Files {

enum class ConflictPolicy {
    Fail,
    Skip,
    KeepBoth,
    Replace,
    Ask
};

struct OperationContext
{
    std::atomic_bool canceled{false};
    const std::atomic_bool* cancelFlag = nullptr;
    std::function<void(const std::string&)> currentItem;
    uint64_t bytes = 0;
    uint64_t files = 0;
    // Space the regular files found by a size scan occupy, without sparse holes.
    uint64_t allocated = 0;
    std::function<void(uint64_t, uint64_t)> progress;
    std::function<ConflictPolicy(const std::string& source, const std::string& destination)> conflict;
    // Set once a Replace merges into an existing folder, which then holds more than was copied.
    bool merged = false;

    void checkpoint(const std::string& path = {}) const;
    void advance(uint64_t amount);
};

} // namespace Lingmo::Files
