// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "operatedbusdata.h"
#include "common.h"
#include "../../deepin-defender/src/window/modules/common/common.h"
#include "../../deepin-defender/src/window/modules/common/invokers/invokerfactory.h"
#include "../../deepin-defender/src/window/modules/common/invokers/invokerinterface.h"

// 联网管控/远程访问
#include "window/modules/resourcemanager/appsstarttoend.h"
#include "window/modules/resourcemanager/socketcreatemanager.h"
#include "window/modules/resourcemanager/socketlistenmanager.h"
#include "window/modules/resourcemanager/resourcedialog/netdisabledialog.h"
#include "window/modules/resourcemanager/resourcedialog/netaskdialog.h"
#include "window/modules/resourcemanager/resourcedialog/remdisabledialog.h"
#include "window/modules/resourcemanager/resourcedialog/remaskdialog.h"

// 开机自启动
#include "window/modules/startup/dbustartmanager.h"

#include <DPinyin>
#include <DSysInfo>
#include <DTrashManager>

#include <QSysInfo>
#include <QDebug>
#include <QGSettings>

#define Action_Flag_Disable 0 // 状态标志 - 不允许
#define Action_Flag_Enable 1 // 状态标志 - 允许

const QString WhitePathsFile = "/usr/share/deepin-defender/cleanerwhite.json";
const QString appPathsFile = "/usr/share/deepin-defender/appInfomation.json";

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

OperateDBusData::OperateDBusData(QObject *parent)
    : QObject(parent)
    , m_gsetting(new QGSettings(DEEPIN_DEFENDER_GSETTING_PATH, QByteArray(), this))
    , m_adaptpr(nullptr)
    , m_socketCreateManage(nullptr)
    , m_socketListenManage(nullptr)
    , m_isAppsStartEndStatus(true)
    , m_netDisableDialog(nullptr)
    , m_netAskDialog(nullptr)
    , m_remDisableDialog(nullptr)
    , m_remAskDialog(nullptr)
    , m_launcherInter(new Launcherd("com.deepin.dde.daemon.Launcher", "/com/deepin/dde/daemon/Launcher", QDBusConnection::sessionBus(), this))
    , m_startManagerInterface(new DBusStartManager)
    , m_monitorNetFlowInter(nullptr)
    , m_managerInter(new LastoreManager("com.deepin.lastore", "/com/deepin/lastore", QDBusConnection::systemBus(), this))
    , m_loginSafetyModel(nullptr)
    , m_appStoreClientInvokerInter(nullptr)
    , m_resourceManagerInvokerInter(nullptr)
{
    m_adaptpr = new DatainterfaceAdaptor(this);
    if (!QDBusConnection::sessionBus().registerService(service) || !QDBusConnection::sessionBus().registerObject(path, this)) {
        qDebug() << "dbus service already registered! " << service;

        exit(0);
    }

    m_monitorNetFlowInter = new NetFlowMonitorInter("com.deepin.defender.MonitorNetFlow",
                                                    "/com/deepin/defender/MonitorNetFlow",
                                                    QDBusConnection::systemBus(), this);

    InvokerFactory *factory = new InvokerFactory(this);
    m_appStoreClientInvokerInter = factory->CreateInvoker("com.home.appstore.client",
                                                          "/com/home/appstore/client",
                                                          "com.home.appstore.client",
                                                          ConnectType::SESSION, this);
    m_resourceManagerInvokerInter = factory->CreateInvoker("com.deepin.daemon.ResourceManager",
                                                           "/com/deepin/daemon/ResourceManager",
                                                           "com.deepin.daemon.ResourceManager",
                                                           ConnectType::SYSTEM, this);

    // 初始化联网管控
    initialize();

    // 登录安全
    m_loginSafetyModel = new LoginSafetySrvModel(this);
    connect(m_loginSafetyModel, &LoginSafetySrvModel::setPwdLimitPolicyEnableFinished, this, &OperateDBusData::SetPwdLimitPolicyEnableFinished);
    connect(m_loginSafetyModel, &LoginSafetySrvModel::setPwdLimitedLevelFinished, this, &OperateDBusData::SetPwdLimitedLevelFinished);
}

