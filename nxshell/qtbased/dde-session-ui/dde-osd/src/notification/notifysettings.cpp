// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "notifysettings.h"
#include "constants.h"
#include "types/objectmanager_type.h"
#include <DUtil>

#include <QGSettings>
#include <QTimer>
#include <QVariant>
#include <QJsonObject>
#include <QJsonDocument>
#include <QByteArray>
#include <DDesktopEntry>
#include <QtConcurrent>
#include <QStringList>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(SessionNotifySettings, "session-ui-notifysettings");

DCORE_USE_NAMESPACE

const QString schemaKey = "com.deepin.dde.notifications";
const QString schemaPath = "/com/deepin/dde/notifications/";
const QString appSchemaKey = "com.deepin.dde.notifications.applications";
const QString appSchemaPath = "/com/deepin/dde/notifications/applications/%1/";

static QStringList getUILanguages()
{
    QLocale syslocal = QLocale::system();

    QStringList uiLanguages = syslocal.uiLanguages();
    // the nameMap uses underscore instead of minus sign
    for (auto &lang : uiLanguages) {
        lang.replace('-', '_');
    }
    uiLanguages << "default";

    return uiLanguages;
}

static LauncherItemInfo fromObjectInterfaceMapToItemInfo(const QDBusObjectPath &path, const ObjectInterfaceMap &valueMap)
{
    auto uiLanguages = getUILanguages();
    LauncherItemInfo info;
    info.path = path.path();
    info.noDisplay = false;
    for (const QVariantMap &mapInter : valueMap) {
        if (mapInter.count() == 0) {
            continue;
        }

        QString icon;
        if (auto mapicons = mapInter.value("Icons"); !mapicons.isNull()) {
            auto icons = qdbus_cast<QMap<QString, QString>>(mapicons);
            if (!icons.value("Desktop Entry").isNull()) {
                icon = icons["Desktop Entry"];
            }
        }

        if (auto noDisplay = mapInter.value("NoDisplay"); !noDisplay.isNull()) {
            info.noDisplay = noDisplay.toBool();
        }

        info.icon = icon;
        bool isDeepinVendor = qdbus_cast<QString>(mapInter["X_Deepin_Vendor"]) == QStringLiteral("deepin");
        auto genericNameMap = qdbus_cast<QMap<QString, QString>>(mapInter["GenericName"]);
        auto nameMap = qdbus_cast<QMap<QString, QString>>(mapInter["Name"]);
        QString id = qdbus_cast<QString>(mapInter["ID"]);
        info.id = id;

        auto findName = [uiLanguages](QMap<QString, QString> &nmap){
            QString name;
            for (auto &lang : uiLanguages) {
                auto iter = nmap.find(lang);
                if (iter != nmap.end()) {
                    name = iter.value();
                    break;
                }
            }
            return name;
        };

        QString showName;
        if (isDeepinVendor)
            showName = findName(genericNameMap);

        if (showName.isEmpty())
            showName = findName(nameMap);

        if (showName.isEmpty())
            showName = id;

        info.name = showName;
        info.names = nameMap.values() + genericNameMap.values();
    }
    return info;

}

static QList<LauncherItemInfo> fromObjectMaptoLauncherItemInfoList(const ObjectMap &map)
{
    QList<LauncherItemInfo> infos;
    for (auto [dbusPath, valueMap] : map.toStdMap()) {
        infos << fromObjectInterfaceMapToItemInfo(dbusPath, valueMap);
    }
    return infos;
}

