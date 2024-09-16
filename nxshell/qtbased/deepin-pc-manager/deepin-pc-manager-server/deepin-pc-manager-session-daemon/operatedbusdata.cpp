// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "operatedbusdata.h"

#include "../../deepin-pc-manager/src/window/modules/common/common.h"
#include "../../deepin-pc-manager/src/window/modules/common/gsettingkey.h"
#include "../../deepin-pc-manager/src/window/modules/common/invokers/invokerfactory.h"
#include "../../deepin-pc-manager/src/window/modules/common/systemsettings/systemsettings.h"
#include "common.h"
#include "dpinyin.h"

// 垃圾清理应用过滤类
#include "window/modules/trashclean/trashcleanappinfofilter.h"
#include "window/modules/trashclean/trashcleanbrowserinfofilter.h"

// 授权
#include "window/modules/authority/defenderauthoritywrapper.h"

// 悬浮框
#include "window/modules/hoverballs/hoverballsdlg.h"

#include <DPinyin>
#include <DSysInfo>
#include <DTrashManager>

#include <QDBusContext>
#include <QDebug>
#include <QEventLoop>
#include <QGSettings>
#include <QSysInfo>
#include <QtConcurrent>

#define Action_Flag_Disable 0 // 状态标志 - 不允许
#define Action_Flag_Enable 1 // 状态标志 - 允许

#define MIN_SYSTEM_LOG_SIZE_TO_CLEAN 100 * MB_COUNT

const QString WhitePathsFile = "/usr/share/deepin-pc-manager/cleanerwhite.json";
const QString appPathsFile = "/usr/share/deepin-pc-manager/appInfomation.json";
// 第三方病毒查杀应用程序信息
const QString natiavOptApps = "/usr/share/deepin-pc-manager/natiavoptapps.json";

// 应用项扫描时过滤文件名称
// 以正则形式执行
bool isFileinRegexpList(QString file, QStringList filter)
{
    foreach (const auto &target, filter) {
        if (!target.length()) {
            continue;
        }
        QRegularExpression regularExpression(target);
        if (regularExpression.match(file).hasMatch()) {
            return true;
        }
    }
    return false;
}

// 合法调用进程路径列表
const QStringList ValidInvokerExePathList = {"/usr/bin/deepin-pc-manager",
                                             "/usr/bin/deepin-pc-manager-session-daemon",
                                             "/usr/bin/deepin-pc-manager-system-daemon"};

OperateDBusData::OperateDBusData(QObject *parent)
    : QObject(parent)
    , m_daemonAdaptor(nullptr)
    , m_gsetting(new QGSettings(DEEPIN_PC_MANAGER_GSETTING_PATH, QByteArray(), this))
    , m_sysSettings(new SystemSettings(this))
    , m_launcherInter(nullptr)
    , m_startManagerInterface(nullptr)
    , m_managerInter(nullptr)
    , m_authWrapper(nullptr)
{
    m_daemonAdaptor = new DaemonAdaptor(this);
    if (!QDBusConnection::sessionBus().registerService(service)
        || !QDBusConnection::sessionBus().registerObject(path, this)) {
        exit(0);
    }

    registerLauncherItemInfoMetaType();

    m_authWrapper = new DefenderAuthorityWrapper(this);
    connect(m_authWrapper,
            &DefenderAuthorityWrapper::notifyAuthStarted,
            this,
            &OperateDBusData::NotifyAuthStarted);
    connect(m_authWrapper,
            &DefenderAuthorityWrapper::notifyAuthFinished,
            this,
            &OperateDBusData::NotifyAuthFinished);
    connect(m_authWrapper,
            &DefenderAuthorityWrapper::notifyAuthResult,
            this,
            &OperateDBusData::NotifyAuthResult);

    // 取应用数据接口
    m_launcherInter = InvokerFactory::GetInstance().CreateInvoker("com.deepin.dde.daemon.Launcher",
                                                                  "/com/deepin/dde/daemon/Launcher",
                                                                  "com.deepin.dde.daemon.Launcher",
                                                                  ConnectType::SESSION,
                                                                  this);

    m_rootMonitorDBusInter =
        InvokerFactory::GetInstance().CreateInvoker("com.deepin.pc.manager.system.daemon",
                                                    "/com/deepin/pc/manager/sysem/daemon",
                                                    "com.deepin.pc.manager.system.daemon",
                                                    ConnectType::SYSTEM,
                                                    this);
    m_managerInter = InvokerFactory::GetInstance().CreateInvoker("org.deepin.lastore1",
                                                                 "/org/deepin/lastore1",
                                                                 "org.deepin.lastore1.Manager",
                                                                 ConnectType::SYSTEM,
                                                                 this);
    m_startManagerInterface =
        InvokerFactory::GetInstance().CreateInvoker("com.deepin.SessionManager",
                                                    "/com/deepin/StartManager",
                                                    "com.deepin.StartManager",
                                                    ConnectType::SESSION,
                                                    this);

    // 初始化联网管控
    initialize();

    // 初始化获取系统架构
    initSystemArchitecture();

    // 电脑管家悬浮框
    HoverBallsDlg *ballsDlg = new HoverBallsDlg;
    ballsDlg->show();
    ballsDlg->raise();
    ballsDlg->activateWindow();
}