OperateDBusData::~OperateDBusData()
{
    m_appsStartToEnd->stopTimer();
    m_appsStartToEndThread->quit();
    m_appsStartToEndThread->wait();
    m_appsStartToEndThread->deleteLater();
    m_appsStartToEndThread = nullptr;
    m_appsStartToEnd->deleteLater();
    m_appsStartToEnd = nullptr;
}

void OperateDBusData::StartApp()
{
}

void OperateDBusData::ExitApp()
{
    qApp->quit();
}

void OperateDBusData::preInitialize()
{
}

void OperateDBusData::initialize()
{
    // 流量详情
    registerDefenderProcInfoMetaType();
    registerDefenderProcInfoListMetaType();
    registerDefenderProcessListMetaType();

    // 初始化联网管控
    initNetControlSocket();

    //usb管控
    connect(m_monitorNetFlowInter, &MonitorNetFlow::requestShowBlockAllSysNotify, this, &OperateDBusData::showBlockAllSysNotify);
    connect(m_monitorNetFlowInter, &MonitorNetFlow::requestShowOnlyWhiteListSysNotify, this, &OperateDBusData::showOnlyWhiteListSysNotify);
}

void OperateDBusData::initNetControlSocket()
{
    // 应用程序开启关闭监控类
    m_appsStartToEnd = new AppsStartToEnd;
    m_appsStartToEndThread = new QThread(this);
    m_appsStartToEnd->moveToThread(m_appsStartToEndThread);
    connect(m_appsStartToEndThread, &QThread::started, m_appsStartToEnd, &AppsStartToEnd::initAppsStartEndThread);
    connect(this, &OperateDBusData::notifyStartAppCheckTimer, m_appsStartToEnd, &AppsStartToEnd::startTimer);
    connect(this, &OperateDBusData::notifyStopAppCheckTimer, m_appsStartToEnd, &AppsStartToEnd::stopTimer);
    m_appsStartToEndThread->start();

    // 联网管控
    m_socketCreateManage = new SocketCreateManager(this);
    connect(m_socketCreateManage, SIGNAL(sendNetAppsChange(QList<DefenderProcInfo>)), this, SLOT(setNetAppsChange(QList<DefenderProcInfo>)));
    connect(m_appsStartToEnd, &AppsStartToEnd::sendAppsStart, m_socketCreateManage, &SocketCreateManager::doAppStartSlot);
    connect(m_appsStartToEnd, &AppsStartToEnd::sendAppsEnd, m_socketCreateManage, &SocketCreateManager::doAppEndSlot);

    // 远程访问
    m_socketListenManage = new SocketListenManager(this);
    connect(m_appsStartToEnd, &AppsStartToEnd::sendAppsStart, m_socketListenManage, &SocketListenManager::doAppStartSlot);
    connect(m_appsStartToEnd, &AppsStartToEnd::sendAppsEnd, m_socketListenManage, &SocketListenManager::doAppEndSlot);

    // 设置APP开启关闭监控状态
    setAppStartEndStatus();
}

/*---------------------新联网管控/远程访问---------------------*/
void OperateDBusData::setNetControlSwitchStatus(bool isbFireWall)
{
    m_socketCreateManage->setNetControlSwitchStatus(isbFireWall);
    setAppStartEndStatus();
}

void OperateDBusData::setRemControlSwitchStatus(bool isbFireWall)
{
    m_socketListenManage->setRemControlSwitchStatus(isbFireWall);
    setAppStartEndStatus();
}

// 设置APP开启关闭监控状态
void OperateDBusData::setAppStartEndStatus()
{
    // 如联网和远程开关均关闭，并且此时app开启关闭监控状态为true，则停止定时器
    if (!m_socketCreateManage->getNetControlSwitchStatus()
        && !m_socketListenManage->getRemControlSwitchStatus() && m_isAppsStartEndStatus) {
        Q_EMIT this->notifyStopAppCheckTimer();
        m_isAppsStartEndStatus = false;
        return;
    }

    // 如果上述条件不满并且 状态为false 则打开监控
    if (!m_isAppsStartEndStatus) {
        Q_EMIT this->notifyStartAppCheckTimer();
        m_isAppsStartEndStatus = true;
    }
}

