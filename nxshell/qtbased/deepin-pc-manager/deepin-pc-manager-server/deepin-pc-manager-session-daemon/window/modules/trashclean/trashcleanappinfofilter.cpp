// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trashcleanappinfofilter.h"

#include "../../deepin-pc-manager/src/window/modules/common/common.h"
#include "../../deepin-pc-manager/src/window/modules/common/invokers/invokerfactory.h"
#include "window/modules/common/defenderlauncherinfo.h"

#include <QDBusReply>
#include <QtDBus/QDBusInterface>

#define LAUNCHER_PATH "/com/deepin/dde/daemon/Launcher"
#define LAUNCHER_SERVICE "com.deepin.dde.daemon.Launcher"
#define LAUNCHER_INTERFACE "com.deepin.dde.daemon.Launcher"

TrashCleanAppInfoFilter::TrashCleanAppInfoFilter(QObject *parent)
    : QObject(parent)
{
    // 向qt注册用于dbus返回解析的数据结构类型
    registerLauncherItemInfoMetaType();
    registerLauncherItemInfoListMetaType();
    m_monitorInterFaceServer =
        InvokerFactory::GetInstance().CreateInvoker("com.deepin.pc.manager.system.daemon",
                                                    "/com/deepin/pc/manager/system/daemon",
                                                    "com.deepin.pc.manager.system.daemon",
                                                    ConnectType::SYSTEM,
                                                    this);
}

TrashCleanAppInfoFilter::~TrashCleanAppInfoFilter()
{
    reset();
}

// 配置文件是不变的，即只需调用一次fromjson，可调用多次tolist更新结果
TrashCleanAppList TrashCleanAppInfoFilter::ToList()
{
    MergeAppInfo();
    TrashCleanAppList appList;

    foreach (const auto &appInfo, m_appInfoList) {
        if (isValidConfig(appInfo)) {
            appList.append(appInfo);
        }
    }

    return appList;
}

// 从配置文件中初步构建数据结构
// 之后与其它信息检查冲突并合并
bool TrashCleanAppInfoFilter::FromJson(QString jsonPath)
{
    reset();

    m_appInfoList = InitAppData(jsonPath);

    if (m_appInfoList.size()) {
        return true;
    }

    return false;
}

// 外部调用，确保写数据库的对象存在于配置中
bool TrashCleanAppInfoFilter::isExistInConfig(const QString &appID)
{
    foreach (const auto &app, m_appInfoList) {
        if (app.appPkgName == appID) {
            return true;
        }
    }

    return false;
}

// 注意此方法在单元测试中应事先mock
QMap<QString, QString> TrashCleanAppInfoFilter::GetInstalledApps()
{
    // 从接口获取已安装应用
    // 注意，非成员变量或没有必要不使用指针
    // 使用局部变量或RAII封装
    QMap<QString, QString> appInfoMap;
    QDBusInterface launchInterface(LAUNCHER_SERVICE,
                                   LAUNCHER_PATH,
                                   LAUNCHER_INTERFACE,
                                   QDBusConnection::sessionBus());

    QDBusReply<LauncherItemInfoList> reply = launchInterface.call("GetAllItemInfos");
    if (!reply.isValid()) {
        return appInfoMap;
    }

    LauncherItemInfoList installedApps = reply.value();
    foreach (auto &installedApp, installedApps) {
        appInfoMap.insert(installedApp.ID, installedApp.Name);
    }

    return appInfoMap;
}

// 注意此方法在单元测试中应事先mock
QList<QPair<QString, QString>> TrashCleanAppInfoFilter::GetUnInstalledApps()
{
    QList<QPair<QString, QString>> appInfoList;
    QDBusMessage msgLog = DBUS_BLOCK_INVOKE(m_monitorInterFaceServer, "GetUnInstalledApps");
    GET_MESSAGE_VALUE(QVariantList, varList, msgLog);
    QStringList line;
    foreach (auto record, varList) {
        line.clear();
        QSequentialIterable iterable = record.value<QSequentialIterable>();
        for (const QVariant &v : iterable) {
            line.append(v.toString());
        }
        // 每行只有两个元素
        if (line.size() != 2) {
            continue;
        }
        appInfoList.append(QPair<QString, QString>(line.at(0), line.at(1)));
    }

    return appInfoList;
}

