// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <map>
#include <string>
#include <vector>

#include "base/signal.h"
#include "base/thread_pool.h"

namespace Lingmo::Shell {

class Applications;

// Do Not Disturb and how each application's notifications reach the user, in
// the [Notifications] group of shell.conf. Read on the file thread; nothing is
// decided before isReady().
class NotificationSettings
{
public:
    // How one application's notifications reach the user. The values are stored.
    enum ApplicationMode {
        Banners = 0,
        CenterOnly = 1,
        Off = 2,
    };

    struct Application {
        // The desktop id or the lower-cased name the notifications came with.
        std::string key;
        std::string name;
        std::string icon;
        ApplicationMode mode = Banners;
    };

    // `path` is shell.conf; `applications` must outlive the settings.
    NotificationSettings(Applications &applications, std::string path);

    bool isReady() const { return m_ready; }

    bool doNotDisturb() const { return m_doNotDisturb; }
    void setDoNotDisturb(bool enabled);

    // `key` is Notification::applicationKey(); an unknown one shows banners.
    ApplicationMode applicationMode(const std::string &key) const;
    void setApplicationMode(const std::string &key, int mode);

    // Remembers an installed application that sent a notification, so it can
    // be listed; senders without a desktop entry are not.
    void noteApplication(const std::string &key, const std::string &name, const std::string &icon);
    // Noted applications that are still installed, sorted by name in the
    // user's language. Their names and icons follow the desktop entry.
    std::vector<Application> applications() const;

    base::Signal<> ready;
    base::Signal<> doNotDisturbChanged;
    base::Signal<> applicationsChanged;

private:
    struct Stored {
        bool doNotDisturb = false;
        std::map<std::string, Application> applications;
    };

    static Stored load(const std::string &path);
    static std::string applicationGroup(const std::string &key);
    bool isInstalled(const std::string &key) const;

    Applications &m_applications;
    std::string m_path;
    bool m_ready = false;
    bool m_doNotDisturb = false;
    std::map<std::string, Application> m_entries;
    base::Job m_load;
    base::Subscription m_applicationsChanged;
};

} // namespace Lingmo::Shell