void OperateDBusData::getNetAppsInfo()
{
    m_socketCreateManage->getNetControlAppInfos();
}

void OperateDBusData::setNetAppsChange(QList<DefenderProcInfo> sNetAppsList)
{
    Q_EMIT m_adaptpr->requestNetAppsChange(sNetAppsList);
}

void OperateDBusData::setNetAppStatusChange(QString sDkgName, int nStatus)
{
    m_socketCreateManage->setNetControlStatusChange(sDkgName, nStatus);
}

void OperateDBusData::setNetRequestReply(QString sPkgName, QString sId, int nType)
{
    m_socketCreateManage->setNetRequestReply(sPkgName, sId, nType);
}

void OperateDBusData::setNetControlDefaultStatus(int nDefaultStatus)
{
    Q_UNUSED(nDefaultStatus);
}

int OperateDBusData::getNetControlDefaultStatus()
{
    return NET_STATUS_ITEMALLOW;
}

void OperateDBusData::getRemAppsInfo()
{
}

void OperateDBusData::setRemAppsChange(QList<DefenderProcInfo> sNetAppsList)
{
    Q_EMIT m_adaptpr->requestRemAppsChange(sNetAppsList);
}

void OperateDBusData::setRemAppStatusChange(QString sDkgName, int nStatus)
{
    Q_UNUSED(sDkgName);
    Q_UNUSED(nStatus);
}

void OperateDBusData::setRemRequestReply(QString sPkgName, QString sId, int nType)
{
    m_socketListenManage->setRemRequestReply(sPkgName, sId, nType);
}

void OperateDBusData::setRemControlDefaultStatus(int nDefaultStatus)
{
    m_socketListenManage->setRemControlDefaultStatus(nDefaultStatus);
}

int OperateDBusData::getRemControlDefaultStatus()
{
    return m_socketListenManage->getRemControlDefaultStatus();
}

void OperateDBusData::setRemDefaultAllowStatus()
{
    Q_EMIT m_adaptpr->requestRemDefaultAllowStatus();
}

// 联网管控禁止弹框
void OperateDBusData::showNetControlDialog(const QString &sPkgName, const QString &sAppName)
{
    if (m_netDisableDialog == nullptr) {
        m_netDisableDialog = new NetDisableDialog(sPkgName, sAppName);
        connect(m_netDisableDialog, &NetDisableDialog::onDialogClose, this, [=]() {
            m_netDisableDialog = nullptr;
        });
        m_netDisableDialog->setAttribute(Qt::WA_DeleteOnClose);
        m_netDisableDialog->setOnButtonClickedClose(true);
        m_netDisableDialog->show();
    }
}

// 联网管控询问弹框
void OperateDBusData::showNetControlAskDialog(const QString &sPkgName, const QString &sAppName, const QString &sId)
{
    if (m_netAskDialog == nullptr) {
        m_netAskDialog = new NetAskDialog(sPkgName, sAppName, sId);
        connect(m_netAskDialog, &NetAskDialog::onDialogClose, this, [=]() {
            m_netAskDialog = nullptr;
        });
        m_netAskDialog->setAttribute(Qt::WA_DeleteOnClose);
        m_netAskDialog->setOnButtonClickedClose(true);
        m_netAskDialog->show();
    }
}

// 远程访问禁止弹框
void OperateDBusData::showRemControlDialog(const QString &sPkgName, const QString &sAppName)
{
    if (m_remDisableDialog == nullptr) {
        m_remDisableDialog = new RemDisableDialog(sPkgName, sAppName);
        connect(m_remDisableDialog, &RemDisableDialog::onDialogClose, this, [=]() {
            m_remDisableDialog = nullptr;
        });
        m_remDisableDialog->setAttribute(Qt::WA_DeleteOnClose);
        m_remDisableDialog->setOnButtonClickedClose(true);
        m_remDisableDialog->show();
    }
}

// 远程访问询问弹框
void OperateDBusData::showRemControlAskDialog(const QString &sPkgName, const QString &sAppName, const QString &sId)
{
    if (m_remAskDialog == nullptr) {
        m_remAskDialog = new RemAskDialog(sPkgName, sAppName, sId);
        connect(m_remAskDialog, &RemAskDialog::onDialogClose, this, [=]() {
            m_remAskDialog = nullptr;
        });
        m_remAskDialog->setAttribute(Qt::WA_DeleteOnClose);
        m_remAskDialog->setOnButtonClickedClose(true);
        m_remAskDialog->show();
    }
}

