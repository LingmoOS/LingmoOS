// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "dnotificationtypes.h"
#include "dnotificationappconfig.h"
#include "dnotificationdndmodeconfig.h"
#include <dexpected.h>

DNOTIFICATIONS_BEGIN_NAMESPACE
DCORE_USE_NAMESPACE

class DNotificationManagerPrivate;
class DNotificationManager : public QObject
{
    Q_OBJECT
public:
    explicit DNotificationManager(QObject *parent = nullptr);
    virtual ~DNotificationManager();
    QList<QString> recordIds() const;

public Q_SLOTS:
    DExpected<QList<DNotificationData>> allRecords() const;
    DExpected<QList<DNotificationData>> getRecordsFromId(int rowCount, const QString &offsetId);
    DExpected<DNotificationData> getRecordById(const QString &id);
    DExpected<int> recordCount();
    DExpected<void> clearRecords();
    DExpected<void> removeRecord(const QString &id);

    DExpected<ServerInformation> serverInformation();
    DExpected<QStringList> capbilities();

    DExpected<uint> notify(const QString &appName, uint replacesId,
                const QString &appIcon, const QString &summary,
                const QString &body = QString(), const QStringList &actions = QStringList(),
                const QVariantMap hints = QVariantMap(), int expireTimeout = 5000);
    DExpected<void> closeNotification(uint id);

    DExpected<QStringList> appList();

    DNotificationAppConfigPtr notificationAppConfig(const QString &id) const;
    DNotificationDNDModeConfigPtr notificationDNDModeConfig() const;
    QList<DAbstractNotificationModeConfigPtr> notificationModeConfigs() const;

    DExpected<void> toggleNotificationCenter();
    DExpected<void> showNotificationCenter();
    DExpected<void> hideNotificationCenter();

Q_SIGNALS:
    void actionInvoked(uint id, const QString &actionId);
    void notificationClosed(uint id, ClosedReason closedReason);
    void recordAdded(const QString &);
    void appNotificationConfigChanged(const QString &id, AppNotificationConfigItem item, QVariant var);
    void dndModeNotificationConfigChanged(DNDModeNotificaitonConfigItem item, QVariant var);
    void appAdded(const QString &id);
    void appRemoved(const QString &id);

private:
    QScopedPointer<DNotificationManagerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DNotificationManager)
};

DNOTIFICATIONS_END_NAMESPACE
