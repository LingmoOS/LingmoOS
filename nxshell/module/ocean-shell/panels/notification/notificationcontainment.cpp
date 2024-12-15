// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "notificationcontainment.h"

#include <QLoggingCategory>

#include "pluginfactory.h"

DS_BEGIN_NAMESPACE
Q_LOGGING_CATEGORY(notifyLog, "ocean.shell.notification")

NotificationContainment::NotificationContainment(QObject *parent)
    : DContainment(parent)
{

}

NotificationContainment::~NotificationContainment()
{
}

bool NotificationContainment::load()
{
    return DContainment::load();
}

D_APPLET_CLASS(NotificationContainment)

DS_END_NAMESPACE

#include "notificationcontainment.moc"
