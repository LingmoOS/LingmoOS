// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/files/directory_model.h"

#include <algorithm>
#include <unordered_set>

#include "lingmo/files/background_release.h"
#include "lingmo/files/task.h"
#include "path_utils_p.h"

namespace Lingmo::Files {

void DirectoryModel::setDirLister(DirectoryLister* lister)
{
    m_itemsChanged.reset();
    m_directoryChanged.reset();

    m_lister = lister;
    m_items = lister->items();
    m_rowsValid = false;
    reset.emit();

    m_itemsChanged = lister->itemsChanged.connect([this] {
        synchronize();
    });
    m_directoryChanged = lister->directoryChanged.connect([this] {
        releaseInBackground(std::exchange(m_items, {}));
        m_rowsValid = false;
        reset.emit();
    });
}

FileItem DirectoryModel::itemForRow(int row) const
{
    return row >= 0 && row < int(m_items.size()) ? m_items[size_t(row)] : FileItem();
}

int DirectoryModel::rowForUrl(const Url& url) const
{
    if (!m_rowsValid) {
        releaseInBackground(std::exchange(m_rowsByUrl, {}));
        m_rowsByUrl.reserve(m_items.size());
        for (int row = 0; row < int(m_items.size()); ++row)
            m_rowsByUrl.insert_or_assign(m_items[size_t(row)].url(), row);
        m_rowsValid = true;
    }

    const auto found = m_rowsByUrl.find(url);
    return found == m_rowsByUrl.cend() ? -1 : found->second;
}

bool DirectoryModel::rename(int row, const std::string& name)
{
    if (name.empty() || name == "." || name == ".." || name.find('/') != std::string::npos
        || name.find('\0') != std::string::npos)
        return false;

    const auto item = itemForRow(row);
    if (!item.isLocalFile())
        return false;

    const Url destination = Url::fromLocalFile(internal::absolutePathOf(item.localPath()) + '/' + name);
    Task* task = TaskController::instance()->submit({Operation::Rename, {item.url()}, destination, ConflictPolicy::Ask, {}});

    task->whenFinished([self = m_weakFactory.getWeakPtr()](Task* done) {
        if (DirectoryModel* model = self.get(); model && done->error())
            model->error.emit(done->errorString());
    });

    return true;
}

void DirectoryModel::synchronize()
{
    const FileItemList incoming = m_lister->items();

    if (m_items.empty()) {
        if (!incoming.empty()) {
            m_items = incoming;
            m_rowsValid = false;
            rowsInserted.emit(0, int(incoming.size()));
        }
        return;
    }

    // A refresh usually lists the same files in the same order.
    const int common = int(std::min(m_items.size(), incoming.size()));
    int same = 0;

    while (same < common && m_items[size_t(same)].url() == incoming[size_t(same)].url())
        ++same;

    if (same == int(m_items.size()) && same == int(incoming.size())) {
        releaseInBackground(std::exchange(m_items, incoming));
        dataChanged.emit(0, int(m_items.size()) - 1);
        return;
    }

    // Files added or removed in one place leave the same files at both ends.
    int tail = 0;
    while (tail < common - same
           && m_items[m_items.size() - 1 - size_t(tail)].url() == incoming[incoming.size() - 1 - size_t(tail)].url())
        ++tail;

    const int oldEnd = int(m_items.size()) - tail;
    const int newEnd = int(incoming.size()) - tail;

    std::unordered_set<Url, UrlHash> between;
    between.reserve(size_t(std::max(0, newEnd - same)));
    for (int i = same; i < newEnd; ++i)
        between.insert(incoming[size_t(i)].url());

    bool moved = false;
    for (int row = same; row < oldEnd && !moved; ++row)
        moved = between.contains(m_items[size_t(row)].url());

    // No file in between kept its row: replace those rows, and the model mirrors
    // the listing without matching every URL.
    if (!moved) {
        FileItemList released;

        if (oldEnd > same) {
            for (int row = same; row < oldEnd; ++row)
                released.push_back(std::move(m_items[size_t(row)]));
            m_items.erase(m_items.begin() + same, m_items.begin() + oldEnd);
            m_rowsValid = false;
            rowsRemoved.emit(same, oldEnd - same);
        }

        releaseInBackground(std::exchange(m_items, incoming));
        m_rowsValid = false;
        if (newEnd > same)
            rowsInserted.emit(same, newEnd - same);

        releaseInBackground(std::move(released));

        if (!m_items.empty())
            dataChanged.emit(0, int(m_items.size()) - 1);
        return;
    }

    std::unordered_map<Url, ptrdiff_t, UrlHash> positions;
    positions.reserve(incoming.size());
    for (size_t i = 0; i < incoming.size(); ++i)
        positions.insert_or_assign(incoming[i].url(), ptrdiff_t(i));

    // Two files can be listed under one URL; their rows then match the last file
    // with it, never by position.
    const bool unique = positions.size() == incoming.size();
    if (!unique)
        same = 0;

    // The rows replaced or removed, freed off this thread.
    FileItemList released;
    released.reserve(m_items.size());

    // Where each row's file is in the new listing, or -1 once it is gone.
    std::vector<ptrdiff_t> sources(m_items.size());
    for (size_t row = 0; row < m_items.size(); ++row) {
        if (int(row) < same) {
            sources[row] = ptrdiff_t(row);
        } else {
            const auto found = positions.find(m_items[row].url());
            sources[row] = found == positions.cend() ? -1 : found->second;
        }
    }

    for (int last = int(m_items.size()) - 1; last >= 0;) {
        if (sources[size_t(last)] >= 0) {
            --last;
            continue;
        }

        int first = last;

        while (first > 0 && sources[size_t(first - 1)] < 0)
            --first;

        for (int row = first; row <= last; ++row)
            released.push_back(std::move(m_items[size_t(row)]));
        m_items.erase(m_items.begin() + first, m_items.begin() + last + 1);
        sources.erase(sources.begin() + first, sources.begin() + last + 1);
        m_rowsValid = false;
        rowsRemoved.emit(first, last - first + 1);
        last = first - 1;
    }

    std::vector<bool> kept(incoming.size(), false);
    for (size_t row = 0; row < m_items.size(); ++row) {
        kept[size_t(sources[row])] = true;
        released.push_back(std::exchange(m_items[row], incoming[size_t(sources[row])]));
    }

    releaseInBackground(std::move(released));

    if (!m_items.empty())
        dataChanged.emit(0, int(m_items.size()) - 1);

    FileItemList added;
    for (size_t i = 0; i < incoming.size(); ++i) {
        if (!kept[unique ? i : size_t(positions[incoming[i].url()])])
            added.push_back(incoming[i]);
    }

    if (!added.empty()) {
        const int first = int(m_items.size());
        m_items.insert(m_items.end(), added.begin(), added.end());
        m_rowsValid = false;
        rowsInserted.emit(first, int(added.size()));
    }

    releaseInBackground(std::move(positions));
}

} // namespace Lingmo::Files