NotifySettings::NotifySettings(QObject *parent)
    : AbstractNotifySetting(parent)
    , m_initTimer(new QTimer(this))
    , m_applicationObjectInter(
                new ApplicationObjectManager1("org.desktopspec.ApplicationManager1",
                                              "/org/desktopspec/ApplicationManager1",
                                              QDBusConnection::sessionBus(),
                                              this))
{
    registerAmMetaType();

    if (!QGSettings::isSchemaInstalled("com.deepin.dde.notification")) {
        qCWarning(SessionNotifySettings)<<"System configuration fetch failed!";
    }
    m_initTimer->start(1000);
    m_initTimer->setSingleShot(true);
    m_systemSetting = new QGSettings(schemaKey.toLocal8Bit(), schemaPath.toLocal8Bit(), this);

    connect(m_initTimer, &QTimer::timeout, this, &NotifySettings::initAllSettings);
    connect(m_applicationObjectInter, &ApplicationObjectManager1::InterfacesAdded, this, [this](const QDBusObjectPath &object_path, ObjectInterfaceMap interfaces) {
        LauncherItemInfo info = fromObjectInterfaceMapToItemInfo(object_path, interfaces);
        qCInfo(SessionNotifySettings) << "Iterface added, added id=" << info.id;
        if (info.noDisplay) {
            return;
        }
        appAdded(info);
    });
    connect(m_applicationObjectInter, &ApplicationObjectManager1::InterfacesRemoved, this, [this](const QDBusObjectPath &object_path, const QStringList) {
        QString path = object_path.path();
        QString id_origin = path.split("/").last();
        QString id = DUtil::unescapeFromObjectPath(id_origin);
        qCInfo(SessionNotifySettings) << "Interface removed, removed id=" << id;
        appRemoved(id);

        LauncherItemInfo info;
        info.path = path;
        if (m_launcherItemInfoList.contains(info))
            m_launcherItemInfoList.removeOne(info);
    });
}

void NotifySettings::initAllSettings()
{
    ObjectMap app_map =  m_applicationObjectInter->GetManagedObjects();
    if (app_map.isEmpty()) {
        m_initTimer->stop();
    }
    m_launcherItemInfoList = fromObjectMaptoLauncherItemInfoList(app_map);

    QStringList appList = m_systemSetting->get("app-list").toStringList();
    QStringList launcherList;

    for(const LauncherItemInfo &item : m_launcherItemInfoList) {
        if (IgnoreList.contains(item.id)) {
            continue;
        }

        if (item.noDisplay) {
            continue;
        }

        launcherList << item.id;

        if (appList.contains(item.id)) {
            // 修改系统语言后需要更新翻译
            QGSettings itemSetting(appSchemaKey.toLocal8Bit(), appSchemaPath.arg(item.id).toLocal8Bit(), this);
            itemSetting.set("app-name", item.name);
            // icon 也可能会更新
            itemSetting.set("app-icon", item.icon);
            continue;
        }
        appList.append(item.id);
        m_systemSetting->set("app-list", appList);
        appAdded(item);
    }

    for (const QString &app : appList) {
        if (!launcherList.contains(app)) {
            appRemoved(app);
        }
    }
}

void NotifySettings::setAppSetting(const QString &id, const NotifySettings::AppConfigurationItem &item, const QVariant &var)
{
    if (id.isEmpty()) {
        return;
    }
    QGSettings itemSetting(appSchemaKey.toLocal8Bit(), appSchemaPath.arg(id).toLocal8Bit(), this);
    switch (item) {
    case APPNAME:
        itemSetting.set("app-name", var);
        break;
    case APPICON:
        itemSetting.set("app-icon", var);
        break;
    case ENABELNOTIFICATION:
        itemSetting.set("enable-notification", var);
        break;
    case ENABELPREVIEW:
        itemSetting.set("enable-preview", var);
        break;
    case ENABELSOUND:
        itemSetting.set("enable-sound", var);
        break;
    case SHOWINNOTIFICATIONCENTER:
        itemSetting.set("show-in-notification-center", var);
        break;
    case LOCKSCREENSHOWNOTIFICATION:
        itemSetting.set("lockscreen-show-notification", var);
        break;
    case SHOWONTOP:
        if (containsAppSettings(itemSetting, "show-on-top")) {
            itemSetting.set("show-on-top", var);
            break;
        }
    default:
        return;
    }

    Q_EMIT appSettingChanged(id, item, var);
}

