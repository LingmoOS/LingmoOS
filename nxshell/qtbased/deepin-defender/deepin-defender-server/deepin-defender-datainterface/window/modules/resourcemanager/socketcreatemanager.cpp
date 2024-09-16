// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "socketcreatemanager.h"
#include "../../deepin-defender/src/window/modules/common/gsettingkey.h"
#include "../../deepin-defender/src/window/modules/common/common.h"

#include <QtDBus/QtDBus>

#include <proc/readproc.h>
#include <fstream>

#define SPLIT_FLAG_1 "|" // 分割符 |
#define SPLIT_FLAG_2 "*" // 分割符 *
#define SPLIT_FLAG_3 "-" // 分割符 横杠
#define NET_ALLOW_ONCE 0 // 弹框第一个按钮
#define NET_ALLOW_ALWAYS 1 // 弹框第二个按钮
#define NET_DISABLE 2 // 弹框第三个按钮

static const int MINOTOR_FLAG_NET = 91;

//通过链接文件名获取被目标文件绝对路径
//为了防止buffer不够大，包装一下readlink()
QString getPackageFilePathByLink(const char *filename)
{
    QString retStr;
    long size = 100;
    char *buffer = nullptr;

    while (1) {
        buffer = new char[size];
        //readlink()返回的路径名并没有以'\0'结尾
        long nchars = readlink(filename, buffer, size - 1);
        if (nchars < 0) {
            break;
        }
        if (nchars <= size - 1) {
            buffer[nchars] = '\0'; //让路径名以'\0'结尾
            retStr = buffer;
            break;
        }

        size = nchars + 1; //buffer空间不足，扩大为2倍
        delete[] buffer;
    }
    delete[] buffer;
    return retStr;
}

SocketCreateManager::SocketCreateManager(QObject *parent)
    : QObject(parent)
    , m_gsNetControl(new QGSettings("com.deepin.dde.deepin-defender", QByteArray(), this))
    , m_resourceManagerInter(new ResourceManagerInter("com.deepin.daemon.ResourceManager", "/com/deepin/daemon/ResourceManager", QDBusConnection::systemBus(), this))
    , m_launcherInter(new LauncherInter("com.deepin.dde.daemon.Launcher", "/com/deepin/dde/daemon/Launcher", QDBusConnection::sessionBus(), this))
    , m_monitornetflow(new MonitorNetFlow("com.deepin.defender.MonitorNetFlow", "/com/deepin/defender/MonitorNetFlow", QDBusConnection::systemBus(), this))
    , m_dataInterFace(new DataInterFace("com.deepin.defender.datainterface", "/com/deepin/defender/datainterface", QDBusConnection::sessionBus(), this))
    , m_isbNetControlSwitch(false)
    , m_isbRegisterStatus(false)
{
    // 初始化本地存储信息
    initNetControlLocalInfo();

    // 注册LauncherItemInfo
    registerLauncherItemInfoListMetaType();
    registerLauncherItemInfoMetaType();
    // 启动器与应用商店应用程序更新信号接收
    connect(m_launcherInter, &LauncherInter::ItemChanged, m_monitornetflow, &MonitorNetFlow::RefreshPackageTable);

    // 包名解析数据库加载完成信号槽
    connect(m_monitornetflow, &MonitorNetFlow::NotifySqlInsertFinish, this, &SocketCreateManager::refreshNetControlStatus);

    // 初始化联网开关状态
    m_isbNetControlSwitch = m_gsNetControl->get(NET_CONTROL_ON_OFF).toBool();
    setNetControlSwitchStatus(m_isbNetControlSwitch);
    //启动安全中心服务
    m_monitornetflow->RefreshPackageTable();
}

SocketCreateManager::~SocketCreateManager()
{
}

