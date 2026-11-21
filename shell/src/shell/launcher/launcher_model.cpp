// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/launcher/launcher_model.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <map>
#include <unistd.h>

#include "base/locale.h"
#include "base/paths.h"
#include "base/settings.h"
#include "base/strings.h"
#include "shell/common/app_launcher.h"
#include "shell/common/applications.h"
#include "shell/common/file_thread.h"
#include "shell/common/text_match.h"

namespace Lingmo::Shell {

static void updateItem(LauncherItem *item, const base::DesktopEntry &entry)
{
    item->id = entry.id;
    item->name = entry.name;
    item->genericName = entry.genericName;
    item->comment = entry.comment;
    item->iconName = entry.icon;
}

static std::string absolutePath(const std::string &path)
{
    std::error_code error;
    return std::filesystem::absolute(path, error).lexically_normal().string();
}

LauncherModel::LauncherModel(Applications &applications, ApplicationLauncher &launcher, std::string path)
    : m_applications(applications)
    , m_launcher(launcher)
    , m_path(std::move(path))
    , m_rows(*this, [](const LauncherItem &item) { return item.id; },
             [](const LauncherItem &a, const LauncherItem &b) {
                 return a.name == b.name && a.genericName == b.genericName && a.comment == b.comment
                         && a.iconName == b.iconName && a.newInstalled == b.newInstalled
                         && a.folder == b.folder && a.members == b.members
                         && a.memberIcons == b.memberIcons;
             })
{
    m_saveTimer.setSingleShot(true);
    m_saveTimer.onTimeout = [this] { save(); };

    m_applicationsChanged = m_applications.changed.connect([this] { onApplicationsChanged(); });
    m_load = runOnFileThread([path = m_path] { return load(path); },
                             [this](Stored stored) {
                                 m_firstLoad = !stored.valid;
                                 m_usage = std::move(stored.usage);
                                 m_folders = std::move(stored.folders);
                                 if (stored.valid) {
                                     m_items = std::move(stored.items);
                                     for (LauncherItem &item : m_items)
                                         item.newInstalled = std::find(stored.newAppIds.begin(),
                                                                       stored.newAppIds.end(), item.id)
                                                 != stored.newAppIds.end();
                                 }
                                 m_loaded = true;
                                 onApplicationsChanged();
                             });
}

LauncherModel::~LauncherModel()
{
    save();
    waitForFileThread();
}

LauncherModel::Stored LauncherModel::load(const std::string &path)
{
    const base::Settings settings(path);
    Stored stored;

    // Read first: a launch recorded before the list was ever saved still
    // counts.
    for (const std::string &id : settings.childKeys("LauncherUsage")) {
        const std::string value = settings.stringValue("LauncherUsage/" + id);
        const size_t space = value.find(' ');
        if (space == std::string::npos)
            continue;
        LaunchUsage usage;
        usage.count = int(std::strtol(value.substr(0, space).c_str(), nullptr, 10));
        usage.lastUsed = int64_t(std::strtoll(value.substr(space + 1).c_str(), nullptr, 10));
        if (usage.count > 0 && usage.lastUsed > 0)
            stored.usage.emplace(id, usage);
    }

    if (!settings.contains("Launcher/order"))
        return stored;

    // Names and icons come from the entries; only the ids are kept.
    for (const std::string &id : settings.stringListValue("Launcher/order")) {
        LauncherItem item;
        item.id = id;
        stored.items.push_back(std::move(item));
    }
    stored.valid = true;
    stored.newAppIds = settings.stringListValue("Launcher/newAppIds");
    for (const std::string &id : settings.childGroups("LauncherFolders")) {
        const std::string key = "LauncherFolders/" + id;
        const auto members = settings.stringListValue(key + "/members");
        if (!members.empty())
            stored.folders.emplace(id, std::pair {settings.stringValue(key + "/name", "Folder"), members});
    }
    return stored;
}

void LauncherModel::save()
{
    // A shutdown before the first scan must not set a baseline.
    if (!m_loaded || m_firstLoad)
        return;
    m_saveTimer.stop();

    std::vector<std::string> order;
    std::vector<std::string> newAppIds;
    for (const LauncherItem &item : m_items) {
        order.push_back(item.id);
        if (item.newInstalled)
            newAppIds.push_back(item.id);
    }

    std::map<std::string, LaunchUsage> usage = m_usage;
    const auto folders = m_folders;
    fileThread().post([path = m_path, order, newAppIds, usage, folders] {
        base::Settings settings(path);
        settings.remove("Launcher");
        settings.remove("LauncherFolders");
        // Rewritten whole, so the entries of applications that are gone do
        // not stay behind.
        settings.remove("LauncherUsage");
        settings.setValue("Launcher/order", order);
        settings.setValue("Launcher/newAppIds", newAppIds);
        for (const auto &[id, folder] : folders) {
            settings.setValue("LauncherFolders/" + id + "/name", folder.first);
            settings.setValue("LauncherFolders/" + id + "/members", folder.second);
        }
        for (const auto &[id, entry] : usage)
            settings.setValue("LauncherUsage/" + id,
                              std::to_string(entry.count) + ' ' + std::to_string(entry.lastUsed));
        settings.sync();
    });
}

void LauncherModel::delaySave()
{
    if (m_reorderBase) {
        m_savePending = true;
        return;
    }
    m_saveTimer.start(kSaveDelay);
}

void LauncherModel::beginReorder()
{
    std::vector<std::string> order;
    for (const LauncherItem &item : m_items)
        order.push_back(item.id);
    m_reorderBase = std::move(order);
    // A save already due would write the drag's moves.
    if (m_saveTimer.isActive()) {
        m_saveTimer.stop();
        m_savePending = true;
    }
}

void LauncherModel::endReorder(bool keep)
{
    if (!m_reorderBase)
        return;
    const std::vector<std::string> base = *std::exchange(m_reorderBase, std::nullopt);
    if (!keep) {
        std::map<std::string, size_t> positions;
        for (size_t i = 0; i < base.size(); ++i)
            positions.emplace(base[i], i);
        // Applications that arrived during the drag stay after the others.
        const auto position = [&positions, &base](const LauncherItem &item) {
            const auto found = positions.find(item.id);
            return found == positions.end() ? base.size() : found->second;
        };
        std::stable_sort(m_items.begin(), m_items.end(),
                         [&position](const LauncherItem &a, const LauncherItem &b) {
                             return position(a) < position(b);
                         });
        updateRows();
    }
    if (std::exchange(m_savePending, false))
        delaySave();
}

void LauncherModel::search(const std::string &key)
{
    const std::string trimmed(base::strings::trimmed(key));
    if (trimmed == m_searchKey)
        return;
    m_searchKey = trimmed;
    updateRows();
}

void LauncherModel::updateRows()
{
    if (m_searchKey.empty()) {
        m_rows.sync(m_items);
        return;
    }

    struct Ranked {
        LauncherItem item;
        int match = 0;
        int64_t weight = 0;
        size_t order = 0;
    };

    std::vector<Ranked> ranked;
    ranked.reserve(m_items.size());
    std::vector<LauncherItem> searchable;
    for (const LauncherItem &item : m_items) {
        if (item.folder) {
            auto members = folderItems(item.id);
            searchable.insert(searchable.end(), members.begin(), members.end());
        } else {
            searchable.push_back(item);
        }
    }
    for (size_t i = 0; i < searchable.size(); ++i) {
        const LauncherItem &item = searchable[i];
        // The name says the most about which application was meant; the rest
        // are there for the searches the name cannot answer.
        const int match = std::max({fuzzyMatchScore(item.name, m_searchKey),
                                    fuzzyMatchScore(item.genericName, m_searchKey),
                                    fuzzyMatchScore(item.comment, m_searchKey),
                                    fuzzyMatchScore(item.id, m_searchKey)});
        if (match == 0)
            continue;
        ranked.push_back(Ranked{item, match, frecency(item.id), i});
    }

    // Equally good matches go to what the person uses, and the user's own
    // order settles what is left.
    std::stable_sort(ranked.begin(), ranked.end(), [](const Ranked &a, const Ranked &b) {
        if (a.match != b.match)
            return a.match > b.match;
        if (a.weight != b.weight)
            return a.weight > b.weight;
        return a.order < b.order;
    });

    std::vector<LauncherItem> results;
    results.reserve(ranked.size());
    for (Ranked &row : ranked)
        results.push_back(std::move(row.item));
    m_rows.sync(results);
}

int64_t LauncherModel::frecency(const std::string &id) const
{
    const auto found = m_usage.find(id);
    if (found == m_usage.end() || found->second.count <= 0)
        return 0;

    const int64_t now = int64_t(std::time(nullptr));
    const int64_t age = std::max<int64_t>(0, now - found->second.lastUsed);

    // A start an hour ago says more about what someone wants now than one
    // from last month.
    int64_t weight = 10;
    if (age < 4 * 3600)
        weight = 100;
    else if (age < 24 * 3600)
        weight = 70;
    else if (age < 7 * 86400)
        weight = 50;
    else if (age < 30 * 86400)
        weight = 30;

    return int64_t(found->second.count) * weight;
}

void LauncherModel::recordLaunch(const std::string &id)
{
    LaunchUsage &usage = m_usage[id];
    usage.count += 1;
    usage.lastUsed = int64_t(std::time(nullptr));
    delaySave();
}

int LauncherModel::findById(const std::string &id) const
{
    std::string canonicalId = id;
    if (const DesktopEntryPtr entry = m_applications.byPath(id))
        canonicalId = entry->id;
    for (size_t i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == canonicalId)
            return int(i);
    }
    return -1;
}

