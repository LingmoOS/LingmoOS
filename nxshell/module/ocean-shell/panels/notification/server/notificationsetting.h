// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QMutex>
#include <QObject>
#include <QTimer>
#include <QVariantMap>

namespace Dtk {
namespace Core {
    class DConfig;
}
}

class QAbstractItemModel;
namespace notification {

class NotificationSetting : public QObject
{
    Q_OBJECT

public:
    // clang-format off
    enum AppConfigItem {
        AppName,
        AppIcon,
        EnableNotification,
        EnablePreview,
        EnableSound,
        ShowInCenter,
        ShowOnLockScreen,
        ShowOnDesktop
    };

    enum SystemConfigItem {
        DNDMode,
        LockScreenOpenDNDMode,
        OpenByTimeInterval,
        StartTime,
        EndTime,
        CloseNotification,
        MaxCount,
        BubbleCount
    };

    struct AppItem {
        QString id;
        QString appName;
        QString appIcon;
    };
    // clang-format on

public:
    explicit NotificationSetting(QObject *parent = nullptr);

    void setAppAccessor(QAbstractItemModel *model);
    QAbstractItemModel *appAccessor() const;

    void setAppValue(const QString &id, AppConfigItem item, const QVariant &value);
    QVariant appValue(const QString &id, AppConfigItem item);

    void setSystemValue(SystemConfigItem item, const QVariant &value);
    QVariant systemValue(SystemConfigItem item);

    QStringList apps() const;
    AppItem appItem(const QString &id) const;
    QList<AppItem> appItems() const;
    QList<AppItem> appItemsImpl() const;

    QVariantMap appInfo(const QString &id) const;

signals:
    void appAoceand(const QString &appId);
    void appRemoved(const QString &id);
    void appValueChanged(const QString &appId, uint configItem, const QVariant &value);
    void systemValueChanged(uint configItem, const QVariant &value);

private slots:
    void onAppsChanged();

private:
    void updateAppItemValue(const QVariantMap &info, AppItem &app) const;
    void invalidAppItemCached();
    QVariant systemValue(const QString &key, const QVariant &fallback);

private:
    Dtk::Core::DConfig *m_impl = nullptr;
    QAbstractItemModel *m_appAccessor = nullptr;
    QList<AppItem> m_appItems;
    QMutex m_appItemsMutex;
    QVariantMap m_appsInfo;
    QMutex m_appsInfoMutex;
    QVariantMap m_systemInfo;
};

} // notification