QVariant NotifySettings::getAppSetting(const QString &id, const NotifySettings::AppConfigurationItem &item)
{
    QString newid = id.isEmpty() ? "empty-app" : id;

    auto it = std::find_if(m_launcherItemInfoList.begin(),
                           m_launcherItemInfoList.end(),
                           [newid](const LauncherItemInfo &info) {
                               return newid == info.id;
                           });
    if (it == m_launcherItemInfoList.end()) {
        for (auto info : m_launcherItemInfoList) {
            if (info.names.contains(newid) && info.id != newid) {
                qDebug() << newid << "not found,fallback to" << info.id;
                newid = info.id;
            }
        }
    }

    QGSettings itemSetting(appSchemaKey.toLocal8Bit(), appSchemaPath.arg(newid).toLocal8Bit(), this);

    QVariant results;
    switch (item) {
    case APPNAME:
        results = itemSetting.get("app-name");
        break;
    case APPICON:
        results = itemSetting.get("app-icon");
        break;
    case ENABELNOTIFICATION:
        results = itemSetting.get("enable-notification");
        break;
    case ENABELPREVIEW:
        results = itemSetting.get("enable-preview");
        break;
    case ENABELSOUND:
        results = itemSetting.get("enable-sound");
        break;
    case SHOWINNOTIFICATIONCENTER:
        results = itemSetting.get("show-in-notification-center");
        break;
    case LOCKSCREENSHOWNOTIFICATION:
        results = itemSetting.get("lockscreen-show-notification");
        break;
    case SHOWONTOP:
        if (containsAppSettings(itemSetting, "show-on-top")) {
            results = itemSetting.get("show-on-top");
            break;
        }
    }

    return results;
}

void NotifySettings::setSystemSetting(const NotifySettings::SystemConfigurationItem &item, const QVariant &var)
{
    switch (item) {
    case DNDMODE:
        m_systemSetting->set("dndmode", var);
        break;
    case LOCKSCREENOPENDNDMODE:
        m_systemSetting->set("lockscreen-open-dndmode", var);
        break;
    case OPENBYTIMEINTERVAL:
        m_systemSetting->set("open-by-time-interval", var);
        break;
    case STARTTIME:
        m_systemSetting->set("start-time", var);
        break;
    case ENDTIME:
        m_systemSetting->set("end-time", var);
        break;
    case SHOWICON:
        m_systemSetting->set("show-icon", var);
        break;
    default:
        return;
    }

    Q_EMIT systemSettingChanged(item, var);
}

QVariant NotifySettings::getSystemSetting(const NotifySettings::SystemConfigurationItem &item)
{
    QVariant results;
    switch (item) {
    case DNDMODE:
        results = m_systemSetting->get("dndmode");
        break;
    case LOCKSCREENOPENDNDMODE:
        results = m_systemSetting->get("lockscreen-open-dndmode");
        break;
    case OPENBYTIMEINTERVAL:
        results = m_systemSetting->get("open-by-time-interval");
        break;
    case STARTTIME:
        results = m_systemSetting->get("start-time");
        break;
    case ENDTIME:
        results = m_systemSetting->get("end-time");
        break;
    case SHOWICON:
        results = m_systemSetting->get("show-icon");
        break;
    }
    return results;
}

QStringList NotifySettings::getAppLists()
{
    return m_systemSetting->get("app-list").toStringList();
}

void NotifySettings::appAdded(const LauncherItemInfo &info)
{
    QStringList appList = m_systemSetting->get("app-list").toStringList();
    if (!appList.contains(info.id)) {
        appList.append(info.id);
        m_systemSetting->set("app-list", appList);
    }

    QGSettings itemSetting(appSchemaKey.toLocal8Bit(), appSchemaPath.arg(info.id).toLocal8Bit(), this);
    if (itemSetting.get("app-name").toString().isEmpty()) { // 用app-name为空来判断是否已经有配置
        itemSetting.set("app-name", info.name);
        itemSetting.set("app-icon", info.icon);
        itemSetting.set("enable-notification", DEFAULT_ALLOW_NOTIFY);
        itemSetting.set("enable-preview", DEFAULT_SHOW_NOTIFY_PREVIEW);
        itemSetting.set("enable-sound", DEFAULT_NOTIFY_SOUND);
        itemSetting.set("show-in-notification-center", DEFAULT_ONLY_IN_NOTIFY);
        itemSetting.set("lockscreen-show-notification", DEFAULT_LOCK_SHOW_NOTIFY);
    }
    if (!m_launcherItemInfoList.contains(info)) {
        m_launcherItemInfoList << info;
    }

    Q_EMIT appAddedSignal(info.id);
}

