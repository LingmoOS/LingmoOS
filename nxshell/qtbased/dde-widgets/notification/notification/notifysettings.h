// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NOTIFYSETTINGS_H
#define NOTIFYSETTINGS_H

#include "launcher_interface.h"
#include "types/launcheriteminfo.h"
#include "types/launcheriteminfolist.h"

#include <QObject>

class QGSettings;
class QTimer;

using LauncherInter = org::deepin::dde::daemon::Launcher1;

class AbstractNotifySetting : public QObject
{
    Q_OBJECT
public:
    typedef enum {
        APPNAME,
        APPICON,
        ENABELNOTIFICATION,
        ENABELPREVIEW,
        ENABELSOUND,
        SHOWINNOTIFICATIONCENTER,
        LOCKSCREENSHOWNOTIFICATION,
        SHOWNOTIFICATIONTOP
    } AppConfigurationItem;

    typedef enum {
        DNDMODE,
        LOCKSCREENOPENDNDMODE,
        OPENBYTIMEINTERVAL,
        STARTTIME,
        ENDTIME,
        SHOWICON
    } SystemConfigurationItem;

    explicit AbstractNotifySetting(QObject *parent = nullptr)
        : QObject(parent)
    {}
    virtual void initAllSettings() = 0;
    virtual void setAppSetting(const QString &id, const AppConfigurationItem &item, const QVariant &var) = 0;
    virtual QVariant getAppSetting(const QString &id, const AppConfigurationItem &item) = 0;
    virtual void setSystemSetting(const SystemConfigurationItem &item, const QVariant &var) = 0;
    virtual QVariant getSystemSetting(const SystemConfigurationItem &item) = 0;
    virtual QStringList getAppLists() = 0;
    virtual void appAdded(const LauncherItemInfo &info) = 0;
    virtual void appRemoved(const QString &id) = 0;

signals:
    void appAddedSignal(const QString &id);
    void appRemovedSignal(const QString &id);
    void appSettingChanged(const QString &id, const uint &item, QVariant var);
    void systemSettingChanged(const uint &item, QVariant var);
};

class NotifySettingsObserver : public AbstractNotifySetting
{
    Q_OBJECT
public:
    explicit NotifySettingsObserver(QObject *parent = nullptr);
    void initAllSettings() override { Q_UNREACHABLE(); }
    void setAppSetting(const QString &id, const AppConfigurationItem &item, const QVariant &var) override;
    QVariant getAppSetting(const QString &id, const AppConfigurationItem &item) override;
    void setSystemSetting(const SystemConfigurationItem &, const QVariant &) override { Q_UNREACHABLE(); }
    QVariant getSystemSetting(const SystemConfigurationItem &item) override { Q_UNUSED(item); Q_UNREACHABLE(); return QVariant();}
    QStringList getAppLists() override { Q_UNREACHABLE(); return QStringList();}
    void appAdded(const LauncherItemInfo &info) override { Q_UNUSED(info); Q_UNREACHABLE(); }
    void appRemoved(const QString &id) override { Q_UNUSED(id); Q_UNREACHABLE(); }

private Q_SLOTS:
    void onReceivedAppInfoChanged(const QString &id, uint item, QDBusVariant var);

private:
    QDBusInterface *notifyObserver();

    QScopedPointer<QDBusInterface> m_notifyObserver;
};

#endif // NOTIFYSETTINGS_H
