// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/defaultapps/defaultapplications.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <filesystem>

#include "base/application_registry.h"
#include "base/mime_apps.h"

namespace fs = std::filesystem;
using namespace Lingmo::base;

namespace settings {

static bool contains(const std::vector<std::string>& list, const std::string& value)
{
    return std::find(list.begin(), list.end(), value) != list.end();
}

static bool playsVideo(const DesktopEntry& entry)
{
    if (!contains(entry.categories, "AudioVideo"))
        return false;
    return std::any_of(entry.mimeTypes.begin(), entry.mimeTypes.end(),
                       [](const std::string& type) { return type.starts_with("video/"); });
}

DefaultApplications::DefaultApplications()
{
    ApplicationRegistry& registry = ApplicationRegistry::instance();
    m_registry = registry.changed.connect([this] { load(); });
    if (!registry.isLoading())
        load();
    // Other programs, the file manager among them, choose defaults too.
    m_reload.setSingleShot(true);
    m_reload.onTimeout = [this] { load(); };
    watchLists();
}

// Lists are saved by replacing the file; the folder sees them come back.
void DefaultApplications::watchLists()
{
    m_watches.clear();
    const auto reload = [this](auto&&...) { m_reload.start(std::chrono::milliseconds(100)); };
    for (const std::string& path : {mimeapps::userListPath(), mimeapps::userTerminalsListPath()})
        m_watches.push_back(FileWatch::file(path, reload));
    m_watches.push_back(FileWatch::directory(fs::path(mimeapps::userListPath()).parent_path().string(), reload));
}

void DefaultApplications::load()
{
    for (auto& list : m_lists)
        list.clear();
    std::fill(std::begin(m_defaults), std::end(m_defaults), -1);

    for (const auto& entry : ApplicationRegistry::instance().applications()) {
        Application application{entry->path, fs::path(entry->path).filename().string(), entry->name, entry->icon,
                                entry->mimeTypes};
        if (contains(entry->categories, "WebBrowser") && contains(entry->mimeTypes, "x-scheme-handler/http"))
            m_lists[Browser].push_back(std::move(application));
        else if (contains(entry->categories, "Email") && contains(entry->mimeTypes, "x-scheme-handler/mailto"))
            m_lists[Email].push_back(std::move(application));
        else if (contains(entry->categories, "TerminalEmulator"))
            m_lists[Terminal].push_back(std::move(application));
        else if (playsVideo(*entry))
            m_lists[VideoPlayer].push_back(std::move(application));
    }

    const auto first = [](const std::vector<std::string>& list) { return list.empty() ? std::string() : list.front(); };
    const std::string browser = first(mimeapps::associations(mimeapps::userListPath(), "Default Applications",
                                                             "x-scheme-handler/http"));
    const std::string email = first(mimeapps::associations(mimeapps::userListPath(), "Default Applications",
                                                           "x-scheme-handler/mailto"));
    const std::string terminal = mimeapps::defaultTerminal(mimeapps::userTerminalsListPath());
    // Players differ in the video types they list; the commonest one stands
    // for all. Without a choice, this is what the file manager opens videos with.
    const std::string videoPlayer = mimeapps::defaultApplication(mimeapps::listPaths(), "video/mp4");

    const auto find = [this](Kind kind, const std::string& value, bool byPath) {
        const auto& list = m_lists[kind];
        for (size_t i = 0; i < list.size(); ++i) {
            if ((byPath ? list[i].path : list[i].fileName) == value) {
                m_defaults[kind] = int(i);
                return;
            }
        }
    };
    find(Browser, browser, false);
    find(Email, email, false);
    find(Terminal, terminal, false);
    find(VideoPlayer, videoPlayer, true);
    changed.emit();
}

void DefaultApplications::setDefault(Kind kind, int index)
{
    const auto& list = m_lists[kind];
    if (index < 0 || index >= int(list.size()))
        return;
    const Application& application = list[size_t(index)];
    const std::string userList = mimeapps::userListPath();
    const auto set = [&userList](const std::string& type, const std::string& desktop) {
        if (!mimeapps::setDefault(userList, type, desktop))
            fprintf(stderr, "Failed to save default application for %s\n", type.c_str());
    };
    switch (kind) {
    case Browser:
        set("x-scheme-handler/http", application.fileName);
        set("x-scheme-handler/https", application.fileName);
        break;
    case Email:
        set("x-scheme-handler/mailto", application.fileName);
        break;
    case Terminal:
        if (!mimeapps::setDefaultTerminal(mimeapps::userTerminalsListPath(), application.fileName)) {
            fprintf(stderr, "Failed to save the default terminal\n");
            return;
        }
        break;
    case VideoPlayer:
        for (const std::string& type : application.mimeTypes) {
            if (type.starts_with("video/"))
                set(type, application.fileName);
        }
        break;
    case KindCount:
        return;
    }
    load();
}

} // namespace settings