void LauncherModel::clearNewInstalled(const std::vector<std::string> &ids)
{
    bool changed = false;
    for (const std::string &id : ids) {
        const int index = findById(id);
        if (index < 0) {
            m_openedUnknownIds.insert(id);
            continue;
        }
        if (m_items[size_t(index)].newInstalled) {
            m_items[size_t(index)].newInstalled = false;
            changed = true;
        }
    }
    if (changed) {
        updateRows();
        delaySave();
    }
}

void LauncherModel::move(int from, int to)
{
    // Indices are into the whole list, which the rows are only outside a
    // search.
    if (from == to || !m_searchKey.empty())
        return;
    if (from < 0 || to < 0 || size_t(from) >= m_items.size() || size_t(to) >= m_items.size())
        return;
    LauncherItem item = std::move(m_items[size_t(from)]);
    m_items.erase(m_items.begin() + from);
    m_items.insert(m_items.begin() + to, std::move(item));
    updateRows();
    delaySave();
}

void LauncherModel::sendToDesktop(const std::string &key)
{
    DesktopEntryPtr entry = m_applications.byId(key);
    if (!entry)
        entry = m_applications.byPath(key);
    if (!entry)
        return;
    // A link, not a copy: the file manager runs only launchers that resolve
    // into a menu directory.
    fileThread().post([source = entry->path] {
        const std::string desktop = base::paths::userDirectory(base::paths::UserDirectory::Desktop);
        const std::string target = desktop + '/' + std::filesystem::path(source).filename().string();
        (void)symlink(source.c_str(), target.c_str());
    });
}

