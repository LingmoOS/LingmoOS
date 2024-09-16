// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mocknotifysetting.h"
#include "notification/constants.h"

NotifySettingHelper::NotifySettingHelper()
{
    initAllSettings();
}

void NotifySettingHelper::initAllSettings()
{
    QStringList appList;
    appList << "deepin-editor" << "google-chrome"
            << "deepin-image-viewer" << "dde-calendar"
            << "deepin-diskmanager" << "org.deepin.scanner";

    m_systemSetting["dndmode"] = true;
    m_systemSetting["lockscreen-open-dndmode"] = true;
    m_systemSetting["open-by-time-interval"] = true;
    m_systemSetting["start-time"] = "07:00";
    m_systemSetting["end-time"] = "22:00";
    m_systemSetting["show-icon"] = true;
    m_systemSetting["app-list"] = appList;

    foreach(auto appname, appList) {
        QMap<QString, QVariant> appSetting;
        appSetting["enable-notification"] = DEFAULT_ALLOW_NOTIFY;
        appSetting["show-in-notification-center"] = DEFAULT_ONLY_IN_NOTIFY;
        appSetting["lockscreen-show-notification"] = DEFAULT_LOCK_SHOW_NOTIFY;
        appSetting["enable-preview"] = DEFAULT_SHOW_NOTIFY_PREVIEW;
        appSetting["enable-sound"] = DEFAULT_NOTIFY_SOUND;
        appSetting["app-icon"] = true;
        appSetting["app-name"] = appname;
        appSetting["show-on-top"] = true;
        m_appSetting[appname] = appSetting;
    }
}

void NotifySettingHelper::setAppSetting(const QString &id, const AbstractNotifySetting::AppConfigurationItem &item, const QVariant &var)
{
    QMap<QString, QVariant> &app1Setting = m_appSetting[id];

    switch (item) {
    case  AbstractNotifySetting::APPNAME:
        app1Setting["app-name"] = var;
        break;
    case  AbstractNotifySetting::APPICON:
        app1Setting["app-icon"] = var;
        break;
    case  AbstractNotifySetting::ENABELNOTIFICATION:
        app1Setting["enable-notification"] = var;
        break;
    case  AbstractNotifySetting::ENABELPREVIEW:
        app1Setting["enable-preview"] = var;
        break;
    case  AbstractNotifySetting::ENABELSOUND:
        app1Setting["enable-sound"] = var;
        break;
    case  AbstractNotifySetting::SHOWINNOTIFICATIONCENTER:
        app1Setting["show-in-notification-center"] = var;
        break;
    case  AbstractNotifySetting::LOCKSCREENSHOWNOTIFICATION:
        app1Setting["lockscreen-show-notification"] = var;
        break;
    case  AbstractNotifySetting::SHOWONTOP:
        app1Setting["show-on-top"] = var;
        break;
    default:
        break;
    }
}

QVariant NotifySettingHelper::getAppSetting(const QString &id, const AbstractNotifySetting::AppConfigurationItem &item) const
{
    const auto &app1Setting = m_appSetting[id];

    switch (item) {
    case  AbstractNotifySetting::APPNAME:
        return app1Setting["app-name"];
    case  AbstractNotifySetting::APPICON:
        return app1Setting["app-icon"];
    case  AbstractNotifySetting::ENABELNOTIFICATION:
        return app1Setting["enable-notification"];
    case  AbstractNotifySetting::ENABELPREVIEW:
        return app1Setting["enable-preview"];
    case  AbstractNotifySetting::ENABELSOUND:
        return app1Setting["enable-sound"];
    case  AbstractNotifySetting::SHOWINNOTIFICATIONCENTER:
        return app1Setting["show-in-notification-center"];
    case  AbstractNotifySetting::LOCKSCREENSHOWNOTIFICATION:
        return app1Setting["lockscreen-show-notification"];
    case  AbstractNotifySetting::SHOWONTOP:
        return app1Setting["show-on-top"];
    }

    return QVariant();
}

void NotifySettingHelper::setSystemSetting(const AbstractNotifySetting::SystemConfigurationItem &item, const QVariant &var)
{
    switch (item) {
    case AbstractNotifySetting::DNDMODE:
        m_systemSetting["dndmode"] = var;
        break;
    case AbstractNotifySetting::LOCKSCREENOPENDNDMODE:
        m_systemSetting["lockscreen-open-dndmode"] = var;
        break;
    case AbstractNotifySetting::OPENBYTIMEINTERVAL:
        m_systemSetting["open-by-time-interval"] = var;
        break;
    case AbstractNotifySetting::STARTTIME:
        m_systemSetting["start-time"] = var;
        break;
    case AbstractNotifySetting::ENDTIME:
        m_systemSetting["end-time"] = var;
        break;
    case AbstractNotifySetting::SHOWICON:
        m_systemSetting["show-icon"] = var;
        break;
    }
}

QVariant NotifySettingHelper::getSystemSetting(const AbstractNotifySetting::SystemConfigurationItem &item) const
{
    switch (item) {
    case AbstractNotifySetting::DNDMODE:
        return m_systemSetting["dndmode"];
    case AbstractNotifySetting::LOCKSCREENOPENDNDMODE:
        return m_systemSetting["lockscreen-open-dndmode"];
    case AbstractNotifySetting::OPENBYTIMEINTERVAL:
        return m_systemSetting["open-by-time-interval"];
    case AbstractNotifySetting::STARTTIME:
        return m_systemSetting["start-time"];
    case AbstractNotifySetting::ENDTIME:
        return m_systemSetting["end-time"];
    case AbstractNotifySetting::SHOWICON:
        return m_systemSetting["show-icon"];
    }

    return QVariant();
}

QStringList NotifySettingHelper::getAppLists()
{
    return m_systemSetting["app-list"].toStringList();
}

void NotifySettingHelper::appAdded(const LauncherItemInfo &info)
{
    QStringList appList = getAppLists();
    if (!appList.contains(info.id)) {
        appList.append(info.id);
        m_systemSetting["app-list"] = appList;
    }

    QMap<QString, QVariant> app1Setting;

    app1Setting["app-name"] = info.name;
    app1Setting["app-icon"] = info.icon;
    app1Setting["enable-notification"] = DEFAULT_ALLOW_NOTIFY;
    app1Setting["enable-preview"] = DEFAULT_SHOW_NOTIFY_PREVIEW;
    app1Setting["enable-sound"] = DEFAULT_NOTIFY_SOUND;
    app1Setting["show-in-notification-center"] = DEFAULT_ONLY_IN_NOTIFY;
    app1Setting["lockscreen-show-notification"] = DEFAULT_LOCK_SHOW_NOTIFY;

    m_appSetting[info.id] = app1Setting;
}

void NotifySettingHelper::appRemoved(const QString &id)
{
    m_appSetting.remove(id);
    m_appSetting["app-list"].remove(id);
}

void NotifySettingHelper::setAppSetting_v1(QString settings)
{
    Q_UNUSED(settings)
}

QString NotifySettingHelper::getAppSettings_v1(const QString &id)
{
    Q_UNUSED(id)
    return QString();
}

void NotifySettingHelper::setSystemSetting_v1(QString settings)
{
    Q_UNUSED(settings)
}

QString NotifySettingHelper::getSystemSetings_v1()
{
    return QString();
}

void NotifySettingHelper::setAllSetting_v1(QString settings)
{
    Q_UNUSED(settings)
}

QString NotifySettingHelper::getAllSetings_v1()
{
    return QString();
}
