// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "base/signal.h"
#include "base/thread_pool.h"
#include "base/timer.h"
#include "shell/common/list_model.h"
#include "shell/common/synced_list.h"

namespace Lingmo::Shell {

class Applications;
class ApplicationLauncher;

struct LauncherItem {
    std::string id;
    std::string name;
    std::string genericName;
    std::string comment;
    std::string iconName;
    // Installed since the launcher last looked, until it is opened.
    bool newInstalled = false;
    bool folder = false;
    std::vector<std::string> members;
    std::vector<std::string> memberIcons;
};

// How often an application was started from the launcher, and when it was
// last started (seconds since the epoch, 0 for never). A search ranks the
// applications the query matches equally by this, so the ones a person
// actually uses come first.
struct LaunchUsage {
    int count = 0;
    int64_t lastUsed = 0;
};

// The launcher's applications in the user's order, or the ones a search
// finds: the order in the [Launcher] group of shell.conf (order, the desktop
// ids, and newAppIds), new applications appended and marked, alphabetical
// the first time. A search ranks instead: the name, generic name, comment or
// id fuzzy-matched, and equally good matches ordered by how recently and how
// often the application was started, from the [LauncherUsage] group.
class LauncherModel : public ListModelSignals
{
public:
    static constexpr std::chrono::milliseconds kSaveDelay{1000};

    // `path` is shell.conf. Both must outlive the model.
    LauncherModel(Applications &applications, ApplicationLauncher &launcher, std::string path);
    ~LauncherModel();

    LauncherModel(const LauncherModel &) = delete;
    LauncherModel &operator=(const LauncherModel &) = delete;

    // The rows: every application, or the search's results.
    const std::vector<LauncherItem> &rows() const { return m_rows.rows(); }
    int count() const { return int(m_rows.rows().size()); }
    // Every application in the user's order.
    const std::vector<LauncherItem> &allItems() const { return m_items; }

    void search(const std::string &key);
    // The index in allItems() of a desktop id or path; -1.
    int findById(const std::string &id) const;
    // Applications opened outside the launcher are no longer new either.
    void clearNewInstalled(const std::vector<std::string> &ids);
    // Within allItems(); not while searching.
    void move(int from, int to);
    bool makeFolder(const std::string &sourceId, const std::string &targetId);
    bool renameFolder(const std::string &folderId, std::string name);
    bool removeFromFolder(const std::string &folderId, const std::string &memberId);
    bool moveInFolder(const std::string &folderId, const std::string &memberId, int toIndex);
    bool dissolveFolder(const std::string &folderId);
    std::vector<LauncherItem> folderItems(const std::string &folderId) const;
    // A drag's moves are not saved while it runs, and a drag that ends
    // without a drop puts the order back as it was.
    void beginReorder();
    void endReorder(bool keep);
    // Links the entry's file onto the desktop.
    void sendToDesktop(const std::string &key);

    bool launch(const std::string &idOrPath);
    bool launchRow(int row);

    // Writes the order now; otherwise it goes a second after the last change.
    void save();

    base::Signal<> applicationLaunched;

private:
    struct Stored {
        bool valid = false;
        std::vector<LauncherItem> items;
        std::vector<std::string> newAppIds;
        std::map<std::string, LaunchUsage> usage;
        std::map<std::string, std::pair<std::string, std::vector<std::string>>> folders;
    };

    static Stored load(const std::string &path);
    void onApplicationsChanged();
    void updateRows();
    void delaySave();
    // Counts one start of `id` and saves it with the next write.
    void recordLaunch(const std::string &id);
    // Starts weighted by how recently and how often the application was
    // used, as a tie-break between equally good search matches.
    int64_t frecency(const std::string &id) const;

    Applications &m_applications;
    ApplicationLauncher &m_launcher;
    std::string m_path;
    std::vector<LauncherItem> m_items;
    std::map<std::string, LauncherItem> m_appItems;
    std::map<std::string, std::pair<std::string, std::vector<std::string>>> m_folders;
    SyncedList<LauncherItem> m_rows;
    std::string m_searchKey;
    // Kept for the whole session, not only while the launcher is open.
    std::map<std::string, LaunchUsage> m_usage;
    // Opened before the registry listed them; they do not arrive as new.
    std::set<std::string> m_openedUnknownIds;
    bool m_loaded = false;
    bool m_firstLoad = false;
    base::Timer m_saveTimer;
    // The order when a drag began, while it runs.
    std::optional<std::vector<std::string>> m_reorderBase;
    bool m_savePending = false;
    base::Job m_load;
    base::Subscription m_applicationsChanged;
};

// A page of the launcher's grid: the source's rows from startIndex(), at
// most limitCount() of them, kept by id so a page shows the same icons
// across changes elsewhere in the list.
class LauncherPageModel : public ListModelSignals
{
public:
    explicit LauncherPageModel(LauncherModel &source);

    int startIndex() const { return m_startIndex; }
    void setStartIndex(int startIndex);
    int limitCount() const { return m_limitCount; }
    void setLimitCount(int limitCount);

    int count() const { return int(m_rows.rows().size()); }
    // The source row of a page row.
    int sourceRow(int row) const;
    const LauncherItem *item(int row) const;

private:
    void sync();

    LauncherModel &m_source;
    SyncedList<std::string> m_rows;
    int m_startIndex = 0;
    int m_limitCount = 0;
    std::vector<base::Subscription> m_subscriptions;
};

} // namespace Lingmo::Shell

