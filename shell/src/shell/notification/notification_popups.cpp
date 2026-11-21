// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/notification/notification_popups.h"

#include <algorithm>
#include <cmath>

#include "core/main_loop.h"
#include "shell/notification/notification_card.h"
#include "shell/notification/notification_controls.h"
#include "shell/notification/notification_models.h"
#include "shell/notification/notification_motion.h"
#include "shell/shell.h"
#include "shell/shell_data.h"
#include "shell/shell_window.h"
#include "ui/embedded_window.h"
#include "ui/theme.h"
#include "wm/window_manager.h"
#include "toolkit/compositor_colors.h"

namespace Lingmo::Shell {

// The plate the card reads as (its fallback colour): its ink.
static ui::Color bannerSurface()
{
    return ui::color(toolkit::CompositorColor::NotificationBannerSurface);
}

// The window's content: the card over the whole banner, and the glass
// following the theme.
class NotificationPopups::Banner::Content : public ui::View
{
public:
    explicit Content(Banner *banner)
        : m_banner(banner)
    {
    }

protected:
    void layout() override
    {
        for (const auto &child : children())
            child->setGeometry(localRect());
    }

    void paint(ui::Painter &) override
    {
        const uint64_t generation = ui::Theme::generation();
        if (generation == m_theme)
            return;
        m_theme = generation;
        m_banner->updateGlass();
        m_banner->card()->setSurfaceColor(bannerSurface());
    }

private:
    Banner *m_banner;
    uint64_t m_theme = 0;
};

NotificationPopups::Banner::Banner(NotificationPopups *popups, const Notification &notification)
    : m_popups(popups)
    , m_notification(notification)
    , m_timer(core::mainLoop())
{
    Shell *shell = popups->m_shell;
    m_window = std::make_unique<ShellWindow>(shell, wm::WindowRole::Notification);
    m_window->setFocusPolicy(wm::FocusPolicy::NoFocus);
    if (const Shell::Output *output = shell->output({}))
        m_window->setOutput(output->name);

    ui::Window *window = m_window->window();
    auto content = std::make_unique<Content>(this);
    m_card = content->addChild<NotificationCard>(window);
    m_card->setSurfaceColor(bannerSurface());
    m_window->setContentView(std::move(content));
    m_card->setNotification(notification);

    m_stackY = std::make_unique<Tween>(window);
    m_stackY->onChanged = [this] { applyPosition(); };

    m_timer.setSingleShot(true);
    m_timer.onTimeout = [this] {
        m_timerRunning = false;
        m_popups->m_model.expired(id());
    };

    NotificationBanners &model = popups->m_model;
    const uint32_t bannerId = id();
    m_card->onClicked = [&model, bannerId] {
        if (const int row = model.rowOf(bannerId); row >= 0
            && model.notifications()[size_t(row)].hasAction("default"))
            model.invokeDefaultAction(bannerId);
        model.close(bannerId);
    };
    m_card->onCloseClicked = [&model, bannerId] { model.close(bannerId); };
    m_card->onActionClicked = [&model, bannerId](const std::string &key) {
        model.invokeAction(bannerId, key);
        model.close(bannerId);
    };
    m_card->onHoveredChanged = [this] { updateTimer(); };
    m_card->onImplicitHeightChanged = [this] {
        if (height() != m_height)
            m_popups->schedulePositioning();
    };

    m_height = height();
    m_window->setSize(base::Size(kWidth, m_height));
    updateGlass();
}

NotificationPopups::Banner::~Banner()
{
    m_timer.stop();
    m_stackY.reset();
    m_window.reset();
}

void NotificationPopups::Banner::setNotification(const Notification &notification)
{
    const bool restart = notification.summary != m_notification.summary || notification.body != m_notification.body
            || notification.value != m_notification.value;
    m_notification = notification;
    m_card->setNotification(notification);
    // A replacement starts the countdown over.
    if (restart && m_timerRunning)
        m_timer.start(timeout());
    updateTimer();
    if (height() != m_height)
        m_popups->schedulePositioning();
}

std::chrono::milliseconds NotificationPopups::Banner::timeout() const
{
    // Critical notifications and a timeout of 0 stay until dismissed.
    if (m_notification.urgency >= 2 || m_notification.timeout == 0)
        return std::chrono::milliseconds(0);
    if (m_notification.timeout > 0)
        return std::max(std::chrono::milliseconds(m_notification.timeout), kMinimumTimeout);
    return kDefaultTimeout;
}

bool NotificationPopups::Banner::isTimerRunning() const
{
    return m_timerRunning;
}

int NotificationPopups::Banner::height() const
{
    return int(m_card->implicitHeight(kWidth));
}

void NotificationPopups::Banner::updateTimer()
{
    // A timer that runs again starts from its full interval.
    const bool running = m_shown && timeout().count() > 0 && !m_card->isHovered();
    if (running == m_timerRunning)
        return;
    m_timerRunning = running;
    if (running)
        m_timer.start(timeout());
    else
        m_timer.stop();
}

void NotificationPopups::Banner::updateGlass()
{
    const bool dark = ui::Theme::current().darkMode;
    m_window->setBackgroundBlur(base::Rect(0, 0, kWidth, m_height), int(kRadius),
                                dark ? popupGlass(0.17, 0.17, 0.19, 0.6) : popupGlass(1, 1, 1, 0.6));
}

void NotificationPopups::Banner::place(const base::Point &position)
{
    const int height = this->height();
    if (height != m_height) {
        m_height = height;
        m_window->setSize(base::Size(kWidth, m_height));
        updateGlass();
    }
    // A banner coming on screen starts in its slot; the rest glide.
    if (!m_shown)
        m_placed = false;
    m_stack = position;
    m_stackY->setTarget(position.y(), m_placed && m_shown);
    m_placed = true;
    applyPosition();
}

void NotificationPopups::Banner::applyPosition()
{
    m_window->setPosition(base::Point(m_stack.x(), int(std::lround(m_stackY->value()))));
}

void NotificationPopups::Banner::setShown(bool shown)
{
    if (shown == m_shown)
        return;
    m_shown = shown;
    if (shown)
        m_window->show();
    else
        m_window->hide();
    updateTimer();
}

NotificationPopups::NotificationPopups(Shell *shell, NotificationBanners &model)
    : m_shell(shell)
    , m_model(model)
{
    m_subscriptions.push_back(model.rowsInserted.connect([this](int first, int count) { insert(first, count); }));
    m_subscriptions.push_back(model.rowsRemoved.connect([this](int first, int count) { remove(first, count); }));
    m_subscriptions.push_back(model.dataChanged.connect([this](int first, int last) { update(first, last); }));
    m_subscriptions.push_back(model.rowMoved.connect([this](int, int) { reset(); }));
    m_subscriptions.push_back(model.modelReset.connect([this] { reset(); }));
    // The work area moves with the status bar and the dock.
    m_subscriptions.push_back(shell->windowManager()->outputsChanged.connect([this] { schedulePositioning(); }));
    reset();
}

NotificationPopups::~NotificationPopups()
{
    *m_alive = false;
    m_subscriptions.clear();
    m_banners.clear();
}

void NotificationPopups::insert(int first, int count)
{
    const auto &rows = m_model.notifications();
    for (int row = first; row < first + count; ++row)
        m_banners.insert(m_banners.begin() + row, std::make_unique<Banner>(this, rows[size_t(row)]));
    positionPopups();
}

void NotificationPopups::remove(int first, int count)
{
    m_banners.erase(m_banners.begin() + first, m_banners.begin() + first + count);
    positionPopups();
}

void NotificationPopups::update(int first, int last)
{
    const auto &rows = m_model.notifications();
    for (int row = first; row <= last && row < int(m_banners.size()); ++row)
        m_banners[size_t(row)]->setNotification(rows[size_t(row)]);
}

void NotificationPopups::reset()
{
    m_banners.clear();
    insert(0, m_model.count());
}

void NotificationPopups::schedulePositioning()
{
    // Heights settle in the card's layout, which may not move windows.
    if (m_positionScheduled)
        return;
    m_positionScheduled = true;
    core::mainLoop()->post([this, alive = m_alive] {
        if (!*alive)
            return;
        m_positionScheduled = false;
        positionPopups();
    });
}

void NotificationPopups::positionPopups()
{
    const Shell::Output *output = m_shell->output({});
    if (!output)
        return;
    const base::Rect area = m_shell->workArea(*output);
    if (area.width() <= 0 || area.height() <= 0)
        return;

    int y = area.y() + kEdgeDistance;
    int shown = 0;
    // Newest on top, pushing the older ones down.
    for (auto it = m_banners.rbegin(); it != m_banners.rend(); ++it) {
        Banner *banner = it->get();
        const int height = banner->height();
        const int x = area.x() + area.width() - kEdgeDistance - kWidth;
        // Never more than 60 % of the screen, but always at least one banner.
        const bool fits = y + height <= area.y() + area.height() * kMaximumScreenFill;
        if (shown > 0 && !fits) {
            banner->setShown(false);
            continue;
        }
        banner->window()->setOutput(output->name);
        banner->place(base::Point(x, y));
        banner->setShown(true);
        y += height + kSpacing;
        ++shown;
    }
}

} // namespace Lingmo::Shell
