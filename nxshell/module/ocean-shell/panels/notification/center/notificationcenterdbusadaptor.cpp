// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "notificationcenterdbusadaptor.h"
#include "notificationcenterproxy.h"

namespace notification {

NotificationCenterDBusAdaptor::NotificationCenterDBusAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    setAutoRelaySignals(true);
}

void NotificationCenterDBusAdaptor::Toggle()
{
    return impl()->Toggle();
}

void NotificationCenterDBusAdaptor::Show()
{
    return impl()->Show();
}

void NotificationCenterDBusAdaptor::Hide()
{
    return impl()->Hide();
}

NotificationCenterProxy *NotificationCenterDBusAdaptor::impl() const
{
    return qobject_cast<NotificationCenterProxy *>(parent());
}

} // notification
