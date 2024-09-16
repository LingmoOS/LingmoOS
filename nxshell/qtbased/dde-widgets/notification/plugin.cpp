// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "plugin.h"
#include "common/helper.hpp"
#include "accessible/accessible.h"

#include <QAccessible>

QString NotificationWidgetPlugin::title() const
{
    return tr("Notification");
}

QString NotificationWidgetPlugin::description() const
{
    return tr("Notification Center");
}

IWidget *NotificationWidgetPlugin::createWidget()
{
    return new NotificationWidget();
}

bool NotificationWidget::initialize(const QStringList &arguments) {
    Q_UNUSED(arguments);
    static bool hasLoaded = false;
    if (!hasLoaded)
        hasLoaded = BuildinWidgetsHelper::instance()->loadTranslator("dde-widgets-notification_");

    // enable accessible
    QAccessible::installFactory(notificationAccessibleFactory);

    m_persistence = new PersistenceObserver();
    m_view.reset(new NotifyCenterWidget(m_persistence));
    m_view->setFixedWidth(handler()->size().width());
    return true;
}

void NotificationWidget::delayInitialize()
{
}


QIcon NotificationWidgetPlugin::logo() const
{
    return QIcon::fromTheme("notifications");
}

QStringList NotificationWidgetPlugin::contributors() const
{
    return {BuildinWidgetsHelper::instance()->contributor()};
}