/**************************联网管控进程号处理**********************************/
// 设置 联网/远程 开关状态
void SocketCreateManager::setNetControlSwitchStatus(bool isNetStatus)
{
    // 赋值
    m_isbNetControlSwitch = isNetStatus;

    // 注册传参
    int nPid = QString::number(QCoreApplication::applicationPid()).toInt();
    QString sCmd = QCoreApplication::applicationFilePath();
    QList<int> interest;
    interest.append(MINOTOR_FLAG_NET);

    // 开关打开并且未注册,则注册
    if (isNetStatus && !m_isbRegisterStatus) {
        // 注册联网管控接口
        connect(m_resourceManagerInter, &ResourceManagerInter::Notification, this, &SocketCreateManager::onAppNetworkConnect, Qt::QueuedConnection);
        m_uuid = m_resourceManagerInter->SubscribeControl(nPid, sCmd, "deepin-defender", interest);
        m_isbRegisterStatus = true;
    }

    // 开关关闭且已经注册,则注销
    if (!isNetStatus && m_isbRegisterStatus) {
        disconnect(m_resourceManagerInter, &ResourceManagerInter::Notification, this, &SocketCreateManager::onAppNetworkConnect);
        m_resourceManagerInter->Unsubscribe(m_uuid);
        m_isbRegisterStatus = false;
    }
}

bool SocketCreateManager::getNetControlSwitchStatus()
{
    return m_isbNetControlSwitch;
}

// 联网管控连接请求消息接收
void SocketCreateManager::onAppNetworkConnect(const QStringList &subscribers, const QString &msg)
{
    Q_UNUSED(subscribers);

    // 取pid 进程号 / 请求类型
    QStringList sList_Request = msg.split(SPLIT_FLAG_3);
    QString sPkgName, sPid;

    // 判断是否符合规范，不符合规范直接放行
    if (sList_Request.count() < 3) {
        m_resourceManagerInter->AllowRequest(m_uuid, msg, true);
        return;
    } else {
        // 只处理联网管控
        if (MINOTOR_FLAG_NET != sList_Request.at(0).toInt()) {
            return;
        }

        // 获取进程号
        sPid = sList_Request.at(sList_Request.count() - 1);
        sPkgName = getPkgNameByPid(sPid.toInt());
    }

    // 联网/远程 开关关闭，直接放行联网请求
    if (!m_isbNetControlSwitch || sPkgName.isEmpty()) {
        m_resourceManagerInter->AllowRequest(m_uuid, msg, true);
        return;
    }

    if (m_mapNetControlAppName[sPkgName].isEmpty()) {
        m_resourceManagerInter->AllowRequest(m_uuid, msg, true);
        return;
    }

    if (NET_STATUS_ITEMALLOW == m_mapNetControlInfo[sPkgName].toInt()) {
        m_disableDialog.removeOne(sPkgName);
        m_askDisableDialog.removeOne(sPkgName);
        m_oneAllowDialog.removeOne(sPkgName);
        m_resourceManagerInter->AllowRequest(m_uuid, msg, true);
    } else if (NET_STATUS_ITEMDISALLOW == m_mapNetControlInfo[sPkgName].toInt()) {
        m_askDisableDialog.removeOne(sPkgName);
        m_oneAllowDialog.removeOne(sPkgName);
        m_resourceManagerInter->AllowRequest(m_uuid, msg, false);
        if (!m_disableDialog.contains(sPkgName) || !m_appsStartList.contains(sPkgName)) {
            m_disableDialog.append(sPkgName);
            // 禁止弹框
            m_dataInterFace->showNetControlDialog(sPkgName, m_mapNetControlAppName.value(sPkgName));
        }
    } else {
        m_disableDialog.removeOne(sPkgName);
        if (m_oneAllowDialog.contains(sPkgName) && m_appsStartList.contains(sPkgName)) {
            m_resourceManagerInter->AllowRequest(m_uuid, msg, true);
        } else if (m_askDisableDialog.contains(sPkgName) && m_appsStartList.contains(sPkgName)) {
            m_resourceManagerInter->AllowRequest(m_uuid, msg, false);
        } else {
            // 询问弹框
            m_mapPidMsg[sPid] = msg;
            m_dataInterFace->showNetControlAskDialog(sPkgName, m_mapNetControlAppName.value(sPkgName), sPid);
        }
    }
}

