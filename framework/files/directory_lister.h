// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "base/signal.h"
#include "base/thread_pool.h"
#include "base/timer.h"
#include "lingmo/files/error.h"
#include "lingmo/files/file_item.h"
#include "lingmo/files/file_watcher.h"
#include "lingmo/files/operation_context.h"

namespace Lingmo::Files {

// Lists a folder, or the whole trash for trash:/, on a pool thread and keeps the
// listing up to date while auto update is on. A refresh keeps the detected
// types of files whose kind, size and times are unchanged. Lives on its loop's
// thread, as its signals do.
class DirectoryLister
{
public:
    DirectoryLister();
    virtual ~DirectoryLister();

    DirectoryLister(const DirectoryLister&) = delete;
    DirectoryLister& operator=(const DirectoryLister&) = delete;

    const Url& url() const { return m_url; }
    bool isListing() const { return m_job.isPending(); }
    const FileItem& rootItem() const { return m_rootItem; }
    const FileItemList& items() const { return m_items; }

    void setShowHiddenFiles(bool show) { m_hidden = show; }
    void setAutoUpdate(bool enabled) { m_autoUpdate = enabled; }

    // The copied callback runs on workers, for several items at once, and must not
    // touch anything of the UI thread.
    void setItemPreparer(std::function<void(FileItem&)> preparer) { m_itemPreparer = std::move(preparer); }

    void openUrl(const Url& url);

    // Lists the files at any depth under root whose names contain every word of
    // text, in place of the directory's own, which an empty text lists again.
    // The first search hands found files on while it runs; it keeps no watch.
    void search(const Url& root, const std::string& text);
    bool isSearching() const { return !m_searchWords.empty(); }

    void updateDirectory(const Url& url);
    void emitChanges() { refresh(); }
    void stop();

    base::Signal<> started;
    base::Signal<> completed;
    base::Signal<> canceled;
    base::Signal<const std::string&> error;
    base::Signal<> itemsChanged;
    base::Signal<> directoryChanged;

private:
    void refresh();
    void watchDirectory();
    void reportError(const std::string& message);
    void cancelListing();
    void takeFound();

    struct Listing
    {
        FileItemList items;
        FileItem rootItem;
        Url url;
        Error error;
        std::string typeDatabase;
        bool typesSettled = false;
        // What changes the trash, which spans folders on every volume.
        std::vector<std::string> trashPaths;
        FileWatcher::Entries trashEntries;
    };

    // What a running search found since the last look.
    struct Found
    {
        std::mutex mutex;
        FileItemList items;
    };

    Url m_url;
    FileItem m_rootItem;
    FileItemList m_items;
    bool m_hidden = false;
    bool m_autoUpdate = true;
    bool m_pending = false;
    bool m_listed = false;
    std::string m_lastError;
    std::string m_typeDatabase;
    bool m_typesSettled = false;
    uint64_t m_generation = 0;
    FileWatcher m_watch;
    base::Subscription m_watchChanged;
    base::Subscription m_watchError;
    std::vector<std::string> m_trashPaths;
    FileWatcher::Entries m_trashEntries;
    base::Timer m_debounce;
    base::Timer m_retry;
    base::Job m_job;
    std::shared_ptr<OperationContext> m_context;
    std::function<void(FileItem&)> m_itemPreparer;
    Url m_searchRoot;
    std::vector<std::string> m_searchWords;
    std::shared_ptr<Found> m_found;
    base::Timer m_foundTimer;
};

} // namespace Lingmo::Files
