// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NOTIFYSETTINGS_H
#define NOTIFYSETTINGS_H

#include "types/launcheriteminfo.h"
#include "objectmanager_interface.h"

#include <QObject>

class QGSettings;
class QTimer;

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
        SHOWONTOP
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

    // 兼容之前版本的接口，后期移除
    virtual void setAppSetting_v1(QString settings) = 0;
    virtual QString getAppSettings_v1(const QString &id) = 0;
    virtual void setSystemSetting_v1(QString settings) = 0;
    virtual QString getSystemSetings_v1() = 0;
    virtual void setAllSetting_v1(QString settings) = 0;
    virtual QString getAllSetings_v1() = 0;

signals:
    void appAddedSignal(const QString &id);
    void appRemovedSignal(const QString &id);
    void appSettingChanged(const QString &id, const uint &item, QVariant var);
    void systemSettingChanged(const uint &item, QVariant var);
};

class NotifySettings : public AbstractNotifySetting
{
    Q_OBJECT
public:
    explicit NotifySettings(QObject *parent = nullptr);
    void initAllSettings() override;
    void setAppSetting(const QString &id, const AppConfigurationItem &item, const QVariant &var) override;
    QVariant getAppSetting(const QString &id, const AppConfigurationItem &item) override;
    void setSystemSetting(const SystemConfigurationItem &item, const QVariant &var) override;
    QVariant getSystemSetting(const SystemConfigurationItem &item) override;
    QStringList getAppLists() override;
    void appAdded(const LauncherItemInfo &info) override;
    void appRemoved(const QString &id) override;

    // 兼容之前版本的接口，后期移除
    void setAppSetting_v1(QString settings) override;
    QString getAppSettings_v1(const QString &id) override;
    void setSystemSetting_v1(QString settings) override;
    QString getSystemSetings_v1() override;
    void setAllSetting_v1(QString settings) override;
    QString getAllSetings_v1() override;

private:
    bool containsAppSettings(const QGSettings &settings, const QString &id);

    QTimer *m_initTimer;
    QGSettings *m_systemSetting;
    ApplicationObjectManager1 *m_applicationObjectInter;
    LauncherItemInfoList m_launcherItemInfoList;
};

#endif // NOTIFYSETTINGS_H
