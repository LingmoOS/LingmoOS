// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "computer_control/remembered_apps.h"
#include "core/logging.h"

#include <sys/stat.h>

#include <algorithm>
#include <array>
#include <charconv>
#include <filesystem>
#include <format>
#include <string_view>
#include <system_error>

#include "base/dbus.h"
#include "base/file.h"
#include "base/json.h"
#include "input/dbus_export.h"

namespace Lingmo::ComputerControl {
namespace {

constexpr auto service = "org.lingmo.ComputerControl1";
constexpr auto path = "/org/lingmo/ComputerControl1";
constexpr auto key = "ComputerControl/AllowedApps";

// Programs that run what they are handed, matched on the executable's name
// with any version suffix ("python3.13", "node22").
constexpr std::array<std::string_view, 20> kRunsAnything = {
        "bash", "bun", "busybox", "dash", "deno", "env", "fish", "ksh", "lua", "node",
        "nodejs", "perl", "php", "python", "ruby", "sh", "sudo", "systemd", "tcsh", "zsh",
};

bool runsAnything(std::string_view name)
{
    const size_t end = name.find_first_of("0123456789.-");
    const std::string_view stem = name.substr(0, end);
    return std::find(kRunsAnything.begin(), kRunsAnything.end(), stem) != kRunsAnything.end();
}

} // namespace

std::optional<int64_t> parentProcess(int64_t pid)
{
    const auto stat = base::file::read(std::format("/proc/{}/stat", pid));
    if (!stat)
        return std::nullopt;
    // The command name in parentheses may hold spaces and ')'; the fields
    // after the last ')' are the state and then the parent.
    const size_t close = stat->rfind(')');
    if (close == std::string::npos)
        return std::nullopt;
    std::string_view rest = std::string_view(*stat).substr(close + 1);
    const size_t state = rest.find_first_not_of(' ');
    if (state == std::string_view::npos)
        return std::nullopt;
    rest = rest.substr(state + 1);
    rest.remove_prefix(std::min(rest.find_first_not_of(' '), rest.size()));
    int64_t parent = 0;
    if (std::from_chars(rest.data(), rest.data() + rest.size(), parent).ec != std::errc())
        return std::nullopt;
    return parent;
}

RememberedApps::RememberedApps(const std::string &settingsFile)
    : m_settings(settingsFile)
{
    const auto stored = base::json::Value::parse(m_settings.stringValue(key));
    if (!stored || !stored->isArray())
        return;
    for (const base::json::Value &value : stored->toArray()) {
        Entry entry {value["path"].toString(), value["file"].toString(), value["name"].toString(),
                     value["app_id"].toString()};
        if (!entry.path.empty() && !entry.file.empty())
            m_entries.push_back(std::move(entry));
    }
}

RememberedApps::~RememberedApps() = default;

bool RememberedApps::registerService()
{
    if (m_object)
        return true;
    auto exported = input::DBusExport::create(service, path, service);
    if (!exported) {
        LM_WARNING(lingmoInput) << "Cannot export computer control settings: no session bus";
        return false;
    }
    base::dbus::Object &object = exported->object();
    object.addMethod("allowedApps", [this] {
        std::vector<base::dbus::VariantMap> apps;
        for (const Entry &entry : m_entries)
            apps.push_back({{"path", base::dbus::Variant(entry.path)},
                            {"name", base::dbus::Variant(entry.name)},
                            {"app_id", base::dbus::Variant(entry.appId)}});
        return apps;
    });
    object.addMethod("forget", [this](const std::string &path) { forget(path); },
                     base::dbus::ArgumentNames {{"path"}, {}});
    object.addSignal("changed", "");
    if (!exported->publish("computer control settings"))
        return false;
    m_object = std::move(exported);
    return true;
}

std::optional<AgentApplication> RememberedApps::applicationOf(int64_t pid)
{
    if (pid <= 0)
        return std::nullopt;
    const std::optional<int64_t> parent = parentProcess(pid);
    if (!parent || *parent <= 1)
        return std::nullopt;
    const std::string exe = std::format("/proc/{}/exe", *parent);
    std::error_code error;
    const std::filesystem::path target = std::filesystem::read_symlink(exe, error);
    // A replaced or removed executable reads " (deleted)".
    if (error || target.empty() || target.native().ends_with(" (deleted)"))
        return std::nullopt;
    if (runsAnything(target.filename().native()))
        return std::nullopt;
    struct stat file {};
    if (stat(exe.c_str(), &file) != 0)
        return std::nullopt;
    return AgentApplication {target.native(),
                             std::format("{}:{}:{}.{}", file.st_dev, file.st_ino,
                                         file.st_ctim.tv_sec, file.st_ctim.tv_nsec)};
}

bool RememberedApps::allows(const AgentApplication &application) const
{
    return std::any_of(m_entries.begin(), m_entries.end(), [&application](const Entry &entry) {
        return entry.path == application.path && entry.file == application.file;
    });
}

void RememberedApps::remember(const AgentApplication &application, const std::string &agentName,
                              const std::string &appId)
{
    const auto it = std::find_if(m_entries.begin(), m_entries.end(), [&application](const Entry &entry) {
        return entry.path == application.path;
    });
    if (it != m_entries.end()) {
        it->file = application.file;
        it->name = agentName;
        it->appId = appId;
    } else {
        m_entries.push_back({application.path, application.file, agentName, appId});
    }
    save();
}

void RememberedApps::forget(const std::string &path)
{
    const auto end = std::remove_if(m_entries.begin(), m_entries.end(), [&path](const Entry &entry) {
        return entry.path == path;
    });
    if (end == m_entries.end())
        return;
    m_entries.erase(end, m_entries.end());
    save();
}

void RememberedApps::save()
{
    base::json::Value::Array apps;
    for (const Entry &entry : m_entries)
        apps.push_back(base::json::Value::object(
                {{"path", entry.path}, {"file", entry.file}, {"name", entry.name}, {"app_id", entry.appId}}));
    if (apps.empty())
        m_settings.remove(key);
    else
        m_settings.setValue(key, base::json::Value(std::move(apps)).serialize());
    if (!m_settings.sync())
        LM_WARNING(lingmoInput) << "Cannot persist computer control settings to" << m_settings.path();
    changed();
    if (m_object)
        m_object->emitSignal("changed");
}

} // namespace Lingmo::ComputerControl
