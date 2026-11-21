// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <chrono>
#include <cstdint>
#include <memory>
#include <vector>

#include "base/geometry.h"
#include "base/signal.h"
#include "base/timer.h"
#include "shell/notification/notification.h"

namespace Lingmo::Shell {

class NotificationBanners;
class NotificationCard;
class Shell;
class ShellWindow;
class Tween;

// The banners: one
// window of role Notification per banner, 380 wide on PopupGlass, stacked
// newest first down the primary output's top right corner, 6 from its
// work area's edges and 9 apart, taking at most 60 % of its height (the
// first always shows). Each times out after its notification's timeout (at
// least 3 s, 7 s by default, never for critical ones), paused while the
// pointer is over it; a banner already on screen glides to a new slot.
class NotificationPopups
{
public:
    static constexpr int kWidth = 380;
    // PanelStyle.popupMargin and FishUI's smallSpacing * 1.5.
    static constexpr int kEdgeDistance = 6;
    static constexpr int kSpacing = 9;
    static constexpr double kMaximumScreenFill = 0.6;
    static constexpr double kRadius = 18;
    static constexpr std::chrono::milliseconds kDefaultTimeout {7000};
    static constexpr std::chrono::milliseconds kMinimumTimeout {3000};

    // One banner's window.
    class Banner
    {
    public:
        Banner(NotificationPopups *popups, const Notification &notification);
        ~Banner();

        uint32_t id() const { return m_notification.id; }
        const Notification &notification() const { return m_notification; }
        void setNotification(const Notification &notification);

        ShellWindow *window() const { return m_window.get(); }
        NotificationCard *card() const { return m_card; }
        // On screen: the stack had room for it.
        bool isShown() const { return m_shown; }
        // Zero for a banner that stays until it is dismissed.
        std::chrono::milliseconds timeout() const;
        bool isTimerRunning() const;
        // The card's height at the banner's width.
        int height() const;
        // Where the stack put it; the window glides there.
        base::Point stackPosition() const { return m_stack; }

        void place(const base::Point &position);
        void setShown(bool shown);
        void updateGlass();

    private:
        class Content;

        void updateTimer();
        void applyPosition();

        NotificationPopups *m_popups;
        Notification m_notification;
        std::unique_ptr<ShellWindow> m_window;
        NotificationCard *m_card = nullptr;
        std::unique_ptr<Tween> m_stackY;
        base::Point m_stack;
        bool m_placed = false;
        bool m_shown = false;
        bool m_timerRunning = false;
        base::Timer m_timer;
        int m_height = 0;
    };

    NotificationPopups(Shell *shell, NotificationBanners &model);
    ~NotificationPopups();

    NotificationPopups(const NotificationPopups &) = delete;
    NotificationPopups &operator=(const NotificationPopups &) = delete;

    // Oldest first, as the model's rows.
    const std::vector<std::unique_ptr<Banner>> &banners() const { return m_banners; }

    // Places every banner again, newest first.
    void positionPopups();

private:
    void insert(int first, int count);
    void remove(int first, int count);
    void update(int first, int last);
    void reset();
    void schedulePositioning();
    friend class Banner;

    Shell *m_shell;
    NotificationBanners &m_model;
    std::vector<std::unique_ptr<Banner>> m_banners;
    bool m_positionScheduled = false;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
    std::vector<base::Subscription> m_subscriptions;
};

} // namespace Lingmo::Shell

