// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "base/settings.h"
#include "base/signal.h"

namespace Lingmo::Input {
class DBusExport;
}

namespace Lingmo::ComputerControl {

// The parent of process `pid`, as /proc has it now.
std::optional<int64_t> parentProcess(int64_t pid);

// The application that started an agent's server, by the executable it runs.
// `file` names that very file (device, inode and change time), so a program
// put in its place, or the same one changed, is another application.
struct AgentApplication {
    std::string path;
    std::string file;
};

// The applications the user allowed to take control without asking, kept in
// compositor.conf [ComputerControl] and exported as
// org.lingmo.ComputerControl1 for Settings to list and forget.
class RememberedApps
{
public:
    explicit RememberedApps(const std::string &settingsFile);
    ~RememberedApps();

    RememberedApps(const RememberedApps &) = delete;
    RememberedApps &operator=(const RememberedApps &) = delete;

    bool registerService();

    // The parent of process `pid`, unless it can stand for any program: an
    // interpreter or a shell runs whatever it is given, and init adopts
    // orphans.
    static std::optional<AgentApplication> applicationOf(int64_t pid);

    bool allows(const AgentApplication &application) const;
    // Settings shows it by `appId`, its desktop entry, or else by `agentName`.
    void remember(const AgentApplication &application, const std::string &agentName,
                  const std::string &appId);
    void forget(const std::string &path);

    base::Signal<> changed;

private:
    struct Entry {
        std::string path;
        std::string file;
        std::string name;
        std::string appId;
    };

    void save();

    base::Settings m_settings;
    std::vector<Entry> m_entries;
    std::unique_ptr<input::DBusExport> m_object;
};

} // namespace Lingmo::ComputerControl

