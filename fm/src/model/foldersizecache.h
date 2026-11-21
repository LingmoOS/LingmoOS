// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/common.h"

#include "base/signal.h"
#include "base/thread_pool.h"

#include <atomic>
#include <map>
#include <memory>
#include <string>

namespace fm {

// How many entries the folders of a listing hold, counted on two threads
// of their own as the list view asks; -1 until counted.
class FolderSizeCache
{
public:
    FolderSizeCache();
    ~FolderSizeCache();

    // Forgets the counts; those under way still arrive, as fresh as any.
    void clear();
    // Forgets them and drops those under way, for another folder.
    void reset();
    int64_t count(const std::string& path);

    base::Signal<std::string> ready;

private:
    std::map<std::string, int64_t> m_counts;
    std::shared_ptr<std::atomic_bool> m_cancelled;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

} // namespace fm