OperateDBusData::~OperateDBusData()
{
}

void OperateDBusData::StartApp()
{
}

void OperateDBusData::ExitApp()
{
    if (!isValidInvoker()) {
        qDebug() << Q_FUNC_INFO << "invalid invoker!";
        return;
    }
    qApp->quit();
}

void OperateDBusData::preInitialize()
{
}

void OperateDBusData::initialize()
{
}

/*---------------------开机自启动管理---------------------*/
bool OperateDBusData::startLauncherManage()
{
    m_enableAppsInfo.clear();
    m_disableAppsInfo.clear();
    m_enableData.clear();
    m_disableData.clear();

    QDBusMessage msg = DBUS_BLOCK_INVOKE(m_launcherInter, "GetAllItemInfos");
    QDBusArgument arg = msg.arguments().first().value<QDBusArgument>();
    QList<LauncherItemInfo> datas;
    arg >> datas;

    // 添加数据到QMap容器
    m_mapEnable.clear();
    m_mapDisable.clear();
    for (const auto &it : datas) {
        QString sInfo = it.Path + "," + it.Icon + "," + it.ID + "," + it.Name;

        // 判断该应用状态（是否是自启动）
        bool bStatus = isAutostart(it.Path);

        QString sName = DTK_CORE_NAMESPACE::Chinese2Pinyin(it.Name);
        for (auto iterEnable = m_mapEnable.begin(); iterEnable != m_mapEnable.end(); ++iterEnable) {
            if (sName == iterEnable.key())
                sName = sName + "_deepin";
        }

        for (auto iterDisable = m_mapDisable.begin(); iterDisable != m_mapDisable.end();
             ++iterDisable) {
            if (sName == iterDisable.key())
                sName = sName + "_deepin";
        }
        sInfo = sInfo + "," + sName;

        if (bStatus) {
            m_enableData.append(sInfo);
            m_mapEnable[sName] = sInfo;
        } else {
            m_disableData.append(sInfo);
            m_mapDisable[sName] = sInfo;
        }
    }

    for (int i = 0; i < m_enableData.count(); ++i) {
        if (i == 0)
            m_enableAppsInfo = m_enableData.at(0);
        else
            m_enableAppsInfo = m_enableAppsInfo + "|" + m_enableData.at(i);
    }

    for (int i = 0; i < m_disableData.count(); ++i) {
        if (i == 0)
            m_disableAppsInfo = m_disableData.at(0);
        else
            m_disableAppsInfo = m_disableAppsInfo + "|" + m_disableData.at(i);
    }

    return true;
}