bool LauncherModel::launchRow(int row)
{
    if (row < 0 || row >= count())
        return false;
    return launch(rows()[size_t(row)].id);
}

bool LauncherModel::launch(const std::string &idOrPath)
{
    DesktopEntryPtr entry = m_applications.byId(idOrPath);
    if (!entry)
        entry = m_applications.byPath(idOrPath);
    if (!entry)
        return false;

    clearNewInstalled({entry->id});
    const std::vector<std::string> command = entry->command();
    if (command.empty())
        return false;
    m_launcher.launchCommand(command, entry->workingDirectory, entry->id);
    recordLaunch(entry->id);
    applicationLaunched();
    return true;
}

void LauncherModel::onApplicationsChanged()
{
    if (!m_loaded || m_applications.isLoading())
        return;

    // Only entries that pass the display rules: a saved list must not bring
    // back an application that is now hidden or not for this desktop.
    const std::vector<DesktopEntryPtr> &available = m_applications.applications();
    std::map<std::string, DesktopEntryPtr> byId;
    std::map<std::string, DesktopEntryPtr> byPath;
    for (const DesktopEntryPtr &entry : available) {
        byId.emplace(entry->id, entry);
        byPath.emplace(absolutePath(entry->path), entry);
    }

    m_appItems.clear();
    for (const DesktopEntryPtr &entry : available) {
        if (entry->terminal)
            continue;
        LauncherItem item;
        updateItem(&item, *entry);
        m_appItems.emplace(item.id, std::move(item));
    }

    for (auto it = m_folders.begin(); it != m_folders.end();) {
        auto &members = it->second.second;
        std::erase_if(members, [&](const std::string &id) { return !m_appItems.contains(id); });
        if (members.size() >= 2) {
            ++it;
            continue;
        }
        const auto top = std::find_if(m_items.begin(), m_items.end(), [&](const LauncherItem &item) {
            return item.id == it->first;
        });
        if (top != m_items.end()) {
            if (!members.empty())
                *top = m_appItems.at(members.front());
            else
                m_items.erase(top);
        }
        it = m_folders.erase(it);
    }

    std::vector<LauncherItem> updated;
    std::set<std::string> seen;
    for (LauncherItem item : m_items) {
        if (const auto folder = m_folders.find(item.id); folder != m_folders.end()) {
            item.folder = true;
            item.name = folder->second.first;
            item.members.clear();
            item.memberIcons.clear();
            for (const std::string &member : folder->second.second) {
                if (m_appItems.contains(member)) {
                    item.members.push_back(member);
                    item.memberIcons.push_back(m_appItems.at(member).iconName);
                }
                seen.insert(member);
            }
            if (!item.members.empty())
                updated.push_back(std::move(item));
            continue;
        }
        DesktopEntryPtr entry;
        if (const auto it = byId.find(item.id); it != byId.end())
            entry = it->second;
        else if (const auto path = byPath.find(absolutePath(item.id)); path != byPath.end())
            entry = path->second;
        if (!entry || entry->terminal || seen.count(entry->id))
            continue;
        updateItem(&item, *entry);
        updated.push_back(item);
        seen.insert(entry->id);
    }

    for (const DesktopEntryPtr &entry : available) {
        if (entry->terminal || seen.count(entry->id))
            continue;
        LauncherItem item;
        updateItem(&item, *entry);
        item.newInstalled = !m_firstLoad && !m_openedUnknownIds.erase(entry->id);
        updated.push_back(item);
        seen.insert(entry->id);
    }

    if (m_firstLoad) {
        const base::Locale &locale = base::Locale::system();
        std::sort(updated.begin(), updated.end(), [&locale](const LauncherItem &a, const LauncherItem &b) {
            return locale.compare(a.name, b.name) < 0;
        });
        m_firstLoad = false;
    }

    m_items = std::move(updated);
    updateRows();
    delaySave();
}

