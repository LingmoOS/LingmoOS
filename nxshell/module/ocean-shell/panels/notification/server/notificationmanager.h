// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QDBusContext>
#include <QDBusVariant>

#include "sessionmanager1interface.h"

using UserSessionManager = org::lingmo::ocean::SessionManager1;

namespace notification {

class NotifyEntity;
class DataAccessor;
class NotificationSetting;

class NotificationManager : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_PROPERTY(uint recordCount READ recordCount NOTIFY RecordCountChanged)
public:
    explicit NotificationManager(QObject *parent = nullptr);
    ~NotificationManager() override;

public:
    bool registerDbusService();

    uint recordCount() const;
    Q_INVOKABLE void actionInvoked(qint64 id, uint bubbleId, const QString &actionKey);
    Q_INVOKABLE void notificationClosed(qint64 id, uint bubbleId, uint reason);
    Q_INVOKABLE void notificationReplaced(qint64 id);

    void removeNotification(qint64 id);
    void removeNotifications(const QString &appName);
    void removeNotifications();

Q_SIGNALS:
    // Standard Notifications dbus implementation
    void ActionInvoked(uint id, const QString &actionKey);
    void NotificationClosed(uint id, uint reason);

    // Extra DBus APIs
    void RecordCountChanged(uint count);
    void AppInfoChanged(const QString &id, uint item, const QDBusVariant &value);
    void SystemInfoChanged(uint item, const QDBusVariant &value);
    void AppAoceand(const QString &id);
    void AppRemoved(const QString &id);

    void NotificationStateChanged(qint64 id, int processedType);

public Q_SLOTS:
    // Standard Notifications dbus implementation
    QStringList GetCapabilities();
    uint Notify(const QString &appName, uint replacesId, const QString &appIcon, const QString &summary, const QString &body, const QStringList &actions, const QVariantMap &hints, int expireTimeout);
    void CloseNotification(uint id);
    void GetServerInformation(QString &name, QString &vendor, QString &version, QString &specVersion);

    // Extra DBus APIS
    QStringList GetAppList();
    void SetAppInfo(const QString &appId, uint configItem, const QVariant &value);
    QVariant GetAppInfo(const QString &appId, uint configItem);

    void SetSystemInfo(uint configItem, const QVariant &value);
    QVariant GetSystemInfo(uint configItem);

private:
    bool isDoNotDisturb() const;
    void tryPlayNotificationSound(const NotifyEntity &entity, const QString &appId, bool dndMode) const;
    void emitRecordCountChanged();

    void pushPendingEntity(const NotifyEntity &entity, int expireTimeout);
    void updateEntityProcessed(qint64 id, uint reason);
    void updateEntityProcessed(const NotifyEntity &entity);

    QString appIdByAppName(const QString &appName) const;
    void doActionInvoked(const NotifyEntity &entity, const QString &actionId);

private slots:
    void onHandingPendingEntities();
    void removePendingEntity(const NotifyEntity &entity);

private:
    uint m_replacesCount = 0;

    DataAccessor *m_persistence = nullptr;
    NotificationSetting *m_setting = nullptr;
    UserSessionManager *m_userSessionManager = nullptr;
    QTimer *m_pendingTimeout = nullptr;
    qint64 m_lastTimeoutPoint = std::numeric_limits<qint64>::max();
    QMultiHash<qint64, NotifyEntity> m_pendingTimeoutEntities;
    QStringList m_systemApps;
    QMap<QString, QVariant> m_appNamesMap;
};

} // notification