void NotifySettings::appRemoved(const QString &id)
{
    QStringList appList = m_systemSetting->get("app-list").toStringList();
    if (appList.contains(id)) {
        appList.removeOne(id);
        m_systemSetting->set("app-list", appList);
    }

    QGSettings itemSetting(appSchemaKey.toLocal8Bit(), appSchemaPath.arg(id).toLocal8Bit(), this);
    itemSetting.reset("app-name");
    itemSetting.reset("app-icon");
    itemSetting.reset("enable-notification");
    itemSetting.reset("enable-preview");
    itemSetting.reset("enable-sound");
    itemSetting.reset("show-in-notification-center");
    itemSetting.reset("lockscreen-show-notification");

    Q_EMIT appRemovedSignal(id);
}

void NotifySettings::setAppSetting_v1(QString settings)
{
    QJsonObject jsonObj = QJsonDocument::fromJson(settings.toLocal8Bit()).object();
    QString id = jsonObj.begin().key();
    jsonObj = jsonObj.begin().value().toObject();
    QGSettings itemSetting(appSchemaKey.toLocal8Bit(), appSchemaPath.arg(id).toLocal8Bit(), this);
    itemSetting.set("enable-notification", jsonObj[AllowNotifyStr].toBool());
    itemSetting.set("show-in-notification-center", jsonObj[ShowInNotifyCenterStr].toBool());
    itemSetting.set("lockscreen-show-notification", jsonObj[LockShowNotifyStr].toBool());
    itemSetting.set("enable-preview", jsonObj[ShowNotifyPreviewStr].toBool());
    itemSetting.set("enable-sound", jsonObj[NotificationSoundStr].toBool());
    itemSetting.set("app-icon", jsonObj[AppIconStr].toString());
    itemSetting.set("app-name", jsonObj[AppNameStr].toString());
}

QString NotifySettings::getAppSettings_v1(const QString &id)
{
    QGSettings itemSetting(appSchemaKey.toLocal8Bit(), appSchemaPath.arg(id).toLocal8Bit(), this);
    QJsonObject jsonObj;
    jsonObj.insert(AllowNotifyStr, itemSetting.get("enable-notification").toJsonValue());
    jsonObj.insert(ShowInNotifyCenterStr, itemSetting.get("show-in-notification-center").toJsonValue());
    jsonObj.insert(LockShowNotifyStr, itemSetting.get("lockscreen-show-notification").toJsonValue());
    jsonObj.insert(ShowNotifyPreviewStr, itemSetting.get("enable-preview").toJsonValue());
    jsonObj.insert(NotificationSoundStr, itemSetting.get("enable-sound").toJsonValue());
    jsonObj.insert(AppIconStr, itemSetting.get("app-icon").toJsonValue());
    jsonObj.insert(AppNameStr, itemSetting.get("app-name").toJsonValue());
    QJsonObject appSetingObj;
    appSetingObj[id] = jsonObj;
    return QString(QJsonDocument(appSetingObj).toJson());
}

void NotifySettings::setSystemSetting_v1(QString settings)
{
    QJsonObject jsonObj = QJsonDocument::fromJson(settings.toUtf8()).object();
    jsonObj = jsonObj.begin().value().toObject();
    if (jsonObj.contains(DoNotDisturbStr)) {
        m_systemSetting->set("dndmode", jsonObj[DoNotDisturbStr].toBool());
        Q_EMIT systemSettingChanged(DNDMODE, jsonObj[DoNotDisturbStr].toBool());
    }
    if (jsonObj.contains(ScreenLockedStr)) {
        m_systemSetting->set("lockscreen-open-dndmode", jsonObj[ScreenLockedStr].toBool());
    }
    if (jsonObj.contains(TimeSlotStr)) {
        m_systemSetting->set("open-by-time-interval", jsonObj[TimeSlotStr].toBool());
    }
    if (jsonObj.contains(StartTimeStr)) {
        m_systemSetting->set("start-time", jsonObj[StartTimeStr].toString());
    }
    if (jsonObj.contains(EndTimeStr)) {
        m_systemSetting->set("end-time", jsonObj[EndTimeStr].toString());
    }
    if (jsonObj.contains(ShowIconOnDockStr)) {
        m_systemSetting->set("show-icon", jsonObj[ShowIconOnDockStr].toBool());
        Q_EMIT systemSettingChanged(SHOWICON, jsonObj[ShowIconOnDockStr].toBool());
    }
}