// 联网管控列表 联网状态改变
void SocketCreateManager::setNetRequestReply(QString sPkgName, QString sId, int nType)
{
    QString sMsg = m_mapPidMsg[sId];
    m_mapPidMsg.remove(sId);

    if (nType == NET_DISABLE) {
        m_askDisableDialog.append(sPkgName);
        m_resourceManagerInter->AllowRequest(m_uuid, sMsg, false);
    } else {
        m_resourceManagerInter->AllowRequest(m_uuid, sMsg, true);
        if (nType == NET_ALLOW_ONCE) {
            m_oneAllowDialog.append(sPkgName);
        } else {
            m_mapNetControlInfo[sPkgName] = QString::number(NET_STATUS_ITEMALLOW);
            m_mapNetControl[sPkgName] = QString::number(NET_STATUS_ITEMALLOW);
            // 将数据集中数据保存到本地
            setGsLocalData(m_mapNetControlInfo);
            getNetControlAppInfos();
        }
    }
}

// 通过pid获取进程的包名
QString SocketCreateManager::getPkgNameByPid(int pid)
{
    // 初始包名为空
    QString sPkgName = "";
    QString sExePath = getProcExecPath(pid);
    if (sExePath.isEmpty()) {
        return "";
    }

    // 最常用方式 - 获取包名的第一种方式 直接通过执行路径从数据库获取
    sPkgName = m_monitornetflow->QueryPackageName(sExePath);

    // 异常情况处理 - 获取包名的第二种方式 通过环境变量中的desktop来获取包名
    if (sPkgName.isEmpty()) {
        QString sDesktopPath = getProcEnvProperties(pid, "GIO_LAUNCHED_DESKTOP_FILE");
        sPkgName = getPkgNameByDesktopPath(sDesktopPath);
    }

    // 异常情况处理 - 获取包名的第三种方式 获取环境变量中的包名
    if (sPkgName.isEmpty()) {
        sPkgName = getProcEnvProperties(pid, "DEB_PACKAGE_NAME");
    }

    return sPkgName;
}

// 通过pid获取进程执行路径
QString SocketCreateManager::getProcExecPath(pid_t pid) const
{
    // 待返回的进程名
    QString retStr;

    // 获取执行路径
    QString sExeLinkPath = "/proc/" + QString::number(pid) + "/exe";
    retStr = getPackageFilePathByLink(sExeLinkPath.toStdString().c_str());
    if (!retStr.isEmpty()) {
        return retStr;
    }

    // 如无法获取执行路径，则使用cmdline代替
    // 获取/prc/路径下对应pid目录下cmdline文件中内容
    std::string temp;
    try {
        std::fstream fs;
        fs.open("/proc/" + std::to_string(long(pid)) + "/cmdline", std::fstream::in);
        std::getline(fs, temp);
        fs.close();
    } catch (std::ifstream::failure e) {
        return "";
    }

    // 如果获取cmdline文件内容为空，则返回空名称
    if (temp.size() < 1) {
        return retStr;
    }

    // change \0 to ' '
    std::replace(temp.begin(), temp.end(), '\0', ' ');

    QString sTmpStr = QString::fromStdString(temp);
    // 按空格分割进程命令行
    QStringList retStrList = sTmpStr.split(" ");
    // 取字符串中"/"后的进程名
    if (0 < retStrList.count()) {
        retStr = retStrList.first();
    }

    return retStr;
}

QString SocketCreateManager::getProcEnvProperties(pid_t pid, QString sEvnKey)
{
    std::string temp;
    try {
        std::fstream fs;
        fs.open("/proc/" + std::to_string(pid) + "/environ", std::fstream::in);
        std::getline(fs, temp);
        fs.close();
    } catch (std::ios_base::failure e) {
        return "";
    }

    if (temp.size() < 1) {
        return "";
    }

    QStringList sEnvs = QString::fromStdString(temp).trimmed().split('\0');
    foreach (auto envValue, sEnvs) {
        if (envValue.startsWith(QString("%1=").arg(sEvnKey))) {
            return envValue.remove(0, sEvnKey.length() + 1);
        }
    }

    return "";
}

