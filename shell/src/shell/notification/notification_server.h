// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <deque>
#include <memory>
#include <optional>
#include <set>
#include <string>

#include "base/dbus.h"
#include "base/signal.h"
#include "base/thread_pool.h"
#include "shell/common/applications.h"
#include "shell/common/pending_calls.h"
#include "shell/notification/notification.h"
#include "shell/notification/notification_images.h"

namespace Lingmo::Shell {

class NotificationSettings;
class TaskManager;

// org.freedesktop.Notifications at /org/freedesktop/Notifications, spec 1.2:
// body, hyperlinks and markup, static icons,
// actions and persistence; "image-data" (and the older "image_data" and
// "icon_data") before "image-path", "urgency", "value" and "desktop-entry".
// "transient" and "resident" are not acted on.
//
// Notify() answers at once with the id. What it takes to show the
// notification then happens without blocking: the sender's process is looked
// up on the bus, its desktop entry found, and an attached picture decoded and
// cached on the file thread. Notifications are shown in the order they came.
class NotificationServer
{
public:
    // The reasons NotificationClosed gives, numbered by the spec.
    enum class CloseReason {
        Expired = 1,
        DismissedByUser = 2,
        Revoked = 3,
    };

    // `tasks` may be null: senders are then named only by what they send.
    NotificationServer(base::dbus::Connection *bus, NotificationSettings &settings, Applications &applications,
                       TaskManager *tasks, std::string imageDirectory = notification_images::defaultDirectory());
    ~NotificationServer();

    NotificationServer(const NotificationServer &) = delete;
    NotificationServer &operator=(const NotificationServer &) = delete;

    // Exports the object, then takes the name.
    bool registerService();

    const std::string &imageDirectory() const { return m_imageDirectory; }

    // ActionInvoked, for a banner's button or its click.
    void invokeAction(uint32_t id, const std::string &actionKey);
    void closeNotification(uint32_t id, CloseReason reason);

    // Shown: a new notification, or one that replaced an open one.
    base::Signal<const Notification &> notificationAdded;
    base::Signal<uint32_t, const Notification &> notificationReplaced;
    base::Signal<uint32_t, CloseReason> notificationRemoved;

private:
    struct Pending {
        Notification notification;
        bool wasReplaced = false;
        std::optional<ImageHint> imageData;
        std::string imagePath;
        // The sender's application, when one was found.
        DesktopEntryPtr entry;
        // Looked up, decided and its picture cached.
        bool resolved = false;
        // Turned off for its application: it never shows.
        bool off = false;
        bool closeRequested = false;
        base::Job imageJob;
    };

    void notify(base::dbus::MethodCall &call);
    void closeFromBus(base::dbus::MethodCall &call);
    void lookUpSender(Pending *pending);
    void decide(Pending *pending);
    void finishImages(Pending *pending);
    // Shows the leading notifications whose lookups are done.
    void flush();
    bool isOpen(uint32_t id) const;
    Pending *pendingFor(uint32_t id) const;
    bool isPending(const Pending *pending) const;
    DesktopEntryPtr visibleOwner(DesktopEntryPtr entry) const;

    base::dbus::Connection *m_bus;
    NotificationSettings &m_settings;
    Applications &m_applications;
    TaskManager *m_tasks;
    std::string m_imageDirectory;
    std::unique_ptr<base::dbus::Object> m_object;
    PendingCalls m_calls;
    uint32_t m_nextId = 1;
    Notification m_last;
    // Shown and not closed yet.
    std::set<uint32_t> m_open;
    // In the order they came.
    std::deque<std::unique_ptr<Pending>> m_pending;
    // Waiting for the settings to be read.
    std::deque<Pending *> m_undecided;
    base::Subscription m_settingsReady;
};

} // namespace Lingmo::Shell

