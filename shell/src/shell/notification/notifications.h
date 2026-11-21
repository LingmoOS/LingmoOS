// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <memory>
#include <string>

#include "base/dbus.h"
#include "base/signal.h"
#include "shell/notification/notification_models.h"
#include "shell/notification/notification_server.h"
#include "shell/notification/notification_settings.h"

namespace Lingmo::Shell {

class Applications;
class TaskManager;

// The notifications component without its views: the
// org.freedesktop.Notifications server, the banners on screen, the history,
// the per-application settings, and org.lingmo.Notification at
// /Notification, which Settings and the session call.
class Notifications
{
public:
    struct Files {
        // shell.conf
        std::string settings;
        std::string history;
        std::string images;

        static Files defaults();
    };

    Notifications(base::dbus::Connection *bus, Applications &applications, TaskManager *tasks,
                  Files files = Files::defaults());
    ~Notifications();

    Notifications(const Notifications &) = delete;
    Notifications &operator=(const Notifications &) = delete;

    // org.freedesktop.Notifications, then org.lingmo.Notification.
    bool registerServices();

    NotificationSettings &settings() { return m_settings; }
    NotificationHistory &history() { return m_history; }
    NotificationBanners &banners() { return m_banners; }

private:
    base::dbus::Connection *m_bus;
    NotificationSettings m_settings;
    NotificationServer m_server;
    NotificationHistory m_history;
    NotificationBanners m_banners;
    std::unique_ptr<base::dbus::Object> m_object;
    std::vector<base::Subscription> m_subscriptions;
};

} // namespace Lingmo::Shell

