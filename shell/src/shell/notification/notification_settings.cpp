// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/notification/notification_settings.h"

#include <algorithm>

#include "base/locale.h"
#include "base/settings.h"
#include "base/url.h"
#include "shell/common/applications.h"
#include "shell/common/file_thread.h"

namespace Lingmo::Shell {

static NotificationSettings::ApplicationMode toApplicationMode(int64_t value)
{
    switch (value) {
    case NotificationSettings::CenterOnly:
        return NotificationSettings::CenterOnly;
    case NotificationSettings::Off:
        return NotificationSettings::Off;
    default:
        return NotificationSettings::Banners;
    }
}

NotificationSettings::NotificationSettings(Applications &applications, std::string path)
    : m_applications(applications)
    , m_path(std::move(path))
{
    m_load = runOnFileThread([path = m_path] { return load(path); },
                             [this](Stored stored) {
                                 // A change made while the file was read stays.
                                 for (auto &[key, application] : stored.applications)
                                     m_entries.try_emplace(key, std::move(application));
                                 if (!m_ready)
                                     m_doNotDisturb = m_doNotDisturb || stored.doNotDisturb;
                                 m_ready = true;
                                 ready();
                                 if (m_doNotDisturb)
                                     doNotDisturbChanged();
                                 applicationsChanged();
                             });

    // Installing or removing an application changes which ones are listed.
    m_applicationsChanged = m_applications.changed.connect([this] { applicationsChanged(); });
}

NotificationSettings::Stored NotificationSettings::load(const std::string &path)
{
    const base::Settings settings(path);
    Stored stored;
    stored.doNotDisturb = settings.boolValue("Notifications/DoNotDisturb", false);
    for (const std::string &group : settings.childGroups("Notifications/Applications")) {
        const std::string prefix = "Notifications/Applications/" + group + '/';
        Application application;
        application.key = base::url::percentDecoded(group);
        application.name = settings.stringValue(prefix + "Name");
        application.icon = settings.stringValue(prefix + "Icon");
        application.mode = toApplicationMode(settings.intValue(prefix + "Mode"));
        stored.applications.emplace(application.key, std::move(application));
    }
    return stored;
}

void NotificationSettings::setDoNotDisturb(bool enabled)
{
    if (m_doNotDisturb == enabled)
        return;
    m_doNotDisturb = enabled;
    writeSettings(m_path, [enabled](base::Settings &settings) {
        settings.setValue("Notifications/DoNotDisturb", enabled);
    });
    doNotDisturbChanged();
}

NotificationSettings::ApplicationMode NotificationSettings::applicationMode(const std::string &key) const
{
    const auto it = m_entries.find(key);
    return it == m_entries.end() ? Banners : it->second.mode;
}

void NotificationSettings::setApplicationMode(const std::string &key, int mode)
{
    const auto it = m_entries.find(key);
    if (key.empty() || it == m_entries.end() || it->second.mode == toApplicationMode(mode))
        return;

    it->second.mode = toApplicationMode(mode);
    const std::string group = applicationGroup(key);
    const int value = int(it->second.mode);
    writeSettings(m_path, [group, value](base::Settings &settings) { settings.setValue(group + "/Mode", value); });
    applicationsChanged();
}

void NotificationSettings::noteApplication(const std::string &key, const std::string &name,
                                           const std::string &icon)
{
    if (key.empty() || !isInstalled(key))
        return;

    Application &application = m_entries[key];
    application.key = key;
    if (application.name == name && application.icon == icon)
        return;
    application.name = name;
    application.icon = icon;

    const std::string group = applicationGroup(key);
    writeSettings(m_path, [group, name, icon](base::Settings &settings) {
        settings.setValue(group + "/Name", name);
        settings.setValue(group + "/Icon", icon);
    });
    applicationsChanged();
}

std::vector<NotificationSettings::Application> NotificationSettings::applications() const
{
    std::vector<Application> list;

    // An uninstalled application keeps its mode, in case it comes back.
    for (const auto &[key, stored] : m_entries) {
        const DesktopEntryPtr entry = m_applications.byId(key);
        if (!entry || entry->hidden)
            continue;

        Application application = stored;
        // The desktop entry follows the current language and icon. A
        // NoDisplay helper's Name is often technical, so its sender's own
        // name stays.
        if (!entry->noDisplay)
            application.name = entry->name;
        if (!entry->icon.empty())
            application.icon = entry->icon;
        if (application.name.empty())
            application.name = key;
        list.push_back(std::move(application));
    }

    const base::Locale &locale = base::Locale::system();
    std::sort(list.begin(), list.end(), [&locale](const Application &a, const Application &b) {
        return locale.compare(a.name, b.name) < 0;
    });
    return list;
}

// A Hidden entry only masks a removed system one.
bool NotificationSettings::isInstalled(const std::string &key) const
{
    const DesktopEntryPtr entry = m_applications.byId(key);
    return entry && !entry->hidden;
}

// Keys are desktop ids or application names, which may hold a '/'.
std::string NotificationSettings::applicationGroup(const std::string &key)
{
    return "Notifications/Applications/" + base::url::percentEncoded(key);
}

} // namespace Lingmo::Shell