QString OperateDBusData::getAppsInfoEnable()
{
    return m_enableAppsInfo;
}

QString OperateDBusData::getAppsInfoDisable()
{
    return m_disableAppsInfo;
}

// 判断是否是开机自启
bool OperateDBusData::isAutostart(QString sPath)
{
    QDBusMessage msg = DBUS_BLOCK_INVOKE(m_startManagerInterface, "IsAutostart", sPath);
    GET_MESSAGE_VALUE(bool, flag, msg);

    return flag;
}

// 执行开机自启动管理 开启/禁止操作
bool OperateDBusData::exeAutostart(int nStatus, QString sPath)
{
    // 执行自启动关闭操作
    if (Action_Flag_Disable == nStatus) {
        QDBusMessage msg = DBUS_BLOCK_INVOKE(m_startManagerInterface, "RemoveAutostart", sPath);
        GET_MESSAGE_VALUE(bool, flag, msg);
        return flag;
    } else {
        // 执行自启动开启操作
        QDBusMessage msg = DBUS_BLOCK_INVOKE(m_startManagerInterface, "AddAutostart", sPath);
        GET_MESSAGE_VALUE(bool, flag, msg);
        return flag;
    }
}

// 外部调用刷新表格数据
void OperateDBusData::refreshData(bool bAdd, QString sID)
{
    Q_EMIT AccessRefreshData(bAdd, sID);
}

// 垃圾清理 - 回收站
QStringList OperateDBusData::GetTrashInfoList()
{
    // 获取回收站目录下所有文件
    m_fileList.clear();
    QEventLoop loop;
    QtConcurrent::run([&] {
        QThread::msleep(50);
        scanFile(QDir::homePath() + CLEANER_SYSTEM_TRASH_PATH);
        loop.exit();
    });
    loop.exec();

    foreach (const QString &file, m_fileList) {
        QFileInfo info(file);
        m_dbCleanerSum += info.size();
        m_fileListSum.append(file);
    }
    return m_fileList;
}

// 垃圾清理 - 日志
QStringList OperateDBusData::GetLogInfoList()
{
    // 获取回收站目录下所有文件
    m_fileList.clear();
    QEventLoop loop;
    QtConcurrent::run([&] {
        QThread::msleep(50);
        scanFile(CLEANER_SYSTEM_LOG_PATH);
        loop.exit();
    });
    loop.exec();

    // 系统日志过滤掉白名单
    double sizeCounter = 0.0;
    QStringList sPathList;
    QFile whiteFile(WhitePathsFile);
    if (!whiteFile.open(QFile::ReadOnly)) {
        qDebug() << "can not find white file:" << WhitePathsFile;
    }

    // 获取白名单数据
    QJsonParseError err;
    const QJsonObject &object = QJsonDocument::fromJson(whiteFile.readAll(), &err).object();
    whiteFile.close();

    if (QJsonParseError::NoError != err.error) {
        return QStringList();
    }
    const QJsonArray &apps = object.value("whitepaths").toArray();

    // 判断是否该目录下文件是否加入到白名单，如果加入，直接跳过
    foreach (const QString &file, m_fileList) {
        if (file.isEmpty()) {
            break;
        }

        bool bAddList = false;
        // 是否是/var/log/journal目录下的文件
        if (file.contains("/var/log/journal")) {
            QFileInfo info(file);
            sizeCounter += info.size();
            sPathList.append(file);
        } else {
            // 黑名单 - 只清除黑名单内带后缀的文件
            for (auto itApp = apps.begin(); itApp != apps.end(); ++itApp) {
                QString sPath = itApp->toObject().value("white_path").toString();
                if (file.contains(sPath) && file != sPath) {
                    bAddList = true;
                }
            }
        }

        // 添加符合规则的文件到清理项
        if (bAddList) {
            QFileInfo info(file);
            sizeCounter += info.size();
            sPathList.append(file);
        }
    }

    // 仅超过100M时对系统日志提供清理
    if (sizeCounter > MIN_SYSTEM_LOG_SIZE_TO_CLEAN) {
        m_fileListSum.append(sPathList);
        m_dbCleanerSum += sizeCounter;
    } else {
        sPathList.clear();
    }

    return sPathList;
}

