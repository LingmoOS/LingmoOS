// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <cstdint>
#include <set>
#include <string>
#include <vector>

#include "base/signal.h"
#include "base/thread_pool.h"
#include "base/timer.h"
#include "shell/common/list_model.h"
#include "shell/notification/notification.h"
#include "shell/notification/notification_server.h"

namespace Lingmo::Shell {

class NotificationSettings;

// "Now", "5 minutes ago", "1 day ago", or the local date and time in the
// short format after ten days, as the history's cards show the time.
std::string friendlyTime(std::optional<int64_t> created, int64_t nowMs);

// Notifications that left the screen, newest first with each application's
// rows kept together, persisted across sessions in
// ~/.config/lingmo/notification-history.json, an array of
// Notification::toJson() records. Read and written on the file thread.
class NotificationHistory : public ListModelSignals
{
public:
    static constexpr size_t kLimit = 200;

    NotificationHistory(std::string path, std::string imageDirectory);
    ~NotificationHistory();

    NotificationHistory(const NotificationHistory &) = delete;
    NotificationHistory &operator=(const NotificationHistory &) = delete;

    static std::string defaultPath();

    bool isLoaded() const { return m_loaded; }
    const std::vector<Notification> &notifications() const { return m_notifications; }
    int count() const { return int(m_notifications.size()); }
    // friendlyTime() of the row, now.
    std::string createdText(int row) const;

    void add(const Notification &notification);
    void remove(std::vector<int> rows);
    void clearAll();
    // The relative times went stale while nobody looked: every row changed.
    void updateTime();

    base::Signal<> loaded;

private:
    void save();
    void removeImages(const Notification &notification);

    std::string m_path;
    std::string m_imageDirectory;
    std::vector<Notification> m_notifications;
    bool m_loaded = false;
    base::Job m_load;
};

// The notifications currently shown as banners, oldest first.
class NotificationBanners : public ListModelSignals
{
public:
    // Past this many banners the oldest goes to the history.
    static constexpr int kMaxBanners = 5;

    NotificationBanners(NotificationServer &server, NotificationSettings &settings, NotificationHistory &history);

    const std::vector<Notification> &notifications() const { return m_notifications; }
    int count() const { return int(m_notifications.size()); }
    int rowOf(uint32_t id) const;

    // Set while the notification centre is open: the banners on screen move
    // into the history, and new notifications go straight there.
    bool bannersSuppressed() const { return m_bannersSuppressed; }
    void setBannersSuppressed(bool suppressed);

    base::Signal<std::string> soundRequested;

    // The banner timed out; it moves to the history.
    void expired(uint32_t id);
    void close(uint32_t id);
    void invokeDefaultAction(uint32_t id);
    void invokeAction(uint32_t id, const std::string &actionKey);

private:
    void add(const Notification &notification, bool playSound = true);
    void replace(uint32_t id, const Notification &notification);
    void onRemoved(uint32_t id, NotificationServer::CloseReason reason);
    void removePending();

    NotificationServer &m_server;
    NotificationSettings &m_settings;
    NotificationHistory &m_history;
    std::vector<Notification> m_notifications;
    // Rows go on the next turn of the loop: the banner that asked may still
    // be running the handler that got it closed.
    std::set<uint32_t> m_pendingRemovals;
    base::Timer m_removalTimer;
    bool m_bannersSuppressed = false;
    std::vector<base::Subscription> m_subscriptions;
};

} // namespace Lingmo::Shell

