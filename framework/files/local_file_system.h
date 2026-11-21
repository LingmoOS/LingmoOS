// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <string>

#include "lingmo/files/file_item.h"
#include "lingmo/files/file_system_url.h"
#include "lingmo/files/operation_context.h"

namespace Lingmo::Files {

// The operations themselves, blocking, on the calling thread. Each throws a
// Failure; copies and moves stage their output under a hidden name and leave
// nothing behind when they fail or are cancelled.
class LocalFileSystem
{
public:
    static FileItemList enumerate(const Url& url, bool hidden = true);
    static uint64_t directorySize(const std::string& path, OperationContext& context);

    // The path the item went to, or empty when skipped.
    static std::string copy(const std::string& source, const std::string& destination, ConflictPolicy policy,
                            OperationContext& context);
    static std::string link(const std::string& source, const std::string& destination, ConflictPolicy policy,
                            OperationContext& context);
    static std::string move(const std::string& source, const std::string& destination, ConflictPolicy policy,
                            OperationContext& context);
    static void remove(const std::string& path, OperationContext& context);
    static void remove(const Url& url, OperationContext& context);
    static void mkdir(const std::string& path);

    static Url trash(const Url& url, OperationContext& context);
    static Url restore(const Url& url, ConflictPolicy policy, OperationContext& context);
    static void emptyTrash(OperationContext& context);
};

} // namespace Lingmo::Files