// 公有化虚函数以提供子类对json的其它处理
// 注意不要使用私有变量，否则将阻止子类重写
TrashCleanAppList TrashCleanAppInfoFilter::InitAppData(QString jsonPath)
{
    TrashCleanAppList appList;
    // 处理json信息
    // 需要从JSON中取出包名与配置
    QJsonDocument appInfoDoc;
    QJsonParseError err;
    // 读app.json获取名称和路径信息
    QFile appfile(jsonPath);
    if (!appfile.open(QFile::ReadOnly)) {
        qDebug() << "can not find app info file:" << jsonPath;
        return appList;
    }

    QJsonObject object = QJsonDocument::fromJson(appfile.readAll(), &err).object();
    appfile.close();

    if (QJsonParseError::NoError != err.error) {
        qDebug() << "incorrect json format:" << jsonPath;
        return appList;
    }
    QJsonArray apps = object.value(CLEANER_APP_JSON_NAME).toArray();

    foreach (const auto &it, apps) {
        TrashCleanApp appData;
        auto appObj = it.toObject();
        // 取出ID 路径配置
        QString appID = appObj.value(CLEANER_APP_ID).toString();
        if (appID.isEmpty()) {
            continue;
        }

        appData.appPkgName = appID;

        // 应用白名单
        foreach (const auto &whiteFile, appObj.value(CLEANER_APP_WHITE_LIST).toArray()) {
            appData.whiteList.append(whiteFile.toString());
        }

        // 应用黑名单
        foreach (const auto &blackFile, appObj.value(CLEANER_APP_BLACK_LIST).toArray()) {
            appData.blackList.append(blackFile.toString());
        }

        // 应用 config 路径配置
        foreach (const auto &configDir, appObj.value(CLEANER_APP_CONFIG_PATH).toArray()) {
            appData.appConfigPaths.append(configDir.toString());
        }

        // 应用 cache 路径配置
        foreach (const auto &cacheDir, appObj.value(CLEANER_APP_CACHE_PATH).toArray()) {
            appData.appCachePaths.append(cacheDir.toString());
        }
        appList.append(appData);
    }

    return appList;
}

// 公有虚方法，以供子类重写
void TrashCleanAppInfoFilter::MergeAppInfo()
{
    MergeInstalledAppInfo(GetInstalledApps());
    MergeUninstalledAppInfo(GetUnInstalledApps());
}

void TrashCleanAppInfoFilter::MergeInstalledAppInfo(const QMap<QString, QString> &installedApps)
{
    for (auto it = m_appInfoList.begin(); it != m_appInfoList.end(); it++) {
        auto installIt = installedApps.find(it->appPkgName);
        if (installedApps.end() == installIt) {
            continue;
        }
        it->appDisplayName = installIt.value();
        it->isInstalled = true;

        // 开始检查目录配置冲突
        // 已安装应用不处理config路径，但是需要记录以避免卸载项意外删除
        bool isMutipleVersionApp = false;
        foreach (auto path, it->appCachePaths) {
            if (m_installedAppPaths.contains(path)) {
                it->appCachePaths.removeAll(path);
                // 路径与已记载内容发生冲突，可以是多版本的应用
                isMutipleVersionApp = true;
            }
        }

        if (isMutipleVersionApp && it->appCachePaths.isEmpty()) {
            // 多版本应用只显示其中一个的处理
            // 清空后,后续判断不会将其作为有效检查项
            it->appConfigPaths.clear();
        }

        m_installedAppPaths.append(it->appCachePaths);
        m_installedAppPaths.append(it->appConfigPaths); // 避免config配置目录被删除
        // 存在性能问题，但当前场景下应当不会有大影响，数据量应该少于30
        m_installedAppPaths =
            QSet<QString>(m_installedAppPaths.begin(), m_installedAppPaths.end()).values();
    }
}

// uninstalledApps内容，应由数据库返回，ID从大到小排序列
void TrashCleanAppInfoFilter::MergeUninstalledAppInfo(
    const QList<QPair<QString, QString>> &uninstalledApps)
{
    auto findItemInList = [&](const QString appID, QString &appName) {
        foreach (auto it, uninstalledApps) {
            if (it.first == appID) {
                appName = it.second;
                return true;
            }
        }
        return false;
    };

    for (auto it = m_appInfoList.begin(); it != m_appInfoList.end(); it++) {
        if (it->isInstalled) {
            continue;
        }
        QString appName;
        if (findItemInList(it->appPkgName, appName)) {
            it->appDisplayName = appName;
        } else {
            continue;
        }

        // 开始检查目录配置冲突
        // cache路径冲突检查
        foreach (auto path, it->appCachePaths) {
            if (m_installedAppPaths.contains(path)) {
                it->appCachePaths.removeAll(path);
            }

            // 最后卸载的应用优先处理
            // 之前卸载的应用，冲突时不再处理
            if (m_uninstalledAppPaths.contains(path)) {
                it->appCachePaths.removeAll(path);
            }
        }
        // config路径冲突检查
        foreach (auto path, it->appConfigPaths) {
            if (m_installedAppPaths.contains(path)) {
                it->appConfigPaths.removeAll(path);
            }

            if (m_uninstalledAppPaths.contains(path)) {
                it->appConfigPaths.removeAll(path);
            }
        }

        m_uninstalledAppPaths.append(it->appCachePaths);
        m_uninstalledAppPaths.append(it->appConfigPaths);
        m_uninstalledAppPaths =
            QSet<QString>(m_uninstalledAppPaths.begin(), m_uninstalledAppPaths.end()).values();
    }
}

void TrashCleanAppInfoFilter::reset()
{
    m_installedAppPaths.clear();
    m_uninstalledAppPaths.clear();
    m_appInfoList.clear();
}

bool TrashCleanAppInfoFilter::isValidConfig(const TrashCleanApp &app)
{
    if (app.appDisplayName.isEmpty()) {
        return false;
    }

    if (app.appCachePaths.size() || app.appConfigPaths.size()) {
        return true;
    }
    return false;
}
