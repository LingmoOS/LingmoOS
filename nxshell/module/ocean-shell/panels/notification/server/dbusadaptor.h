// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDBusVariant>
#include <QDBusAbstractAdaptor>

namespace notification {

class NotificationManager;
class DbusAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.Notifications")

public:
    explicit DbusAdaptor(QObject *parent = nullptr);

public Q_SLOTS: // methods
    QStringList GetCapabilities();
    uint Notify(const QString &appName, uint replacesId, const QString &appIcon, const QString &summary,
                const QString &body, const QStringList &actions, const QVariantMap &hints, int expireTimeout);
    void CloseNotification(uint id);
    void GetServerInformation(QString &name, QString &vendor, QString &version, QString &specVersion);


private:
    NotificationManager *manager() const;

Q_SIGNALS:
    void ActionInvoked(uint id, const QString &actionKey);
    void NotificationClosed(uint id, uint reason);
    // todo void ActivationToken(uint id, const QString &activationToken)
};

class OCEANNotificationDbusAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_PROPERTY(uint recordCount READ recordCount NOTIFY RecordCountChanged)
    Q_CLASSINFO("D-Bus Interface", "org.lingmo.ocean.Notification1")

public:
    explicit OCEANNotificationDbusAdaptor(QObject *parent = nullptr);

public Q_SLOTS: // methods
    QStringList GetCapabilities();
    uint Notify(const QString &appName, uint replacesId, const QString &appIcon, const QString &summary,
                const QString &body, const QStringList &actions, const QVariantMap &hints, int expireTimeout);
    void CloseNotification(uint id);
    void GetServerInformation(QString &name, QString &vendor, QString &version, QString &specVersion);

Q_SIGNALS:
    void ActionInvoked(uint id, const QString &actionKey);
    void NotificationClosed(uint id, uint reason);
    // todo void ActivationToken(uint id, const QString &activationToken)

public Q_SLOTS: // methods
    uint recordCount() const;

    QStringList GetAppList();
    QDBusVariant GetAppInfo(const QString &appId, uint configItem);
    void SetAppInfo(const QString &appId, uint configItem, const QDBusVariant &value);

    QString GetAppSetting(const QString &appName);
    void SetAppSetting(const QString &settings);

    void SetSystemInfo(uint configItem, const QDBusVariant &value);
    QDBusVariant GetSystemInfo(uint configItem);

private:
    NotificationManager *manager() const;

Q_SIGNALS:
    void AppAoceand(const QString &appId);
    void AppRemoved(const QString &appId);
    void AppInfoChanged(const QString &appId, uint configItem, const QDBusVariant &value);
    void AppSettingChanged(const QString &settings);

    void SystemSettingChanged(const QString &settings);
    void SystemInfoChanged(uint configItem, const QDBusVariant &value);

    void NotificationStateChanged(qint64 id, int processedType);
    void RecordCountChanged(uint count);
};

} // notification