/*---------------------开机自启动管理---------------------*/
bool OperateDBusData::startLauncherManage()
{
    m_enableAppsInfo.clear();
    m_disableAppsInfo.clear();
    m_enableData.clear();
    m_disableData.clear();

    m_reply = m_launcherInter->GetAllItemInfos();
    m_reply.waitForFinished();
    if (m_reply.isError()) {
        qWarning() << "data is empty, quit!!";
        qWarning() << m_reply.error();
    }

    // 添加数据到QMap容器
    m_mapEnable.clear();
    m_mapDisable.clear();
    const LauncherItemInfoList &datas = m_reply.value();
    for (const auto &it : datas) {
        QString sInfo = it.Path
                        + "," + it.Icon
                        + "," + it.ID
                        + "," + it.Name;

        // 判断该应用状态（是否是自启动）
        bool bStatus = isAutostart(it.Path);

        QString sName = DTK_CORE_NAMESPACE::Chinese2Pinyin(it.Name);
        for (auto iterEnable = m_mapEnable.begin(); iterEnable != m_mapEnable.end(); ++iterEnable) {
            if (sName == iterEnable.key())
                sName = sName + "_deepin";
        }

        for (auto iterDisable = m_mapDisable.begin(); iterDisable != m_mapDisable.end(); ++iterDisable) {
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
    return m_startManagerInterface->IsAutostart(sPath);
}

// 执行开机自启动管理 开启/禁止操作
bool OperateDBusData::exeAutostart(int nStatus, QString sPath)
{
    bool flag = false;
    // 执行自启动关闭操作
    if (Action_Flag_Disable == nStatus) {
        QDBusPendingReply<bool> reply = m_startManagerInterface->RemoveAutostart(sPath);
        reply.waitForFinished();
        if (!reply.isError()) {
            flag = true;
        }
    } else {
        // 执行自启动开启操作
        QDBusPendingReply<bool> reply = m_startManagerInterface->AddAutostart(sPath);
        reply.waitForFinished();
        if (!reply.isError()) {
            flag = true;
        }
    }

    return flag;
}
// 外部调用刷新表格数据
void OperateDBusData::refreshData(bool bAdd, QString sID)
{
    Q_EMIT AccessRefreshData(bAdd, sID);
}

void OperateDBusData::showBlockAllSysNotify()
{
    QDBusMessage m = QDBusMessage::createMethodCall("com.deepin.dde.Notification", "/com/deepin/dde/Notification", "com.deepin.dde.Notification", "Notify");
    QStringList action;
    action << "_open1" << tr("Settings"); //添加按钮
    QVariantMap inform; //按钮的点击操作
    QString openSysSafetyPageCmd = QString("qdbus,com.deepin.defender.hmiscreen,"
                                           "/com/deepin/defender/hmiscreen,"
                                           "com.deepin.defender.hmiscreen.ShowPage,"
                                           "%1,%2")
                                       .arg(MODULE_SECURITY_TOOLS_NAME)
                                       .arg(USB_CONN_NAME);

    inform.insert(QString("x-deepin-action-_open1"), openSysSafetyPageCmd);
    m << QString("deepin-defender")
      << uint(0)
      << QString("deepin-defender")
      << QString("") << tr("Your system is set to block all USB storage devices")
      << action
      << inform
      << int(5000);
    QDBusMessage respone = QDBusConnection::sessionBus().call(m);
}

void OperateDBusData::showOnlyWhiteListSysNotify()
{
    QDBusMessage m = QDBusMessage::createMethodCall("com.deepin.dde.Notification", "/com/deepin/dde/Notification", "com.deepin.dde.Notification", "Notify");
    QStringList action;
    action << "_open1" << tr("Settings"); //添加按钮
    QVariantMap inform; //按钮的点击操作
    QString openSysSafetyPageCmd = QString("qdbus,com.deepin.defender.hmiscreen,"
                                           "/com/deepin/defender/hmiscreen,"
                                           "com.deepin.defender.hmiscreen.ShowPage,"
                                           "%1,%2")
                                       .arg(MODULE_SECURITY_TOOLS_NAME)
                                       .arg(USB_CONN_NAME);

    inform.insert(QString("x-deepin-action-_open1"), openSysSafetyPageCmd);
    m << QString("deepin-defender")
      << uint(0)
      << QString("deepin-defender")
      << QString("") << tr("Your USB device is not in the whitelist and has been blocked")
      << action
      << inform
      << int(5000);
    QDBusMessage respone = QDBusConnection::sessionBus().call(m);
}

// 垃圾清理 - 回收站
QStringList OperateDBusData::GetTrashInfoList()
{
    // 获取回收站目录下所有文件
    m_fileList.clear();
    scanFile(QDir::homePath() + CLEANER_SYSTEM_TRASH_PATH);
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
    scanFile(CLEANER_SYSTEM_LOG_PATH);

    // 系统日志过滤掉白名单
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
            sPathList.append(file);
            QFileInfo info(file);
            m_dbCleanerSum += info.size();
            m_fileListSum.append(file);
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
            sPathList.append(file);
            QFileInfo info(file);
            m_dbCleanerSum += info.size();
            m_fileListSum.append(file);
        }
    }

    return sPathList;
}