QString SocketCreateManager::getPkgNameByDesktopPath(QString sDesktopPath)
{
    // 初始包名为空
    QString sPkgName;

    QString sRealDesktopPath = getPackageFilePathByLink(sDesktopPath.toStdString().c_str());
    if (sRealDesktopPath.isEmpty()) {
        sRealDesktopPath = sDesktopPath;
    }

    // 获取包名的第一种方式 直接通过执行路径从数据库获取
    QDBusPendingReply<QString> reply = m_monitornetflow->QueryPackageName(sRealDesktopPath);
    reply.waitForFinished();
    if (reply.isError()) {
        qDebug() << reply.error();
    }

    sPkgName = reply.value();
    return sPkgName;
}

QString SocketCreateManager::getPPid(QString sPid)
{
    QString sPidPath = "/proc/" + sPid + "/status";
    QFile fileInfo(sPidPath);
    if (fileInfo.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&fileInfo);
        QString sAllInfo = in.readAll();

        QStringList sList = sAllInfo.split("/n", QString::SkipEmptyParts);
        for (int i = 0; i < sList.count(); ++i) {
            QString sStr = sList.at(i);
            if (sStr.contains("PPid")) {
                QStringList sStrList = sStr.split(":", QString::SkipEmptyParts);
                return sStrList.at(sStrList.count() - 1);
            }
        }
    }
    return "";
}

/**************************联网管控APP状态数据处理**********************************/
// 接收包名解析数据库插入数据完成信号槽
void SocketCreateManager::initNetControlLocalInfo()
{
    // 获取所有APP联网选项
    QString sNetAppsOption = m_gsNetControl->get(NET_CONTROL_APPS_STATUS).toString();

    // 初始化时拆分本地保存项，用QMap接收
    m_mapNetControl.clear();
    QStringList sNetAppsOptionList = sNetAppsOption.split(SPLIT_FLAG_1);
    for (int i = 0; i < sNetAppsOptionList.count(); ++i) {
        QStringList sNetAppsList = sNetAppsOptionList.at(i).split(SPLIT_FLAG_2);

        // 保证拆分的总数必须为2,否则崩溃
        if (sNetAppsList.count() >= 2) {
            m_mapNetControl[sNetAppsList.at(0)] = sNetAppsList.at(1);
        }
    }
}

// 刷新联网管控状态
void SocketCreateManager::refreshNetControlStatus()
{
    QDBusPendingReply<LauncherItemInfoList> reply = m_launcherInter->GetAllItemInfos();
    reply.waitForFinished();
    if (reply.isError()) {
        qDebug() << reply.error();
    }

    for (LauncherItemInfo launcherItemInfo : reply.value()) {
        QString sDesktopPath = launcherItemInfo.Path;
        QString sPkgName = getPkgNameByDesktopPath(sDesktopPath);
        if (sPkgName.isEmpty()) {
            continue;
        }

        if (!m_mapNetControl.contains(sPkgName)) {
            m_mapNetControlInfo[sPkgName] = QString::number(NET_STATUS_ITEMALLOW);
        } else {
            m_mapNetControlInfo[sPkgName] = m_mapNetControl[sPkgName];
        }
        m_mapNetControlAppName[sPkgName] = launcherItemInfo.Name;
    }

    // 将数据集中数据保存到本地
    setGsLocalData(m_mapNetControlInfo);
    getNetControlAppInfos();
}

// 联网管控列表 联网状态改变
void SocketCreateManager::setNetControlStatusChange(QString sPkgName, int nStatus)
{
    if (sPkgName.isEmpty()) {
        return;
    }

    m_mapNetControlInfo[sPkgName] = QString::number(nStatus);
    m_mapNetControl[sPkgName] = QString::number(nStatus);
    // 将数据集中数据保存到本地
    setGsLocalData(m_mapNetControlInfo);

    // 刷新APP数据信息
    getNetControlAppInfos();
}

