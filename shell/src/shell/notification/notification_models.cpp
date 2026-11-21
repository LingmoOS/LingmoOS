// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/notification/notification_models.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>

#include <sys/stat.h>

#include "base/file.h"
#include "base/i18n.h"
#include "base/locale.h"
#include "core/preferences.h"
#include "shell/common/file_thread.h"
#include "shell/notification/notification_images.h"
#include "shell/notification/notification_settings.h"

using Lingmo::i18n::tr;
using Lingmo::i18n::trn;

namespace Lingmo::Shell {


// The elapsed time rounded as qRound64 of a float, half away from zero.
static int64_t round64(float value)
{
    return value >= 0.0f ? int64_t(value + 0.5f) : int64_t(value - 0.5f);
}

std::string friendlyTime(std::optional<int64_t> created, int64_t nowMs)
{
    if (!created)
        return {};

    const int64_t seconds = (nowMs - *created) / 1000;
    const int64_t minutes = round64(float(seconds) / 60.0f);
    if (minutes < 1)
        return tr("Now");
    if (minutes < 60)
        return trn("{n} minute ago", "{n} minutes ago", minutes);

    const int64_t hours = round64(float(minutes) / 60.0f);
    if (hours < 24)
        return trn("{n} hour ago", "{n} hours ago", hours);

    const int64_t days = round64(float(hours) / 24.0f);
    if (days <= 10)
        return trn("{n} day ago", "{n} days ago", days);

    // Stored in UTC; the date shown is the local one.
    const int64_t createdSeconds = *created >= 0 ? *created / 1000 : (*created - 999) / 1000;
    return base::Locale::system().toString(base::DateTime::fromSecondsSinceEpoch(createdSeconds),
                                           base::Locale::FormatType::Short);
}

static int64_t currentMs()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
            .count();
}

NotificationHistory::NotificationHistory(std::string path, std::string imageDirectory)
    : m_path(std::move(path))
    , m_imageDirectory(std::move(imageDirectory))
{
    m_load = runOnFileThread(
            [path = m_path] {
                std::vector<Notification> notifications;
                const std::optional<std::string> data = base::file::read(path);
                if (!data)
                    return notifications;
                const std::optional<base::json::Value> stored = base::json::Value::parse(*data);
                if (!stored || !stored->isArray())
                    return notifications;
                for (const base::json::Value &record : stored->toArray()) {
                    if (record.isObject())
                        notifications.push_back(Notification::fromJson(record));
                }
                return notifications;
            },
            [this](std::vector<Notification> stored) {
                // What came while the file was read is newer.
                for (Notification &notification : stored)
                    m_notifications.push_back(std::move(notification));
                if (m_notifications.size() > kLimit)
                    m_notifications.resize(kLimit);

                // Each application's rows together, behind its newest one.
                std::vector<std::string> order;
                for (const Notification &notification : m_notifications) {
                    const std::string key = notification.groupKey();
                    if (std::find(order.begin(), order.end(), key) == order.end())
                        order.push_back(key);
                }
                const auto rank = [&order](const Notification &notification) {
                    return std::find(order.begin(), order.end(), notification.groupKey()) - order.begin();
                };
                std::stable_sort(m_notifications.begin(), m_notifications.end(),
                                 [&rank](const Notification &a, const Notification &b) { return rank(a) < rank(b); });

                std::vector<std::string> images;
                for (const Notification &notification : m_notifications) {
                    images.push_back(notification.image);
                    images.push_back(notification.appIcon);
                }
                fileThread().post([directory = m_imageDirectory, images] {
                    notification_images::prune(directory, images);
                });

                m_loaded = true;
                modelReset();
                loaded();
            });
}

NotificationHistory::~NotificationHistory() = default;

std::string NotificationHistory::defaultPath()
{
    return preferencesDirectory() + "/notification-history.json";
}

std::string NotificationHistory::createdText(int row) const
{
    if (row < 0 || row >= count())
        return {};
    return friendlyTime(m_notifications[size_t(row)].created, currentMs());
}

void NotificationHistory::removeImages(const Notification &notification)
{
    fileThread().post([directory = m_imageDirectory, image = notification.image, icon = notification.appIcon] {
        notification_images::remove(directory, image);
        notification_images::remove(directory, icon);
    });
}