// 垃圾清理 - 缓存
// 应用laststore接口,获取可以安全删除的deb包路径
QStringList OperateDBusData::GetCacheInfoList()
{
    QStringList debFileList;
    QString debJsonStr;
    QEventLoop loop;
    QtConcurrent::run([&] {
        QThread::msleep(50);
        QDBusMessage msg = DBUS_BLOCK_INVOKE(m_managerInter, "GetArchivesInfo");
        debJsonStr = QDBusReply<QString>(msg).value();
        loop.exit();
    });
    loop.exec();

    QJsonParseError err;
    QJsonObject object = QJsonDocument::fromJson(debJsonStr.toUtf8(), &err).object();
    if (QJsonParseError::NoError != err.error) {
        qDebug() << "incorrect json format:" << debJsonStr;
        return debFileList;
    }

    // 获取/var/cache/apt/archives 可清理deb缓存
    QJsonArray debObjArrayApt = object.value(CLEANER_SYSTEM_CACHE_ARRAY_NAME)
                                    .toObject()
                                    .value(CLEANER_SYSTEM_CACHE_APT_PATH)
                                    .toArray();
    foreach (const auto &it, debObjArrayApt) {
        QString debName = it.toObject().value(CLEANER_SYSTEM_CACHE_DEB_NAME).toString();

        if (debName.isEmpty()) {
            continue;
        }

        // 设置文件完整路径
        QString debFilePath = QString("%1%2%3")
                                  .arg(CLEANER_SYSTEM_CACHE_APT_PATH)
                                  .arg(QDir::separator())
                                  .arg(debName);
        QFileInfo info(debFilePath);
        if (info.exists()) {
            debFileList.append(debFilePath);
            m_fileListSum.append(debFilePath);
            m_dbCleanerSum += info.size();
        }
    }

    // 获取/var/cache/lastore/archives 可清理deb缓存
    QJsonArray debObjArray = object.value(CLEANER_SYSTEM_CACHE_ARRAY_NAME)
                                 .toObject()
                                 .value(CLEANER_SYSTEM_CACHE_LASTORE_PATH)
                                 .toArray();
    foreach (const auto &it, debObjArray) {
        QString debName = it.toObject().value(CLEANER_SYSTEM_CACHE_DEB_NAME).toString();

        if (debName.isEmpty()) {
            continue;
        }

        // 设置文件完整路径
        QString debFilePath = QString("%1%2%3")
                                  .arg(CLEANER_SYSTEM_CACHE_LASTORE_PATH)
                                  .arg(QDir::separator())
                                  .arg(debName);
        QFileInfo info(debFilePath);
        if (info.exists()) {
            debFileList.append(debFilePath);
            m_fileListSum.append(debFilePath);
            m_dbCleanerSum += info.size();
        }
    }

    return debFileList;
}

// 垃圾清理 - 痕迹
QStringList OperateDBusData::GetHistoryInfoList()
{
    // 统计清理文件大小和路径
    QFileInfo info(QDir::homePath() + CLEANER_SYSTEM_HISTORY_PATH);
    m_dbCleanerSum += info.size();
    m_fileListSum.append(info.absoluteFilePath());

    // 获取回收站目录下所有文件
    m_fileList.clear();
    scanFile(QDir::homePath() + CLEANER_SYSTEM_HISTORY_PATH);

    return m_fileList;
}