bool LauncherModel::makeFolder(const std::string &sourceId, const std::string &targetId)
{
    if (sourceId == targetId || !m_searchKey.empty())
        return false;
    // A folder is represented by its top-level item. Its members are not
    // draggable source rows, so never allow a folder id to be inserted into
    // another folder through a stale drag/drop request.
    if (m_folders.contains(sourceId))
        return false;
    auto source = std::find_if(m_items.begin(), m_items.end(), [&](const LauncherItem &item) {
        return item.id == sourceId;
    });
    auto target = std::find_if(m_items.begin(), m_items.end(), [&](const LauncherItem &item) {
        return item.id == targetId;
    });
    if (source == m_items.end() || target == m_items.end())
        return false;
    if (source->folder)
        return false;
    if (target->folder) {
        auto &members = m_folders[targetId].second;
        if (std::find(members.begin(), members.end(), sourceId) != members.end())
            return false;
        members.push_back(sourceId);
        target->members = members;
        target->memberIcons.push_back(source->iconName);
        m_items.erase(source);
    } else {
        static uint64_t serial = 0;
        std::string folderId;
        do {
            folderId = "folder-" + std::to_string(std::time(nullptr)) + "-" + std::to_string(++serial);
        } while (m_folders.contains(folderId));
        const size_t targetIndex = size_t(std::distance(m_items.begin(), target));
        const std::string targetMember = target->id;
        const std::string targetIcon = target->iconName;
        const std::string sourceIcon = source->iconName;
        m_items.erase(std::remove_if(m_items.begin(), m_items.end(), [&](const LauncherItem &item) {
                          return item.id == sourceId || item.id == targetId;
                      }), m_items.end());
        LauncherItem folder;
        folder.id = folderId;
        folder.name = "Folder";
        folder.folder = true;
        folder.members = {targetMember, sourceId};
        folder.memberIcons = {targetIcon, sourceIcon};
        m_folders[folderId] = {folder.name, folder.members};
        m_items.insert(m_items.begin() + std::min(targetIndex, m_items.size()), std::move(folder));
    }
    updateRows();
    delaySave();
    return true;
}