void NotificationHistory::add(const Notification &notification)
{
    // A replaced notification takes the place of the version already here.
    // Ids are only unique within a session, and rows loaded from disk carry
    // none.
    for (size_t row = 0; notification.id && row < m_notifications.size(); ++row) {
        const Notification &old = m_notifications[row];
        if (old.id != notification.id || old.service != notification.service)
            continue;
        if (old.image != notification.image || old.appIcon != notification.appIcon) {
            fileThread().post([directory = m_imageDirectory, oldImage = old.image, oldIcon = old.appIcon,
                               image = notification.image, icon = notification.appIcon] {
                if (oldImage != image)
                    notification_images::remove(directory, oldImage);
                if (oldIcon != icon)
                    notification_images::remove(directory, oldIcon);
            });
        }
        m_notifications.erase(m_notifications.begin() + ptrdiff_t(row));
        rowsRemoved(int(row), 1);
        break;
    }

    // The application's earlier rows move up with the new one, so the most
    // recently active application always heads the list.
    const std::string key = notification.groupKey();
    const auto first = std::find_if(m_notifications.begin(), m_notifications.end(),
                                    [&key](const Notification &n) { return n.groupKey() == key; });
    if (first != m_notifications.end()) {
        const size_t from = size_t(first - m_notifications.begin());
        size_t to = from;
        while (to + 1 < m_notifications.size() && m_notifications[to + 1].groupKey() == key)
            ++to;
        if (from > 0) {
            std::rotate(m_notifications.begin(), m_notifications.begin() + ptrdiff_t(from),
                        m_notifications.begin() + ptrdiff_t(to + 1));
            for (size_t i = 0; i <= to - from; ++i)
                rowMoved(int(from + i), int(i));
        }
    }

    m_notifications.insert(m_notifications.begin(), notification);
    rowsInserted(0, 1);

    if (m_notifications.size() > kLimit) {
        const int removed = int(m_notifications.size() - kLimit);
        for (size_t row = kLimit; row < m_notifications.size(); ++row)
            removeImages(m_notifications[row]);
        m_notifications.resize(kLimit);
        rowsRemoved(int(kLimit), removed);
    }

    save();
}

void NotificationHistory::remove(std::vector<int> rows)
{
    std::sort(rows.begin(), rows.end(), std::greater<int>());
    rows.erase(std::unique(rows.begin(), rows.end()), rows.end());

    for (int row : rows) {
        if (row < 0 || row >= count())
            continue;
        removeImages(m_notifications[size_t(row)]);
        m_notifications.erase(m_notifications.begin() + row);
        rowsRemoved(row, 1);
    }

    save();
}

void NotificationHistory::clearAll()
{
    for (const Notification &notification : m_notifications)
        removeImages(notification);
    m_notifications.clear();
    modelReset();
    save();
}

void NotificationHistory::updateTime()
{
    if (!m_notifications.empty())
        dataChanged(0, count() - 1);
}

void NotificationHistory::save()
{
    // Before the file is read, a write would lose what it holds.
    if (!m_loaded)
        return;

    base::json::Value::Array records;
    for (const Notification &notification : m_notifications)
        records.push_back(notification.toJson());

    fileThread().post([path = m_path, data = base::json::Value(std::move(records)).serialize()] {
        // The history holds message text, so only the user may read it.
        if (base::file::writeAtomically(path, data))
            chmod(path.c_str(), 0600);
    });
}

NotificationBanners::NotificationBanners(NotificationServer &server, NotificationSettings &settings,
                                         NotificationHistory &history)
    : m_server(server)
    , m_settings(settings)
    , m_history(history)
{
    m_removalTimer.setSingleShot(true);
    m_removalTimer.onTimeout = [this] { removePending(); };

    m_subscriptions.push_back(server.notificationAdded.connect([this](const Notification &n) { add(n); }));
    m_subscriptions.push_back(server.notificationReplaced.connect(
            [this](uint32_t id, const Notification &n) { replace(id, n); }));
    m_subscriptions.push_back(server.notificationRemoved.connect(
            [this](uint32_t id, NotificationServer::CloseReason reason) { onRemoved(id, reason); }));
}

int NotificationBanners::rowOf(uint32_t id) const
{
    for (size_t row = 0; row < m_notifications.size(); ++row) {
        if (m_notifications[row].id == id)
            return int(row);
    }
    return -1;
}