// 浏览器cookies
// 文件暂时没有清单,需要下一步清查各浏览器cookies位置
QString OperateDBusData::GetBrowserCookiesInfoList()
{
    QJsonDocument appInfoDoc;
    QJsonArray appInfoForUI;

    TrashCleanBrowserInfoFilter filter;

    bool isInitSuccess = filter.FromJson(appPathsFile);
    if (!isInitSuccess) {
        return QString();
    }

    TrashCleanAppList appList = filter.ToList();

    QEventLoop loop;
    QtConcurrent::run([&] {
        QThread::msleep(50);
        foreach (const auto &app, appList) {
            QJsonObject cookiesInfo;

            m_fileList.clear();
            // 在目录下获取文件路径
            foreach (const auto &dir, app.browserCookiesPaths) {
                scanFile(QDir::homePath() + dir);
            }
            // 填充cookies路径
            QJsonArray cookies;
            foreach (const auto &cookiesFile, m_fileList) {
                cookies.insert(0, QJsonValue(cookiesFile));
            }
            cookiesInfo[CLEANER_APP_NAME] = app.appDisplayName;
            cookiesInfo[CLEANER_BROWSER_COOKIES_PATH] = cookies;

            m_fileList.clear();
            appInfoForUI.insert(0, cookiesInfo);
        }
        loop.exit();
    });
    loop.exec();

    QJsonObject root;
    root.insert(CLEANER_BROWSER_JSON_NAME, appInfoForUI);
    appInfoDoc.setObject(root);
    return appInfoDoc.toJson();
}

// 垃圾清理 - 应用
// 重构,以结构体返回
// 由于应用存在层次关系，此处返回json字符串以记录层次
QString OperateDBusData::GetAppTrashInfoList()
{
    // 通过过滤器获取应用配置
    QJsonArray appInfoForUI; // 发给前端的json内容
    TrashCleanAppInfoFilter filter;
    bool isInitSuccess = filter.FromJson(appPathsFile);
    if (!isInitSuccess) {
        return QString();
    }

    TrashCleanAppList appList = filter.ToList();

    // 由于另外模块中使用double型作为大小值类型。此处也使用double
    // 该map用于元素自动排序
    QMultiMap<qint64, QJsonObject> appTrashMap;

    QEventLoop loop;
    QtConcurrent::run([&] {
        QThread::msleep(50);
        foreach (const auto &app, appList) {
            QJsonObject appInfo;
            qint64 appTrashSize = 0;
            appInfo[CLEANER_APP_COM_NAME] = app.appPkgName;
            appInfo[CLEANER_APP_NAME] = app.appDisplayName;
            appInfo[CLEANER_APP_INSTALL_FLAG] = app.isInstalled;

            // 获取应用的白名单
            QStringList whiteList;
            foreach (const auto &filterItem, app.whiteList) {
                whiteList.append(filterItem);
            }

            m_fileList.clear();
            // 在目录下获取cache文件路径
            foreach (const auto &cacheDir, app.appCachePaths) {
                QString path = cacheDir;
                scanFile(QDir::homePath() + path);
            }
            // 填充JSON cache路径
            QJsonArray cache;
            foreach (const auto &cacheFile, m_fileList) {
                // 如果在白名单中，跳过
                if (isFileinRegexpList(cacheFile, whiteList)) {
                    continue;
                }
                cache.insert(0, QJsonValue(cacheFile));
                QFileInfo info(cacheFile);
                if (info.exists()) {
                    appTrashSize += info.size();
                    m_fileListSum.append(cacheFile);
                }
            }

            // 将黑名单内容加入到cache项以进行日常清理
            foreach (const auto &blackFile, app.blackList) {
                QString fileName = blackFile;
                if (!fileName.length()) {
                    continue;
                }
                fileName = QDir::homePath() + fileName;
                QFileInfo info(fileName);
                if (info.exists()) {
                    cache.insert(0, QJsonValue(fileName));
                    appTrashSize += info.size();
                    m_fileListSum.append(fileName);
                }
            }

            appInfo[CLEANER_APP_CACHE_PATH] = cache;
            m_fileList.clear();

            foreach (const auto &configDir, app.appConfigPaths) {
                scanFile(QDir::homePath() + configDir);
            }

            QJsonArray config;
            if (!app.isInstalled) {
                // 填充config路径

                foreach (const auto &configFile, m_fileList) {
                    // 如果在白名单中，跳过
                    if (isFileinRegexpList(configFile, whiteList)) {
                        continue;
                    }
                    config.insert(0, QJsonValue(configFile));

                    // 卸载残留追加记录
                    if (!app.isInstalled) {
                        QFileInfo info(configFile);
                        if (info.exists()) {
                            appTrashSize += info.size();
                            m_fileListSum.append(configFile);
                        }
                    }
                }
            }
            appInfo[CLEANER_APP_CONFIG_PATH] = config;

            // 通过MAP进行升序排列
            // 存在同样大小的项，不要使用insert()
            appTrashMap.insert(appTrashSize, appInfo);

            m_dbCleanerSum += appTrashSize;
        }
        loop.exit();
    });
    loop.exec();

    foreach (auto it, appTrashMap) {
        // 降序
        // 输出到界面此时MAP中最大值排在0索引处
        appInfoForUI.insert(0, it);
    }

    QJsonObject root;
    root.insert(CLEANER_APP_JSON_NAME, appInfoForUI);
    QJsonDocument appInfoDoc;
    appInfoDoc.setObject(root);
    return QString(appInfoDoc.toJson());
}

