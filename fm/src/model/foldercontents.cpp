// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "model/foldercontents.h"
#include "core/lister.h"
#include "core/preferences.h"
#include "core/standardfolders.h"

#include "base/collator.h"

#include <algorithm>
#include <numeric>

namespace fm {

namespace {

// What the sort compares, read once per item instead of once per comparison.
struct SortEntry
{
    const std::string* nameKey = nullptr;
    std::string mimeType;
    std::string originKey;
    int64_t modified = 0;
    int64_t deleted = 0;
    uint64_t size = 0;
    bool dir = false;
};

struct Cancelled
{
};

} // namespace

struct FolderContents::Prepared
{
    FileItemList listed;
    uint64_t listingGeneration = 0;
    FileItemList ordered;
    std::unordered_map<std::string, int> rows;
    ContentsChange change;
    std::vector<std::string> consumedRenames;
};

FolderContents::FolderContents()
{
    m_timer.setSingleShot(true);
    m_timer.setInterval(std::chrono::milliseconds(0));
    m_timer.onTimeout = [this] {
        prepare();
    };
}

FolderContents::~FolderContents()
{
    cancelPreparation();
}

FileItemPtr FolderContents::item(int row) const
{
    return row >= 0 && row < count() ? m_items[size_t(row)] : nullptr;
}

int FolderContents::rowForUrl(const std::string& url) const
{
    const auto found = m_rows.find(url);
    return found != m_rows.end() ? found->second : -1;
}

int FolderContents::rowForLocalPath(const std::string& path) const
{
    for (size_t row = 0; row < m_items.size(); ++row) {
        if (m_items[row]->path == path)
            return int(row);
    }
    return -1;
}

std::string FolderContents::originalLocation(const FileItem& item)
{
    if (item.originalPath.empty())
        return {};

    const std::string folder = parentPath(item.originalPath);
    const std::string home = homePath();
    if (folder == home)
        return "~";
    if (home != "/" && folder.starts_with(home + '/'))
        return '~' + folder.substr(home.size());
    return folder;
}

void FolderContents::setLister(DirectoryLister* lister)
{
    if (m_lister == lister)
        return;

    m_itemsChanged.reset();
    clear();
    m_lister = lister;
    if (!lister)
        return;

    m_itemsChanged = lister->itemsChanged.connect([this] {
        ++m_listingGeneration;
        schedulePreparation();
    });
    if (!lister->items().empty())
        schedulePreparation();
}

void FolderContents::setQuery(const SortQuery& query)
{
    m_query = query;
    schedulePreparation();
}

void FolderContents::retakeSnapshots()
{
    if (!m_lister)
        return;
    m_listed.clear();
    ++m_listingGeneration;
    schedulePreparation();
}

void FolderContents::cancelPreparation()
{
    if (m_cancelled)
        m_cancelled->store(true);
    m_timer.stop();
    m_job.cancel();
}

void FolderContents::clear()
{
    cancelPreparation();
    m_items.clear();
    m_rows.clear();
    m_renames.clear();
    m_listed.clear();
    ++m_listingGeneration;
    reset.emit();
}

void FolderContents::expectRename(const std::string& from, const std::string& to)
{
    m_renames[from] = to;
}

void FolderContents::forgetRename(const std::string& from, const std::string& to)
{
    if (const auto found = m_renames.find(from); found != m_renames.end() && found->second == to)
        m_renames.erase(found);
}

bool FolderContents::isSettled() const
{
    return !m_lister->isListing() || m_lister->isSearching();
}

void FolderContents::schedulePreparation()
{
    cancelPreparation();
    if (m_lister && isSettled())
        m_timer.start();
}

void FolderContents::setFilter(Filter filter)
{
    m_filter = std::move(filter);
    schedulePreparation();
}

void FolderContents::prepare()
{
    if (!m_lister || !isSettled())
        return;

    auto cancelled = std::make_shared<std::atomic_bool>(false);
    m_cancelled = cancelled;

    std::vector<std::string> offered;
    for (const auto& [from, to] : m_renames)
        offered.push_back(from);

    // A new sort or filter of an unchanged listing reuses its snapshots.
    const bool relist = m_listedGeneration != m_listingGeneration;
    std::vector<files::FileItem> listed;
    if (relist)
        listed = m_lister->items();
    m_job = base::ThreadPool::global().run(
            [listed = std::move(listed), relist, kept = m_listed, generation = m_listingGeneration, previous = m_items,
             query = m_query, renames = m_renames, filter = m_filter, cancelled]() -> std::shared_ptr<Prepared> {
                auto result = std::make_shared<Prepared>();
                result->listingGeneration = generation;
                const auto checkpoint = [&] {
                    if (cancelled->load())
                        throw Cancelled();
                };

                try {
                    if (relist) {
                        // The previous snapshot of a file whose stat is unchanged is kept.
                        std::unordered_map<std::string, const FileItemPtr*> known;
                        known.reserve(kept.size());
                        for (const FileItemPtr& item : kept)
                            known[item->url] = &item;
                        // Numbers in names compare by value: "img2" before "img10".
                        base::Collator collator;
                        collator.setNumericMode(true);
                        const auto standardNames = standardFolderNames();
                        result->listed.reserve(listed.size());
                        for (const files::FileItem& file : listed) {
                            checkpoint();
                            FileItem snapshot = DirectoryLister::snapshot(file);
                            const auto found = known.find(snapshot.url);
                            if (found != known.end() && (*found->second)->sameMetadata(snapshot)) {
                                result->listed.push_back(*found->second);
                                continue;
                            }
                            // Standard folders sort by the name they are shown under.
                            std::string name = snapshot.name;
                            if (snapshot.isDir && snapshot.isLocalFile()) {
                                if (const auto standard = standardNames->find(snapshot.path); standard != standardNames->end())
                                    name = standard->second;
                            }
                            snapshot.sortKey = collator.sortKey(name);
                            result->listed.push_back(std::make_shared<const FileItem>(std::move(snapshot)));
                        }
                    } else {
                        result->listed = kept;
                    }
                    FileItemList incoming;
                    incoming.reserve(result->listed.size());
                    for (const FileItemPtr& item : result->listed) {
                        if (!filter || filter(*item))
                            incoming.push_back(item);
                    }

                    // Rows by URL, in listing order until the sort renumbers them.
                    std::unordered_map<std::string, int> listedRows;
                    listedRows.reserve(incoming.size());
                    for (size_t row = 0; row < incoming.size(); ++row)
                        listedRows[incoming[row]->url] = int(row);

                    std::vector<size_t> positions(incoming.size());
                    std::iota(positions.begin(), positions.end(), 0);

                    if (query.column >= 0) {
                        base::Collator collator;
                        collator.setNumericMode(true);

                        std::vector<SortEntry> entries(incoming.size());
                        for (size_t i = 0; i < incoming.size(); ++i) {
                            checkpoint();
                            const FileItem& item = *incoming[i];
                            SortEntry& entry = entries[i];
                            entry.dir = item.isDir;
                            entry.nameKey = &item.sortKey;
                            if (query.column == Preferences::Size)
                                entry.size = item.size;
                            else if (query.column == Preferences::ModifiedTime)
                                entry.modified = item.modified;
                            else if (query.column == Preferences::Type)
                                entry.mimeType = item.mimeType;
                            else if (query.column == Preferences::OriginalPath)
                                entry.originKey = collator.sortKey(originalLocation(item));
                            else if (query.column == Preferences::DeletionTime)
                                entry.deleted = item.deletionTime;
                        }

                        const auto less = [&](size_t a, size_t b) {
                            const SortEntry& left = entries[a];
                            const SortEntry& right = entries[b];

                            if ((query.directoriesFirst || query.column == Preferences::Size) && left.dir != right.dir)
                                return left.dir;

                            int order = 0;
                            if (query.column == Preferences::Size && !left.dir)
                                order = left.size < right.size ? -1 : left.size > right.size;
                            else if (query.column == Preferences::ModifiedTime)
                                order = left.modified < right.modified ? -1 : left.modified > right.modified;
                            else if (query.column == Preferences::Type)
                                order = left.mimeType.compare(right.mimeType);
                            else if (query.column == Preferences::OriginalPath)
                                order = left.originKey.compare(right.originKey);
                            else if (query.column == Preferences::DeletionTime)
                                order = left.deleted < right.deleted ? -1 : left.deleted > right.deleted;
                            if (order == 0)
                                order = left.nameKey->compare(*right.nameKey);
                            if (order == 0)
                                order = incoming[a]->url.compare(incoming[b]->url);
                            return query.descending ? order > 0 : order < 0;
                        };

                        checkpoint();
                        std::stable_sort(positions.begin(), positions.end(), less);
                    }

                    // Which listed files a previous row keeps, and those that are new.
                    std::vector<bool> retained(incoming.size(), false);
                    std::vector<int> previousRows(previous.size(), -1);
                    for (size_t row = 0; row < previous.size(); ++row) {
                        checkpoint();
                        const FileItem& item = *previous[row];
                        int found = -1;
                        if (const auto it = listedRows.find(item.url); it != listedRows.end())
                            found = it->second;
                        if (found < 0) {
                            // A rename we made ourselves: the file went away under its old
                            // name and is back under the new one; the row stays.
                            if (const auto renamed = renames.find(item.url); renamed != renames.end()) {
                                result->consumedRenames.push_back(item.url);
                                if (const auto it = listedRows.find(renamed->second); it != listedRows.end())
                                    found = it->second;
                            }
                        }
                        // A rename onto an existing name keeps one row, not two.
                        if (found >= 0 && retained[size_t(found)])
                            found = -1;
                        if (found < 0) {
                            result->change.removed.push_back(item.url);
                            continue;
                        }
                        retained[size_t(found)] = true;
                        previousRows[row] = found;
                    }

                    if (query.column < 0) {
                        // Unsorted views keep their rows in place; new files come last.
                        positions.clear();
                        for (size_t row = 0; row < previous.size(); ++row) {
                            if (previousRows[row] >= 0)
                                positions.push_back(size_t(previousRows[row]));
                        }
                        for (size_t i = 0; i < incoming.size(); ++i) {
                            if (!retained[i])
                                positions.push_back(i);
                        }
                    }

                    result->ordered.reserve(positions.size());
                    std::unordered_map<std::string, int> oldRows;
                    for (size_t row = 0; row < previous.size(); ++row) {
                        if (previousRows[row] >= 0)
                            oldRows[incoming[size_t(previousRows[row])]->url] = int(row);
                    }
                    for (size_t row = 0; row < positions.size(); ++row) {
                        const FileItemPtr& item = incoming[positions[row]];
                        result->ordered.push_back(item);
                        result->rows[item->url] = int(row);
                        if (!retained[positions[row]]) {
                            result->change.added.push_back(item->url);
                            continue;
                        }
                        const int old = oldRows[item->url];
                        if (old != int(row))
                            result->change.reordered = true;
                        if (!previous[size_t(old)]->sameMetadata(*item) || previous[size_t(old)]->url != item->url)
                            result->change.changedRows.push_back(int(row));
                    }
                    if (!result->change.removed.empty() || !result->change.added.empty())
                        result->change.reordered = true;
                } catch (const Cancelled&) {
                    return nullptr;
                }
                return result;
            },
            [this, cancelled, offered](std::shared_ptr<Prepared> prepared) {
                if (!prepared || cancelled->load())
                    return;
                // A rename this listing did not match had its chance; only ones
                // expected since it started stay.
                for (const std::string& from : offered)
                    m_renames.erase(from);
                publish(std::move(prepared));
            });

    preparationStarted.emit();
}

void FolderContents::publish(std::shared_ptr<Prepared> prepared)
{
    // The old lists are freed on a worker: tens of thousands of entries
    // take milliseconds the UI thread does not have.
    auto old = std::make_unique<std::tuple<FileItemList, FileItemList, std::unordered_map<std::string, int>>>(
            std::move(m_listed), std::move(m_items), std::move(m_rows));
    m_listed = std::move(prepared->listed);
    m_listedGeneration = prepared->listingGeneration;
    m_items = std::move(prepared->ordered);
    m_rows = std::move(prepared->rows);
    rowsChanged.emit(prepared->change);
    preparationFinished.emit();
    base::ThreadPool::global().post([old = std::shared_ptr(std::move(old)), prepared = std::move(prepared)]() mutable {
        old.reset();
        prepared.reset();
    });
}

} // namespace fm