// 垃圾清理 - 缓存
// 应用laststore接口,获取可以安全删除的deb包路径
QStringList OperateDBusData::GetCacheInfoList()
{
    QStringList debFileList;
    QString debJsonStr = m_managerInter->GetArchivesInfo();

    QJsonParseError err;
    QJsonObject object = QJsonDocument::fromJson(debJsonStr.toUtf8(), &err).object();
    if (QJsonParseError::NoError != err.error) {
        qDebug() << "incorrect json format:" << debJsonStr;
        return debFileList;
    }

    // 获取/var/cache/apt/archives 可清理deb缓存
    QJsonArray debObjArrayApt = object.value(CLEANER_SYSTEM_CACHE_ARRAY_NAME).toObject().value(CLEANER_SYSTEM_CACHE_APT_PATH).toArray();
    foreach (const auto &it, debObjArrayApt) {
        QString debName = it.toObject().value(CLEANER_SYSTEM_CACHE_DEB_NAME).toString();

        if (debName.isEmpty()) {
            continue;
        }

        // 设置文件完整路径
        QString debFilePath = QString("%1%2%3").arg(CLEANER_SYSTEM_CACHE_APT_PATH).arg(QDir::separator()).arg(debName);
        QFileInfo info(debFilePath);
        if (info.exists()) {
            debFileList.append(debFilePath);
            m_fileListSum.append(debFilePath);
            m_dbCleanerSum += info.size();
        }
    }

    // 获取/var/cache/lastore/archives 可清理deb缓存
    QJsonArray debObjArray = object.value(CLEANER_SYSTEM_CACHE_ARRAY_NAME).toObject().value(CLEANER_SYSTEM_CACHE_LASTORE_PATH).toArray();
    foreach (const auto &it, debObjArray) {
        QString debName = it.toObject().value(CLEANER_SYSTEM_CACHE_DEB_NAME).toString();

        if (debName.isEmpty()) {
            continue;
        }

        // 设置文件完整路径
        QString debFilePath = QString("%1%2%3").arg(CLEANER_SYSTEM_CACHE_LASTORE_PATH).arg(QDir::separator()).arg(debName);
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

    QJsonParseError err;
    // 读app.json获取名称和路径信息
    QFile appfile(appPathsFile);
    if (!appfile.open(QFile::ReadOnly)) {
        qDebug() << "can not find app info file:" << appPathsFile;
        return QString();
    }

    QJsonObject object = QJsonDocument::fromJson(appfile.readAll(), &err).object();
    appfile.close();

    if (QJsonParseError::NoError != err.error) {
        qDebug() << "incorrect json format:" << appPathsFile;
        return QString();
    }

    // 获取已安装应用信息，如果浏览器被安装才处理cookies
    m_reply = m_launcherInter->GetAllItemInfos();
    m_reply.waitForFinished();
    if (m_reply.isError()) {
        qWarning() << "data is empty, quit!!";
        qWarning() << m_reply.error();
        return QString();
    }
    const LauncherItemInfoList &datas = m_reply.value();

    // 以安装软件的字符串ID为内容创建列表
    // 考虑可能存在同名软件
    QStringList installedApp;
    for (const auto &it : datas) {
        installedApp.append(it.ID);
    }

    QJsonArray browsers = object.value(CLEANER_BROWSER_JSON_NAME).toArray();
    foreach (const auto &it, browsers) {
        QJsonObject cookiesInfo;
        auto appObj = it.toObject();
        QString browserName = appObj.value(CLEANER_APP_NAME).toString();
        // 判断浏览器是否已安装，未安装则不处理
        if (!installedApp.contains(browserName)) {
            continue;
        }
        m_fileList.clear();
        // 在目录下获取文件路径
        foreach (const auto &dir, appObj.value(CLEANER_BROWSER_COOKIES_PATH).toArray()) {
            QString path = dir.toString();
            scanFile(QDir::homePath() + path);
        }
        // 填充cookies路径
        QJsonArray cookies;
        foreach (const auto &cookiesFile, m_fileList) {
            cookies.insert(0, QJsonValue(cookiesFile));
        }

        cookiesInfo[CLEANER_APP_COM_NAME] = appObj.value(CLEANER_APP_COM_NAME);
        cookiesInfo[CLEANER_APP_NAME] = appObj.value(CLEANER_APP_NAME);
        cookiesInfo[CLEANER_APP_TIP] = appObj.value(CLEANER_APP_TIP);
        cookiesInfo[CLEANER_BROWSER_COOKIES_PATH] = cookies;

        m_fileList.clear();
        appInfoForUI.insert(0, cookiesInfo);
    }

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
    QJsonDocument appInfoDoc;
    QJsonParseError err;
    // 读app.json获取名称和路径信息
    QFile appfile(appPathsFile);
    if (!appfile.open(QFile::ReadOnly)) {
        qDebug() << "can not find app info file:" << appPathsFile;
    }

    QJsonObject object = QJsonDocument::fromJson(appfile.readAll(), &err).object();
    appfile.close();

    if (QJsonParseError::NoError != err.error) {
        qDebug() << "incorrect json format:" << appPathsFile;
        return QString();
    }

    // 获取已安装应用信息，从JSON数组中将已安装的路径信息删除
    m_reply = m_launcherInter->GetAllItemInfos();
    m_reply.waitForFinished();
    if (m_reply.isError()) {
        qWarning() << "data is empty, quit!!";
        qWarning() << m_reply.error();
    }
    // 从json中清除所有已安装的内容
    const LauncherItemInfoList &datas = m_reply.value();

    // 以安装软件的字符串ID为内容创建列表
    // 考虑可能存在同名软件
    QStringList installedApp;
    for (const auto &it : datas) {
        installedApp.append(it.ID);
    }
    QJsonArray apps = object.value(CLEANER_APP_JSON_NAME).toArray();
    QJsonArray appInfoForUI;
    // 重建发送给前端的JSON
    foreach (const auto &it, apps) {
        QJsonObject appInfo;
        auto appObj = it.toObject();
        QString tmpName = appObj.value(CLEANER_APP_NAME).toString();

        appInfo[CLEANER_APP_COM_NAME] = appObj.value(CLEANER_APP_COM_NAME);
        appInfo[CLEANER_APP_NAME] = appObj.value(CLEANER_APP_NAME);
        appInfo[CLEANER_APP_TIP] = appObj.value(CLEANER_APP_TIP);
        appInfo[CLEANER_APP_INSTALL_FLAG] = QJsonValue(installedApp.contains(tmpName));

        // 获取应用的白名单
        QStringList whiteList;
        foreach (const auto &filterItem, appObj.value(CLEANER_APP_WHITE_LIST).toArray()) {
            whiteList.append(filterItem.toString());
        }

        m_fileList.clear();
        // 在目录下获取文件路径
        foreach (const auto &cacheDir, appObj.value(CLEANER_APP_CACHE_PATH).toArray()) {
            QString path = cacheDir.toString();
            scanFile(QDir::homePath() + path);
        }
        // 填充cache路径
        QJsonArray cache;
        foreach (const auto &cacheFile, m_fileList) {
            // 如果在白名单中，跳过
            if (isFileinRegexpList(cacheFile, whiteList)) {
                continue;
            }
            cache.insert(0, QJsonValue(cacheFile));
            QFileInfo info(cacheFile);
            m_dbCleanerSum += info.size();
            m_fileListSum.append(cacheFile);
        }

        // 将黑名单内容加入到cache项以进行日常清理
        foreach (const auto &fileNameObj, appObj.value(CLEANER_APP_BLACK_LIST).toArray()) {
            QString fileName = fileNameObj.toString();
            if (!fileName.length()) {
                continue;
            }
            fileName = QDir::homePath() + fileName;
            QFileInfo info(fileName);
            if (info.exists()) {
                cache.insert(0, QJsonValue(fileName));
                m_dbCleanerSum += info.size();
                m_fileListSum.append(fileName);
            }
        }

        appInfo[CLEANER_APP_CACHE_PATH] = cache;
        m_fileList.clear();

        foreach (const auto &configDir, appObj.value(CLEANER_APP_CONFIG_PATH).toArray()) {
            scanFile(QDir::homePath() + configDir.toString());
        }
        // 填充config路径
        QJsonArray config;
        foreach (const auto &configFile, m_fileList) {
            // 如果在白名单中，跳过
            if (isFileinRegexpList(configFile, whiteList)) {
                continue;
            }
            config.insert(0, QJsonValue(configFile));
        }
        appInfo[CLEANER_APP_CONFIG_PATH] = config;

        appInfoForUI.insert(0, appInfo);
    }

    QJsonObject root;
    root.insert(CLEANER_APP_JSON_NAME, appInfoForUI);
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
            m_fileList.append(info.absoluteFilePath());
        } else if (info.isDir()) {
            QDir dir(path);
            for (const QFileInfo &i : dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs | QDir::Hidden | QDir::NoSymLinks)) {
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
    Q_EMIT m_adaptpr->TrashScanFinished(m_homepageCleanerResultSize);
}

// 请求清理选中的垃圾文件
void OperateDBusData::RequestCleanSelectTrash()
{
    DeleteAllCleaner();
    // 清理完成
    Q_EMIT m_adaptpr->CleanSelectTrashFinished();
}

// 获取密码限制策略开启/关闭 状态
bool OperateDBusData::GetPwdLimitPolicyEnable() const
{
    return m_loginSafetyModel->getPwdLimitPolicyEnable();
}

// 开启/关闭密码限制策略
void OperateDBusData::SetPwdLimitPolicyEnable(const bool &enable)
{
    m_loginSafetyModel->setPwdLimitPolicyEnable(enable);
}

// 设置密码限制等级
void OperateDBusData::SetPwdLimitedLevel(const int &level)
{
    m_loginSafetyModel->setPwdLimitedLevel(level);
}

// 获得密码安全错误信息
QString OperateDBusData::GetPwdChangeError() const
{
    return m_loginSafetyModel->getPwdChangeError();
}

// 清理指定的用户文件
void OperateDBusData::DeleteSpecifiedFiles(QStringList filePaths)
{
    m_monitorNetFlowInter->CleanSelectFile(filePaths);
}

// 获取uos-resource-manager此时是否为异常状态
bool OperateDBusData::OperateDBusData::GetUosResourceStatus()
{
    QVariantList args = m_resourceManagerInvokerInter->Invoke(DBUS_PROPERTY_INVOKER_NAME, {"CapabilitiesOfLocalEnv"}, BlockMode::BLOCK).arguments();

    QVariant var = args.first();
    QDBusVariant dbvFirst = var.value<QDBusVariant>();
    QVariant vFirst = dbvFirst.variant();

    // 属性为 int数组转换
    QDBusArgument dbusArgs = vFirst.value<QDBusArgument>();
    QList<int> nTypeList;
    dbusArgs >> nTypeList;

    for (int i = 0; i < nTypeList.count(); ++i) {
        if (91 == nTypeList.at(i) || 92 == nTypeList.at(i)) {
            return true;
        }
    }

    return false;
}