// 获取单个文件的大小，同时记录文件总数
void OperateDBusData::scanFile(const QString &path)
{
    QFileInfo info(path);
    if (info.exists()) {
        if (info.isFile()) {
            // 用户显示在主界面的标题内容，由于文件路径是同检查项层次决定的，所以不需要该文件的路径信息
            QString abpath = info.absoluteFilePath();
            m_fileList.append(abpath);
        } else if (info.isDir()) {
            QDir dir(path);
            for (const QFileInfo &i :
                 dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs | QDir::Hidden
                                   | QDir::NoSymLinks)) {
                // 递归扫描目录内容
                scanFile(i.absoluteFilePath());
            }
        }
    }
}

// 首页体检垃圾清理项大小
double OperateDBusData::GetCleanerSum()
{
    m_dbCleanerSum = 0.00;
    m_fileListSum.clear();

    // 获取垃圾清理项
    GetTrashInfoList();
    GetLogInfoList();
    GetCacheInfoList();
    GetAppTrashInfoList();

    return m_dbCleanerSum;
}

// 首页体检清理所有垃圾
void OperateDBusData::DeleteAllCleaner()
{
    if (!isValidInvoker()) {
        qDebug() << Q_FUNC_INFO << "invalid invoker!";
        return;
    }
    if (!m_homepageCleanerResultFileList.empty()) {
        // 首页清理一定会清空回收站，所以在这里使用DKT功能清空回收站
        DTrashManager::instance()->cleanTrash();
        // 删除其它文件
        DeleteSpecifiedFiles(m_homepageCleanerResultFileList);
        m_homepageCleanerResultFileList.clear();
        m_homepageCleanerResultSize = 0;
    }
}

// 请求开始垃圾文件扫描
void OperateDBusData::RequestStartTrashScan()
{
    GetCleanerSum();
    m_homepageCleanerResultFileList = m_fileListSum;
    m_homepageCleanerResultSize = m_dbCleanerSum;

    // 发送扫描结果
    Q_EMIT TrashScanFinished(m_homepageCleanerResultSize);
}

// 请求清理选中的垃圾文件
void OperateDBusData::RequestCleanSelectTrash()
{
    if (!isValidInvoker()) {
        qDebug() << Q_FUNC_INFO << "invalid invoker!";
        return;
    }
    DeleteAllCleaner();
    // 清理完成
    Q_EMIT CleanSelectTrashFinished();
}