bool LauncherModel::renameFolder(const std::string &folderId, std::string name)
{
    auto folder = m_folders.find(folderId);
    if (folder == m_folders.end() || name.empty()
        || std::all_of(name.begin(), name.end(), [](unsigned char c) { return std::isspace(c); }))
        return false;
    if (folder->second.first == name)
        return false;
    folder->second.first = std::move(name);
    for (LauncherItem &item : m_items) {
        if (item.id == folderId) {
            item.name = folder->second.first;
            updateRows();
            delaySave();
            return true;
        }
    }
    return false;
}

bool LauncherModel::removeFromFolder(const std::string &folderId, const std::string &memberId)
{
    auto folder = m_folders.find(folderId);
    if (folder == m_folders.end())
        return false;
    auto &members = folder->second.second;
    const auto member = std::find(members.begin(), members.end(), memberId);
    if (member == members.end())
        return false;
    if (!m_appItems.contains(memberId))
        return false;
    const auto top = std::find_if(m_items.begin(), m_items.end(), [&](const LauncherItem &item) {
        return item.id == folderId;
    });
    if (top == m_items.end())
        return false;
    LauncherItem extracted = m_appItems.at(memberId);
    members.erase(member);
    if (members.size() < 2) {
        if (!members.empty() && m_appItems.contains(members.front())) {
            *top = m_appItems.at(members.front());
            m_items.insert(std::next(top), std::move(extracted));
        } else {
            const auto index = std::distance(m_items.begin(), top);
            m_items.erase(top);
            m_items.insert(m_items.begin() + index, std::move(extracted));
        }
        m_folders.erase(folder);
    } else {
        top->members = members;
        top->memberIcons.clear();
        for (const std::string &id : members)
            top->memberIcons.push_back(m_appItems.contains(id) ? m_appItems.at(id).iconName : "");
        m_items.insert(std::next(top), std::move(extracted));
    }
    updateRows();
    delaySave();
    return true;
}

