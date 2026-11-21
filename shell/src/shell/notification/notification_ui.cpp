// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/notification/notification_ui.h"

#include "shell/notification/notification_centre.h"
#include "shell/notification/notification_popups.h"
#include "shell/notification/notifications.h"
#include "shell/shell_window.h"

namespace Lingmo::Shell {

NotificationUi::NotificationUi(Shell *shell, std::unique_ptr<Notifications> notifications)
    : m_notifications(std::move(notifications))
    , m_popups(std::make_unique<NotificationPopups>(shell, m_notifications->banners()))
    , m_centre(std::make_unique<NotificationCentre>(shell, m_notifications->history(), m_notifications->banners()))
{
    m_openChanged = m_centre->openChanged.connect([this] { panelsChanged(); });
}

NotificationUi::~NotificationUi()
{
    m_openChanged.reset();
    // The views go before the models they show.
    m_centre.reset();
    m_popups.reset();
    m_notifications.reset();
}

bool NotificationUi::registerServices()
{
    return m_notifications->registerServices();
}

bool NotificationUi::hasPanel(Panel panel) const
{
    return panel == Panel::NotificationCenter;
}

void NotificationUi::openPanel(Panel panel, ShellWindow *bar, const base::Rect &anchor)
{
    if (panel != Panel::NotificationCenter || !bar)
        return;
    // The clock item's plate starts 3 in from its rect (its leftInset).
    m_centre->open(bar, base::Point(bar->geometry().x() + anchor.x() + 3, bar->geometry().bottom()));
}

void NotificationUi::closePanels()
{
    m_centre->close();
}

bool NotificationUi::isOpen(Panel panel) const
{
    return panel == Panel::NotificationCenter && m_centre->isOpen();
}

} // namespace Lingmo::Shell
