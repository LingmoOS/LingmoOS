// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/common.h"
#include "core/fileitem.h"

#include "base/signal.h"

#include "lingmo/files/directory_lister.h"
#include "lingmo/files/file_item.h"
#include "lingmo/files/url.h"

#include <memory>
#include <string>
#include <vector>

namespace fm {

// The files library's lister, with its items read into the views' snapshots
// (FileItem) on whichever thread asks: the sorting worker for a listing.
class DirectoryLister
{
public:
    DirectoryLister();
    ~DirectoryLister();

    DirectoryLister(const DirectoryLister&) = delete;
    DirectoryLister& operator=(const DirectoryLister&) = delete;

    // As given to openUrl, or the trash's root.
    std::string url() const { return m_lister.url().toString(); }
    bool isListing() const { return m_lister.isListing(); }
    const FileItem& rootItem() const { return m_rootItem; }
    const files::FileItemList& items() const { return m_lister.items(); }

    void setShowHiddenFiles(bool show) { m_lister.setShowHiddenFiles(show); }
    void setAutoUpdate(bool enabled) { m_lister.setAutoUpdate(enabled); }

    void openUrl(const std::string& url);
    void search(const std::string& rootUrl, const std::string& text);
    bool isSearching() const { return m_lister.isSearching(); }
    void refresh();
    void stop() { m_lister.stop(); }

    base::Signal<> started;
    base::Signal<> completed;
    base::Signal<> canceled;
    base::Signal<std::string> error;
    base::Signal<> itemsChanged;
    base::Signal<> directoryChanged;

    // The local file a URL names (trash:/ items included); empty otherwise.
    static std::string localPathForUrl(const std::string& url);
    // The snapshot of a library item; any thread.
    static FileItem snapshot(const files::FileItem& item);
    // A snapshot of one file by path, read now.
    static FileItem makeItem(const std::string& path);

private:
    files::DirectoryLister m_lister;
    FileItem m_rootItem;
    std::vector<base::Subscription> m_subscriptions;
};

} // namespace fm