void NotificationBanners::setBannersSuppressed(bool suppressed)
{
    if (m_bannersSuppressed == suppressed)
        return;
    m_bannersSuppressed = suppressed;

    if (suppressed) {
        std::vector<uint32_t> ids;
        for (const Notification &notification : m_notifications) {
            if (!m_pendingRemovals.count(notification.id))
                ids.push_back(notification.id);
        }
        for (uint32_t id : ids)
            m_server.closeNotification(id, NotificationServer::CloseReason::Expired);
    }
}

void NotificationBanners::expired(uint32_t id)
{
    if (rowOf(id) >= 0)
        m_server.closeNotification(id, NotificationServer::CloseReason::Expired);
}

void NotificationBanners::close(uint32_t id)
{
    if (rowOf(id) >= 0)
        m_server.closeNotification(id, NotificationServer::CloseReason::DismissedByUser);
}

void NotificationBanners::invokeDefaultAction(uint32_t id)
{
    invokeAction(id, "default");
}

void NotificationBanners::invokeAction(uint32_t id, const std::string &actionKey)
{
    const int row = rowOf(id);
    if (row >= 0 && m_notifications[size_t(row)].hasAction(actionKey))
        m_server.invokeAction(id, actionKey);
}

void NotificationBanners::add(const Notification &notification, bool playSound)
{
    const bool critical = notification.urgency >= 2;
    if (m_bannersSuppressed
        || (!critical
            && (m_settings.doNotDisturb()
                || m_settings.applicationMode(notification.applicationKey()) == NotificationSettings::CenterOnly))) {
        m_history.add(notification);
        return;
    }

    // Each banner is a window of its own; past the cap the oldest one goes to
    // the history as if its time had run out.
    int live = 0;
    for (const Notification &banner : m_notifications)
        live += m_pendingRemovals.count(banner.id) ? 0 : 1;
    for (size_t row = 0; live >= kMaxBanners && row < m_notifications.size(); ++row) {
        const uint32_t id = m_notifications[row].id;
        if (m_pendingRemovals.count(id))
            continue;
        expired(id);
        --live;
    }

    if (playSound && !notification.suppressSound && !m_settings.doNotDisturb()
        && m_settings.applicationMode(notification.applicationKey()) == NotificationSettings::Banners) {
        const std::string event = notification.soundName.empty() ? "message-new-instant" : notification.soundName;
        soundRequested(event);
    }
    m_notifications.push_back(notification);
    rowsInserted(count() - 1, 1);
}

void NotificationBanners::replace(uint32_t id, const Notification &notification)
{
    const int row = rowOf(id);
    if (row < 0 || m_pendingRemovals.count(id)) {
        add(notification, false);
        return;
    }

    Notification &current = m_notifications[size_t(row)];
    fileThread().post([directory = m_server.imageDirectory(), oldImage = current.image, oldIcon = current.appIcon,
                       image = notification.image, icon = notification.appIcon] {
        if (oldImage != image)
            notification_images::remove(directory, oldImage);
        if (oldIcon != icon)
            notification_images::remove(directory, oldIcon);
    });

    // The banner keeps its place in the stack and the time it first appeared.
    const std::optional<int64_t> created = current.created;
    current = notification;
    current.created = created;
    dataChanged(row, row);
}

void NotificationBanners::onRemoved(uint32_t id, NotificationServer::CloseReason reason)
{
    const int row = rowOf(id);
    if (row < 0 || m_pendingRemovals.count(id))
        return;

    const Notification &notification = m_notifications[size_t(row)];
    if (reason == NotificationServer::CloseReason::Expired) {
        // The application may be gone by the time the history is looked at.
        Notification archived = notification;
        archived.actions.clear();
        m_history.add(archived);
    } else {
        fileThread().post([directory = m_server.imageDirectory(), image = notification.image,
                           icon = notification.appIcon] {
            notification_images::remove(directory, image);
            notification_images::remove(directory, icon);
        });
    }

    m_pendingRemovals.insert(id);
    m_removalTimer.start(std::chrono::nanoseconds(0));
}

void NotificationBanners::removePending()
{
    for (int row = count() - 1; row >= 0; --row) {
        if (!m_pendingRemovals.count(m_notifications[size_t(row)].id))
            continue;
        m_notifications.erase(m_notifications.begin() + row);
        rowsRemoved(row, 1);
    }
    m_pendingRemovals.clear();
}

} // namespace Lingmo::Shell