// 得到联网管控所有APP信息
void SocketCreateManager::getNetControlAppInfos()
{
    // 用于接收APP的保存状态及其他所有信息
    QList<DefenderProcInfo> sNetAppInfoList;

    // 从启动器dbus服务中获取所有应用信息
    QDBusPendingReply<LauncherItemInfoList> reply = m_launcherInter->GetAllItemInfos();
    reply.waitForFinished();
    if (reply.isError()) {
        qDebug() << reply.error();
    }

    for (LauncherItemInfo launcherItemInfo : reply.value()) {
        DefenderProcInfo procInfo;
        procInfo.nPid = 0;

        // 获取执行路径
        QSettings readIniSettingMethod(launcherItemInfo.Path, QSettings::Format::IniFormat);
        readIniSettingMethod.beginGroup("Desktop Entry");
        procInfo.sExecPath = readIniSettingMethod.value("Exec").toString();
        readIniSettingMethod.endGroup();

        // 判断是否是系统应用
        procInfo.isbSysApp = true;
        if (procInfo.sExecPath.contains("/opt/"))
            procInfo.isbSysApp = false;

        // 获取进程名
        procInfo.sProcName = "";
        // 获取desktop路径
        procInfo.sDesktopPath = launcherItemInfo.Path;

        // 获取包名
        procInfo.sPkgName = getPkgNameByDesktopPath(procInfo.sDesktopPath);
        procInfo.sStatus = m_mapNetControlInfo[procInfo.sPkgName];
        procInfo.sDefaultStatus = QString::number(NET_STATUS_ITEMALLOW);

        // 获取 应用名称、图标
        procInfo.sAppName = launcherItemInfo.Name;
        procInfo.sThemeIcon = launcherItemInfo.Icon;
        // 获取应用启动器id
        procInfo.sID = launcherItemInfo.ID;
        // 获取应用启动器分类ID
        procInfo.nCategoryID = launcherItemInfo.CategoryID;
        // 获取应用安装时间
        procInfo.nTimeInstalled = launcherItemInfo.TimeInstalled;

        // 初始化流量信息
        procInfo.dDownloadSpeed = 0.0;
        procInfo.dUploadSpeed = 0.0;
        procInfo.dDownloads = 0.0;
        procInfo.dUploads = 0.0;
        sNetAppInfoList.append(procInfo);
    }

    Q_EMIT sendNetAppsChange(sNetAppInfoList);
}

// 将数据集中数据保存到本地
void SocketCreateManager::setGsLocalData(QMap<QString, QString> mapNetControl)
{
    // 清除本地配置
    reset();

    //加载数据
    QString str;
    QMapIterator<QString, QString> iter(mapNetControl);
    while (iter.hasNext()) {
        iter.next();
        str += QString("%1%2%3%4").arg(iter.key()).arg(SPLIT_FLAG_2).arg(iter.value()).arg(SPLIT_FLAG_1);
    }
    m_gsNetControl->set(NET_CONTROL_APPS_STATUS, str);
}

// 清除本地配置
void SocketCreateManager::reset()
{
    m_gsNetControl->reset(NET_CONTROL_APPS_STATUS);
}

// 应用程序开启关闭槽
void SocketCreateManager::doAppStartSlot(QStringList sAppList)
{
    m_appsStartList.clear();
    for (int i = 0; i < sAppList.count(); ++i) {
        m_appsStartList.append(sAppList.at(i));
    }
}

// 应用程序开启关闭槽
void SocketCreateManager::doAppEndSlot(QStringList sAppList)
{
    for (int i = 0; i < sAppList.count(); ++i) {
        QString sPkgName = sAppList.at(i);
        m_disableDialog.removeOne(sPkgName);
        m_askDisableDialog.removeOne(sPkgName);
        m_oneAllowDialog.removeOne(sPkgName);
        m_appsStartList.removeOne(sPkgName);
    }
}
