// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbusadaptor.h"
#include "notificationmanager.h"

#include <QLoggingCategory>
namespace notification {
Q_DECLARE_LOGGING_CATEGORY(notifyLog)
}
namespace notification {

DbusAdaptor::DbusAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    setAutoRelaySignals(true);
}

QStringList DbusAdaptor::GetCapabilities()
{
    qInfo(notifyLog) << "GetCapabilities";
    return manager()->GetCapabilities();
}

uint DbusAdaptor::Notify(const QString &appName, uint replacesId, const QString &appIcon, const QString &summary,
                         const QString &body, const QStringList &actions, const QVariantMap &hints,
                         int expireTimeout)
{
    uint id = manager()->Notify(appName, replacesId, appIcon, summary, body, actions, hints, expireTimeout);
    if (id == 0) {
        QDBusError error(QDBusError::InternalError, "Notify failed.");
        QDBusMessage reply = QDBusMessage::createError(error);

        QDBusConnection::sessionBus().send(reply);
    }

    return id;
}

void DbusAdaptor::CloseNotification(uint id)
{
    qInfo(notifyLog) << "Close notification" << id;
    manager()->CloseNotification(id);
}

void DbusAdaptor::GetServerInformation(QString &name, QString &vendor, QString &version, QString &specVersion)
{
    qInfo(notifyLog) << "GetServerInformation";
    manager()->GetServerInformation(name, vendor, version, specVersion);
}

NotificationManager *DbusAdaptor::manager() const
{
    return qobject_cast<NotificationManager *>(parent());
}

OCEANNotificationDbusAdaptor::OCEANNotificationDbusAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    setAutoRelaySignals(true);
}

QStringList OCEANNotificationDbusAdaptor::GetCapabilities()
{
    qInfo(notifyLog) << "GetCapabilities";
    return manager()->GetCapabilities();
}

uint OCEANNotificationDbusAdaptor::Notify(const QString &appName, uint replacesId, const QString &appIcon,
    const QString &summary, const QString &body, const QStringList &actions, const QVariantMap &hints,
    int expireTimeout)
{
    uint id = manager()->Notify(appName, replacesId, appIcon, summary, body, actions, hints, expireTimeout);
    if (id == 0) {
        QDBusError error(QDBusError::InternalError, "Notify failed.");
        QDBusMessage reply = QDBusMessage::createError(error);

        QDBusConnection::sessionBus().send(reply);
    }

    return id;
}

void OCEANNotificationDbusAdaptor::CloseNotification(uint id)
{
    qInfo(notifyLog) << "Close Notification" << id;
    manager()->CloseNotification(id);
}

void OCEANNotificationDbusAdaptor::GetServerInformation(QString &name, QString &vendor, QString &version, QString &specVersion)
{
    qInfo(notifyLog) << "GetServerInformation";
    manager()->GetServerInformation(name, vendor, version, specVersion);
}

NotificationManager *OCEANNotificationDbusAdaptor::manager() const
{
    return qobject_cast<NotificationManager *>(parent());
}

uint OCEANNotificationDbusAdaptor::recordCount() const
{
    return manager()->recordCount();
}

QStringList OCEANNotificationDbusAdaptor::GetAppList()
{
    return manager()->GetAppList();
}

QDBusVariant OCEANNotificationDbusAdaptor::GetAppInfo(const QString &appId, uint configItem)
{
    return QDBusVariant(manager()->GetAppInfo(appId, configItem));
}

void OCEANNotificationDbusAdaptor::SetAppInfo(const QString &appId, uint configItem, const QDBusVariant &value)
{
    return manager()->SetAppInfo(appId, configItem, value.variant());
}

QString OCEANNotificationDbusAdaptor::GetAppSetting(const QString &appName)
{
    Q_UNUSED(appName)
    return {};
}

void OCEANNotificationDbusAdaptor::SetAppSetting(const QString &settings)
{

}

void OCEANNotificationDbusAdaptor::SetSystemInfo(uint configItem, const QDBusVariant &value)
{
    return manager()->SetSystemInfo(configItem, value.variant());
}

QDBusVariant OCEANNotificationDbusAdaptor::GetSystemInfo(uint configItem)
{
    return QDBusVariant(manager()->GetSystemInfo(configItem));
}

} // notification
