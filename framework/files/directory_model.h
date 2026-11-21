// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <unordered_map>

#include "base/signal.h"
#include "base/weak_ptr.h"
#include "lingmo/files/directory_lister.h"

namespace Lingmo::Files {

// The rows of a lister's listing, kept in step with it by the smallest changes:
// rows of unchanged files stay where they are, so a view keeps its selection
// and scroll position across refreshes.
class DirectoryModel
{
public:
    // The columns a view sorts by.
    enum Column {
        Name,
        Size,
        ModifiedTime,
        Type,
        Permissions,
        Owner,
        Group,
        // Of items at the top of a trash.
        OriginalPath,
        DeletionTime,
        ColumnCount
    };

    DirectoryModel() = default;

    DirectoryModel(const DirectoryModel&) = delete;
    DirectoryModel& operator=(const DirectoryModel&) = delete;

    void setDirLister(DirectoryLister* lister);
    DirectoryLister* dirLister() const { return m_lister; }

    const FileItemList& items() const { return m_items; }
    int rowCount() const { return int(m_items.size()); }

    // A null item for a row out of range.
    FileItem itemForRow(int row) const;
    // -1 when no row holds the URL.
    int rowForUrl(const Url& url) const;

    // Renames the row's file through the task controller; false for a name no
    // file can take or an item that is not local.
    bool rename(int row, const std::string& name);

    // Every row was replaced.
    base::Signal<> reset;
    // count rows now start at first.
    base::Signal<int, int> rowsInserted;
    // count rows that started at first are gone.
    base::Signal<int, int> rowsRemoved;
    // Rows first to last hold new items of the same files.
    base::Signal<int, int> dataChanged;
    base::Signal<const std::string&> error;

private:
    void synchronize();

    DirectoryLister* m_lister = nullptr;
    base::Subscription m_itemsChanged;
    base::Subscription m_directoryChanged;
    FileItemList m_items;
    // Built on the first lookup after rows move; a listing of thousands of files
    // would otherwise hash every URL on the UI thread.
    mutable std::unordered_map<Url, int, UrlHash> m_rowsByUrl;
    mutable bool m_rowsValid = true;
    base::WeakPtrFactory<DirectoryModel> m_weakFactory{this};
};

} // namespace Lingmo::Files
