// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/common.h"
#include "core/fileitem.h"

#include "base/signal.h"
#include "base/thread_pool.h"
#include "base/timer.h"

#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace fm {

class DirectoryLister;

struct SortQuery
{
    // A Preferences::SortColumn; -1 keeps the rows in the order they came,
    // new ones last, as the desktop wants.
    int column = 0;
    bool descending = false;
    bool directoriesFirst = true;
};

// What one update of the rows did, for views that keep state per file.
struct ContentsChange
{
    std::vector<std::string> removed;
    std::vector<std::string> added;
    // The rows of files whose metadata changed, in the new order.
    std::vector<int> changedRows;
    bool reordered = false;
};

// A listing's items sorted on a worker as the folder view shows them:
// numbers in names compare by value, standard folders sort by their shown
// names, the URL breaks ties. A folder shows once listed, a search as it finds.
class FolderContents
{
public:
    FolderContents();
    ~FolderContents();

    FolderContents(const FolderContents&) = delete;
    FolderContents& operator=(const FolderContents&) = delete;

    void setLister(DirectoryLister* lister);
    void setQuery(const SortQuery& query);
    // Which items show, asked on the sorting worker; none shows them all.
    using Filter = std::function<bool(const FileItem& item)>;
    void setFilter(Filter filter);
    const SortQuery& query() const { return m_query; }

    int count() const { return int(m_items.size()); }
    const FileItemList& items() const { return m_items; }
    // Null past the end.
    FileItemPtr item(int row) const;
    int rowForUrl(const std::string& url) const;
    int rowForLocalPath(const std::string& path) const;

    // Lets the next listing take `from` reappearing as `to` as the same row.
    void expectRename(const std::string& from, const std::string& to);
    // Drops the expectation of a rename that did not happen.
    void forgetRename(const std::string& from, const std::string& to);
    void schedulePreparation();
    // Takes every file's snapshot anew and sorts again: the standard
    // folders' shown names, which they sort by, follow the language.
    void retakeSnapshots();
    void cancelPreparation();
    bool isPreparing() const { return m_job.isPending() || m_timer.isActive(); }
    void clear();

    // The folder an item at the top of the trash was trashed from, home
    // written as ~; empty for any other item.
    static std::string originalLocation(const FileItem& item);

    base::Signal<> preparationStarted;
    base::Signal<> preparationFinished;
    base::Signal<const ContentsChange&> rowsChanged;
    base::Signal<> reset;

private:
    struct Prepared;

    bool isSettled() const;
    void prepare();
    void publish(std::shared_ptr<Prepared> prepared);

    DirectoryLister* m_lister = nullptr;
    base::Subscription m_itemsChanged;
    FileItemList m_items;
    std::unordered_map<std::string, int> m_rows;
    std::map<std::string, std::string> m_renames;
    SortQuery m_query;
    Filter m_filter;
    // Every listed file's snapshot, before the filter: a new sort or filter
    // reuses them while the listing is unchanged.
    FileItemList m_listed;
    uint64_t m_listedGeneration = 0;
    uint64_t m_listingGeneration = 1;
    base::Timer m_timer;
    base::Job m_job;
    std::shared_ptr<std::atomic_bool> m_cancelled;
};

} // namespace fm