bool LauncherModel::moveInFolder(const std::string &folderId, const std::string &memberId, int toIndex)
{
    const auto folder = m_folders.find(folderId);
    if (folder == m_folders.end())
        return false;
    auto &members = folder->second.second;
    const auto from = std::find(members.begin(), members.end(), memberId);
    if (from == members.end())
        return false;
    const int clamped = std::clamp(toIndex, 0, int(members.size()) - 1);
    if (int(std::distance(members.begin(), from)) == clamped)
        return false;
    std::string moved = *from;
    members.erase(from);
    members.insert(members.begin() + clamped, std::move(moved));
    updateRows();
    delaySave();
    return true;
}

bool LauncherModel::dissolveFolder(const std::string &folderId)
{
    const auto folder = m_folders.find(folderId);
    if (folder == m_folders.end())
        return false;
    const auto top = std::find_if(m_items.begin(), m_items.end(), [&](const LauncherItem &item) {
        return item.id == folderId;
    });
    if (top == m_items.end())
        return false;

    std::vector<LauncherItem> members;
    for (const std::string &id : folder->second.second) {
        if (const auto item = m_appItems.find(id); item != m_appItems.end())
            members.push_back(item->second);
    }
    if (members.empty()) {
        m_items.erase(top);
    } else {
        const auto index = std::distance(m_items.begin(), top);
        m_items.erase(top);
        m_items.insert(m_items.begin() + index,
                       std::make_move_iterator(members.begin()), std::make_move_iterator(members.end()));
    }
    m_folders.erase(folder);
    updateRows();
    delaySave();
    return true;
}

std::vector<LauncherItem> LauncherModel::folderItems(const std::string &folderId) const
{
    std::vector<LauncherItem> result;
    const auto folder = m_folders.find(folderId);
    if (folder == m_folders.end())
        return result;
    for (const std::string &id : folder->second.second) {
        if (const auto item = m_appItems.find(id); item != m_appItems.end())
            result.push_back(item->second);
    }
    return result;
}

LauncherPageModel::LauncherPageModel(LauncherModel &source)
    : m_source(source)
    , m_rows(*this, [](const std::string &id) { return id; })
{
    const auto resync = [this](int, int) { sync(); };
    m_subscriptions.push_back(m_source.rowsInserted.connect(resync));
    m_subscriptions.push_back(m_source.rowsRemoved.connect(resync));
    m_subscriptions.push_back(m_source.rowMoved.connect(resync));
    m_subscriptions.push_back(m_source.modelReset.connect([this] { this->sync(); }));
    m_subscriptions.push_back(m_source.dataChanged.connect([this](int first, int last) {
        for (int row = 0; row < count(); ++row) {
            const int source = sourceRow(row);
            if (source >= first && source <= last)
                dataChanged(row, row);
        }
    }));
    sync();
}

void LauncherPageModel::setStartIndex(int startIndex)
{
    if (startIndex == m_startIndex)
        return;
    m_startIndex = startIndex;
    sync();
}

void LauncherPageModel::setLimitCount(int limitCount)
{
    if (limitCount == m_limitCount)
        return;
    m_limitCount = limitCount;
    sync();
}

int LauncherPageModel::sourceRow(int row) const
{
    if (row < 0 || row >= count())
        return -1;
    const std::string &id = m_rows.rows()[size_t(row)];
    const auto &rows = m_source.rows();
    const int end = std::min(int(rows.size()), m_startIndex + m_limitCount);
    for (int source = std::max(0, m_startIndex); source < end; ++source) {
        if (rows[size_t(source)].id == id)
            return source;
    }
    return -1;
}

const LauncherItem *LauncherPageModel::item(int row) const
{
    const int source = sourceRow(row);
    return source < 0 ? nullptr : &m_source.rows()[size_t(source)];
}

void LauncherPageModel::sync()
{
    std::vector<std::string> ids;
    const auto &rows = m_source.rows();
    const int end = std::min(int(rows.size()), m_startIndex + m_limitCount);
    for (int source = std::max(0, m_startIndex); source < end; ++source)
        ids.push_back(rows[size_t(source)].id);
    m_rows.sync(ids);
}

} // namespace Lingmo::Shell