QString NotifySettings::getSystemSetings_v1()
{
    QJsonObject jsonObj;
    jsonObj.insert(DoNotDisturbStr, m_systemSetting->get("dndmode").toJsonValue());
    jsonObj.insert(ScreenLockedStr, m_systemSetting->get("lockscreen-open-dndmode").toJsonValue());
    jsonObj.insert(TimeSlotStr, m_systemSetting->get("open-by-time-interval").toJsonValue());
    jsonObj.insert(StartTimeStr, m_systemSetting->get("start-time").toJsonValue());
    jsonObj.insert(EndTimeStr, m_systemSetting->get("end-time").toJsonValue());
    jsonObj.insert(ShowIconOnDockStr, m_systemSetting->get("show-icon").toJsonValue());
    QJsonObject SystemSetingObj;
    SystemSetingObj[SystemNotifySettingStr] = jsonObj;
    return QString(QJsonDocument(SystemSetingObj).toJson());
}

void NotifySettings::setAllSetting_v1(QString settings)
{
    // 未被使用的接口废弃
    Q_UNUSED(settings)
}

QString NotifySettings::getAllSetings_v1()
{
    QStringList appList = m_systemSetting->get("app-list").toStringList();

    QJsonObject jsonObj;
    foreach (const auto &id, appList) {
        QGSettings itemSetting(appSchemaKey.toLocal8Bit(), appSchemaPath.arg(id).toLocal8Bit(), this);
        QJsonObject itemObj;
        itemObj.insert(AllowNotifyStr, itemSetting.get("enable-notification").toJsonValue());
        itemObj.insert(ShowInNotifyCenterStr, itemSetting.get("show-in-notification-center").toJsonValue());
        itemObj.insert(LockShowNotifyStr, itemSetting.get("lockscreen-show-notification").toJsonValue());
        itemObj.insert(ShowNotifyPreviewStr, itemSetting.get("enable-preview").toJsonValue());
        itemObj.insert(NotificationSoundStr, itemSetting.get("enable-sound").toJsonValue());
        itemObj.insert(AppIconStr, itemSetting.get("app-icon").toJsonValue());
        itemObj.insert(AppNameStr, itemSetting.get("app-name").toJsonValue());
        jsonObj[id] = itemObj;
    }

    QJsonObject Obj;
    Obj.insert(DoNotDisturbStr, m_systemSetting->get("dndmode").toJsonValue());
    Obj.insert(ScreenLockedStr, m_systemSetting->get("lockscreen-open-dndmode").toJsonValue());
    Obj.insert(TimeSlotStr, m_systemSetting->get("open-by-time-interval").toJsonValue());
    Obj.insert(StartTimeStr, m_systemSetting->get("start-time").toJsonValue());
    Obj.insert(EndTimeStr, m_systemSetting->get("end-time").toJsonValue());
    Obj.insert(ShowIconOnDockStr, m_systemSetting->get("show-icon").toJsonValue());
    jsonObj[SystemNotifySettingStr] = Obj;
    return QString(QJsonDocument(jsonObj).toJson());
}

// it exists in gsettings-qt package of util.h, but it not installed in dev package.
// and the symbol is default export in linux.
extern QString qtify_name(const char *name);
bool NotifySettings::containsAppSettings(const QGSettings &settings, const QString &id)
{
    const QString &key = qtify_name(id.toLocal8Bit().constData());
    if (settings.keys().contains(key))
        return true;

    return false;
}
