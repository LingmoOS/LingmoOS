// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "notifyserverapplet.h"
#include "notificationmanager.h"
#include "dbusadaptor.h"
#include "pluginfactory.h"

namespace notification {
Q_DECLARE_LOGGING_CATEGORY(notifyLog)
}

namespace notification {

NotifyServerApplet::NotifyServerApplet(QObject *parent)
        : DApplet(parent)
{

}

NotifyServerApplet::~NotifyServerApplet()
{
    qDebug(notifyLog) << "Exit notification server.";
    if (m_manager) {
        m_manager->deleteLater();
    }
    if (m_worker) {
        m_worker->exit();
        m_worker->wait();
        m_worker->deleteLater();
    }
}

bool NotifyServerApplet::load()
{
    return DApplet::load();
}

bool NotifyServerApplet::init()
{
    DApplet::init();

    m_manager = new NotificationManager();

    if (!m_manager->registerDbusService()) {
        qWarning(notifyLog) << QString("Can't register Notifications to the D-Bus object.");
        return false;
    }

    new DbusAdaptor(m_manager);
    new OCEANNotificationDbusAdaptor(m_manager);

    connect(m_manager, &NotificationManager::NotificationStateChanged, this, &NotifyServerApplet::notificationStateChanged);

    m_worker = new QThread();
    m_manager->moveToThread(m_worker);
    m_worker->start();
    return true;
}

void NotifyServerApplet::actionInvoked(qint64 id, uint bubbleId, const QString &actionKey)
{
    QMetaObject::invokeMethod(m_manager, "actionInvoked", Qt::DirectConnection, Q_ARG(qint64, id), Q_ARG(uint, bubbleId), Q_ARG(QString, actionKey));
}

void NotifyServerApplet::notificationClosed(qint64 id, uint bubbleId, uint reason)
{
    QMetaObject::invokeMethod(m_manager, "notificationClosed", Qt::DirectConnection, Q_ARG(qint64, id), Q_ARG(uint, bubbleId), Q_ARG(uint, reason));
}

QVariant NotifyServerApplet::appValue(const QString &appId, int configItem)
{
    return m_manager->GetAppInfo(appId, configItem);
}

void NotifyServerApplet::removeNotification(qint64 id)
{
    m_manager->removeNotification(id);
}

void NotifyServerApplet::removeNotifications(const QString &appName)
{
    m_manager->removeNotifications(appName);
}

void NotifyServerApplet::removeNotifications()
{
    m_manager->removeNotifications();
}

D_APPLET_CLASS(NotifyServerApplet)

}

#include "notifyserverapplet.moc"