// 清理指定的用户文件
void OperateDBusData::DeleteSpecifiedFiles(QStringList filePaths)
{
    if (!isValidInvoker()) {
        qDebug() << Q_FUNC_INFO << "invalid invoker!";
        return;
    }
    DBUS_NOBLOCK_INVOKE(m_rootMonitorDBusInter, "CleanSelectFile", filePaths);
}

// 发生应用安装或卸载时的响应
void OperateDBusData::OnLauncherItemChanged(const QString &action,
                                            const LauncherItemInfo &appInfo,
                                            qint64 code)
{
    Q_UNUSED(code);

    QString appId = appInfo.ID;
    QString appName = appInfo.Name;
    if ("created" == action) {
        // 删除记录
        if (!appId.isEmpty()) {
            DBUS_NOBLOCK_INVOKE(m_rootMonitorDBusInter, "DeleteUninstalledAppRecord", appId);
        }
        return;
    }

    if ("deleted" == action) {
        // 更新记录，先删除后插入以更新数据库记录的ID值
        if (!appId.isEmpty()) {
            TrashCleanAppInfoFilter filter;
            filter.FromJson(appPathsFile);
            if (filter.isExistInConfig(appId)) {
                DBUS_NOBLOCK_INVOKE(m_rootMonitorDBusInter, "DeleteUninstalledAppRecord", appId);
                DBUS_NOBLOCK_INVOKE(m_rootMonitorDBusInter,
                                    "InsertUninstalledAppRecord",
                                    appId,
                                    appName);
            }
        }
        return;
    }
}

// 初始化获取系统架构
void OperateDBusData::initSystemArchitecture()
{
    QProcess fileCheck;
    fileCheck.start("uname", {"-m"});
    fileCheck.waitForFinished(1000);
    m_sSystemArchitecture = fileCheck.readAll().trimmed();
}

QString OperateDBusData::GetSystemArchitecture()
{
    return m_sSystemArchitecture;
}

void OperateDBusData::RequestAuthWithID(const QString &moduleName, int id)
{
    if (!isValidInvoker()) {
        qDebug() << Q_FUNC_INFO << "invalid invoker!";
        return;
    }
    Q_EMIT m_authWrapper->onAuthRequest(moduleName, id);
}

void OperateDBusData::ModulesRequestAuthorityActived()
{
    if (!isValidInvoker()) {
        qDebug() << Q_FUNC_INFO << "invalid invoker!";
        return;
    }
    Q_EMIT m_authWrapper->onModulesActived();
}

// 校验调用者
bool OperateDBusData::isValidInvoker()
{
#ifdef QT_DEBUG
    return true;
#else
    // 判断是否位自身调用
    if (!calledFromDBus()) {
        return true;
    }

    bool valid = false;
    QDBusConnection conn = connection();
    QDBusMessage msg = message();

    // 判断是否存在执行路径且是否存在于可调用者名单中
    uint pid = conn.interface()->servicePid(msg.service()).value();
    QFileInfo f(QString("/proc/%1/exe").arg(pid));
    QString invokerPath = f.canonicalFilePath();

    // 防止读取的执行路径为空，为空时取cmdline内容
    if (invokerPath.isEmpty()) {
        QFile cmdlineFile(QString("/proc/%1/cmdline").arg(pid));
        if (cmdlineFile.open(QIODevice::OpenModeFlag::ReadOnly)) {
            QString sCmdline = cmdlineFile.readAll();
            invokerPath = sCmdline.split(" ").first();
        }
        cmdlineFile.close();
    }

    if (ValidInvokerExePathList.contains(invokerPath)) {
        valid = true;
    }

    // 如果是非法调用者，则输出错误
    if (!valid) {
        sendErrorReply(QDBusError::ErrorType::Failed,
                       QString("(pid: %1)[%2] is not allowed to transfer antiav inter")
                           .arg(pid)
                           .arg(invokerPath));
    }

    return valid;
#endif
}
