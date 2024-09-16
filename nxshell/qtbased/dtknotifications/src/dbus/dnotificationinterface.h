// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QDBusPendingReply>
#include <DDBusInterface>
#include "dnotificationtypes.h"

DNOTIFICATIONS_BEGIN_NAMESPACE

using DTK_CORE_NAMESPACE::DDBusInterface;
class DNotificationInterface : public QObject
{
    Q_OBJECT

public:
    explicit DNotificationInterface(QObject *parent = nullptr);
    virtual ~DNotificationInterface() = default;

public slots:
    QDBusPendingReply<void> clearRecords();
    QDBusPendingReply<void> closeNotification(uint id);
    QDBusPendingReply<QString> allRecords();
    QDBusPendingReply<QDBusVariant> appInfo(const QString &id, uint item);
    QDBusPendingReply<QStringList> appList();
    QDBusPendingReply<QStringList> capbilities();
    QDBusPendingReply<QString> getRecordById(const QString &id);
    QDBusPendingReply<QString> getRecordsFromId(int rowCount, const QString &offsetId);
    QDBusPendingReply<QString, QString, QString, QString> serverInformation();
    QDBusPendingReply<QDBusVariant> systemInformation(uint item);
    QDBusPendingReply<uint> notify(const QString &appName, uint replacesId, const QString &appIcon, const QString &summary, const QString &body, const QStringList &actions, const QVariantMap &hints, int expireTimeout);
    QDBusPendingReply<void> removeRecord(const QString &id);
    QDBusPendingReply<void> setAppInfo(const QString &id, uint item, const QDBusVariant &var);
    QDBusPendingReply<void> setSystemInfo(uint item, const QDBusVariant &var);
    QDBusPendingReply<void> toggle();
    QDBusPendingReply<void> show();
    QDBusPendingReply<void> hide();
    QDBusPendingReply<uint> recordCount();

signals:
    void actionInvoked(uint id, const QString &actionId);
    void notificationClosed(uint id, ClosedReason closedReason);
    void recordAdded(const QString &);
    void appInfoChanged(const QString &id, AppNotificationConfigItem item, QVariant var);
    void systemInfoChanged(DNDModeNotificaitonConfigItem item, QVariant var);
    void appAdded(const QString &id);
    void appRemoved(const QString &id);

private slots:
    void receiveNotificationClosed(uint id, uint closedReason);
    void receiveAppInfoChanged(const QString &id, uint item, QDBusVariant var);
    void receiveSystemInfoChanged(uint item, QDBusVariant var);

private:
    DDBusInterface *m_inter;
};

DNOTIFICATIONS_END_NAMESPACE
