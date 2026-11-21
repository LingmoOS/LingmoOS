// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/files/operation_context.h"

#include <cerrno>

namespace Lingmo::Files {

void OperationContext::checkpoint(const std::string& path) const
{
    if (canceled.load() || (cancelFlag && cancelFlag->load()))
        throw Failure(ECANCELED, path);

    if (currentItem && !path.empty())
        currentItem(path);
}

void OperationContext::advance(uint64_t amount)
{
    bytes += amount;

    if (progress)
        progress(bytes, files);
}

} // namespace Lingmo::Files
