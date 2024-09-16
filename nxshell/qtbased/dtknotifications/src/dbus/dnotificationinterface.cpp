// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include <QDBusConnection>

#include "dnotificationinterface.h"

DNOTIFICATIONS_BEGIN_NAMESPACE

using DTK_CORE_NAMESPACE::DDBusInterface;
DNotificationInterface::DNotificationInterface(QObject *parent)
    : QObject(parent)
{
#if defined(USE_FAKE_INTERFACE)
    const QString &Service = QStringLiteral("com.deepin.dde.FakeNotification");
    const QString &Path = QStringLiteral("/com/deepin/dde/FakeNotification");
    const QString &Interface = QStringLiteral("com.deepin.dde.FakeNotification");
    QDBusConnection Connection = QDBusConnection::sessionBus();
#else
    const QString &Service = QStringLiteral("com.deepin.dde.Notification");
    const QString &Path = QStringLiteral("/com/deepin/dde/Notification");
    const QString &Interface = QStringLiteral("com.deepin.dde.Notification");
    QDBusConnection Connection = QDBusConnection::sessionBus();
#endif
    Connection.connect(Service, Path, Interface, "ActionInvoked", this, SIGNAL(actionInvoked(uint, const QString &)));
    Connection.connect(Service, Path, Interface, "NotificationClosed", this, SLOT(receiveNotificationClosed(uint, uint)));
    Connection.connect(Service, Path, Interface, "RecordAdded", this, SIGNAL(recordAdded(const QString &)));
    Connection.connect(Service, Path, Interface, "AppInfoChanged", this, SLOT(receiveAppInfoChanged(const QString&, uint, QDBusVariant)));
    Connection.connect(Service, Path, Interface, "SystemInfoChanged", this, SLOT(receiveSystemInfoChanged(uint, QDBusVariant)));
    Connection.connect(Service, Path, Interface, "AppAddedSignal", this, SIGNAL(appAdded(const QString &)));
    Connection.connect(Service, Path, Interface, "AppRemovedSignal", this, SIGNAL(appRemoved(const QString &)));
    m_inter = new DDBusInterface(Service, Path, Interface, Connection, this);
}

QDBusPendingReply<void> DNotificationInterface::clearRecords()
{
    return m_inter->asyncCall("ClearRecords");
}

QDBusPendingReply<void> DNotificationInterface::closeNotification(uint id)
{
    QVariantList args{QVariant::fromValue(id)};
    return m_inter->asyncCallWithArgumentList("CloseNotification", args);
}

QDBusPendingReply<QString> DNotificationInterface::allRecords()
{
    return m_inter->asyncCall("GetAllRecords");
}

QDBusPendingReply<QDBusVariant> DNotificationInterface::appInfo(const QString &id, uint item)
{
    QVariantList args{QVariant::fromValue(id), QVariant::fromValue(item)};
    return m_inter->asyncCallWithArgumentList("GetAppInfo", args);
}

QDBusPendingReply<QStringList> DNotificationInterface::appList()
{
    return m_inter->asyncCall("GetAppList");
}

QDBusPendingReply<QStringList> DNotificationInterface::capbilities()
{
    return m_inter->asyncCall("GetCapbilities");
}

QDBusPendingReply<QString> DNotificationInterface::getRecordById(const QString &id)
{
    QVariantList args{QVariant::fromValue(id)};
    return m_inter->asyncCallWithArgumentList("GetRecordById", args);
}

QDBusPendingReply<QString> DNotificationInterface::getRecordsFromId(int rowCount, const QString &offsetId)
{
    QVariantList args{QVariant::fromValue(rowCount), QVariant::fromValue(offsetId)};
    return m_inter->asyncCallWithArgumentList("GetRecordsFromId", args);
}

QDBusPendingReply<QString, QString, QString, QString> DNotificationInterface::serverInformation()
{
    return m_inter->asyncCall("GetServerInformation");
}

QDBusPendingReply<QDBusVariant> DNotificationInterface::systemInformation(uint item)
{
    QVariantList args{QVariant::fromValue(item)};
    return m_inter->asyncCallWithArgumentList("GetSystemInfo", args);
}

QDBusPendingReply<uint> DNotificationInterface::notify(const QString &appName, uint replacesId, const QString &appIcon, const QString &summary, const QString &body, const QStringList &actions, const QVariantMap &hints, int expireTimeout)
{
    QVariantList args{QVariant::fromValue(appName), QVariant::fromValue(replacesId), QVariant::fromValue(appIcon), QVariant::fromValue(summary), QVariant::fromValue(body), QVariant::fromValue(actions), QVariant::fromValue(hints), QVariant::fromValue(expireTimeout)};
    return m_inter->asyncCallWithArgumentList("Notify", args);
}

QDBusPendingReply<void> DNotificationInterface::removeRecord(const QString &id)
{
    QVariantList args{QVariant::fromValue(id)};
    return m_inter->asyncCallWithArgumentList("RemoveRecord", args);
}

QDBusPendingReply<void> DNotificationInterface::setAppInfo(const QString &id, uint item, const QDBusVariant &var)
{
    QVariantList args{QVariant::fromValue(id), QVariant::fromValue(item), QVariant::fromValue(var)};
    return m_inter->asyncCallWithArgumentList("SetAppInfo", args);
}

QDBusPendingReply<void> DNotificationInterface::setSystemInfo(uint item, const QDBusVariant &var)
{
    QVariantList args{QVariant::fromValue(item), QVariant::fromValue(var)};
    return m_inter->asyncCallWithArgumentList("SetSystemInfo", args);
}

QDBusPendingReply<void> DNotificationInterface::toggle()
{
    return m_inter->asyncCall("Toggle");
}

QDBusPendingReply<void> DNotificationInterface::show()
{
    return m_inter->asyncCall("Show");
}

QDBusPendingReply<void> DNotificationInterface::hide()
{
    return m_inter->asyncCall("Hide");
}

QDBusPendingReply<uint> DNotificationInterface::recordCount()
{
    return m_inter->asyncCall("recordCount");
}

void DNotificationInterface::receiveNotificationClosed(uint id, uint reason)
{
    ClosedReason closedReason = static_cast<ClosedReason>(reason);
    Q_EMIT notificationClosed(id, closedReason);
}

void DNotificationInterface::receiveAppInfoChanged(const QString &id, uint item, QDBusVariant var)
{
    AppNotificationConfigItem appConfigItem = static_cast<AppNotificationConfigItem>(item);

    Q_EMIT appInfoChanged(id, appConfigItem , var.variant());
}

void DNotificationInterface::receiveSystemInfoChanged(uint item, QDBusVariant var)
{
    DNDModeNotificaitonConfigItem sysConfigItem = static_cast<DNDModeNotificaitonConfigItem>(item);
    Q_EMIT systemInfoChanged(sysConfigItem, var.variant());
}
DNOTIFICATIONS_END_NAMESPACE
