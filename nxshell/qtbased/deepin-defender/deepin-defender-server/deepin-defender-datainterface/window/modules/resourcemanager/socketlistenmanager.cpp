// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "socketlistenmanager.h"
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

static const int MINOTOR_FLAG_CONTROL = 92;

//通过链接文件名获取被目标文件绝对路径
//为了防止buffer不够大，包装一下readlink()
QString getListenPackageFilePathByLink(const char *filename)
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

SocketListenManager::SocketListenManager(QObject *parent)
    : QObject(parent)
    , m_gsRemControl(new QGSettings("com.deepin.dde.deepin-defender", QByteArray(), this))
    , m_resourceManagerInter(new ResourceManagerInter("com.deepin.daemon.ResourceManager", "/com/deepin/daemon/ResourceManager", QDBusConnection::systemBus(), this))
    , m_launcherInter(new LauncherInter("com.deepin.dde.daemon.Launcher", "/com/deepin/dde/daemon/Launcher", QDBusConnection::sessionBus(), this))
    , m_monitornetflow(new MonitorNetFlow("com.deepin.defender.MonitorNetFlow", "/com/deepin/defender/MonitorNetFlow", QDBusConnection::systemBus(), this))
    , m_dataInterFace(new DataInterFace("com.deepin.defender.datainterface", "/com/deepin/defender/datainterface", QDBusConnection::sessionBus(), this))
    , m_isbRemControlSwitch(false)
    , m_isbRegisterStatus(false)
{
    // 包名解析数据库加载完成信号槽
    connect(m_monitornetflow, &MonitorNetFlow::NotifySqlInsertFinish, this, &SocketListenManager::refreshRemControlStatus);

    // 初始化远程访问开关状态
    m_isbRemControlSwitch = m_gsRemControl->get(REM_CONTROL_ON_OFF).toBool();
    setRemControlSwitchStatus(m_isbRemControlSwitch);
}

SocketListenManager::~SocketListenManager()
{
}

/**************************远程访问进程号处理**********************************/
// 设置 联网/远程 开关状态
void SocketListenManager::setRemControlSwitchStatus(bool isRemStatus)
{
    // 赋值
    m_isbRemControlSwitch = isRemStatus;

    // 注册传参
    int nPid = QString::number(QCoreApplication::applicationPid()).toInt();
    QString sCmd = QCoreApplication::applicationFilePath();
    QList<int> interest;
    interest.append(MINOTOR_FLAG_CONTROL);

    // 开关打开并且未注册,则注册
    if (isRemStatus && !m_isbRegisterStatus) {
        // 注册联网管控接口
        connect(m_resourceManagerInter, &ResourceManagerInter::Notification, this, &SocketListenManager::onAppRemworkConnect, Qt::QueuedConnection);
        m_uuid = m_resourceManagerInter->SubscribeControl(nPid, sCmd, "deepin-defender", interest);
        m_isbRegisterStatus = true;
    }

    // 开关关闭且已经注册,则注销
    if (!isRemStatus && m_isbRegisterStatus) {
        disconnect(m_resourceManagerInter, &ResourceManagerInter::Notification, this, &SocketListenManager::onAppRemworkConnect);
        m_resourceManagerInter->Unsubscribe(m_uuid);
        m_isbRegisterStatus = false;
    }
}

bool SocketListenManager::getRemControlSwitchStatus()
{
    return m_isbRemControlSwitch;
}

// 远程访问连接请求消息接收
void SocketListenManager::onAppRemworkConnect(const QStringList &subscribers, const QString &msg)
{
    Q_UNUSED(subscribers);

    m_nRemOption = m_gsRemControl->get(REM_CONTROL_DEFAULT_STATUS).toInt();

    // 取pid 进程号 / 请求类型
    QStringList sList_Request = msg.split(SPLIT_FLAG_3);
    QString sPkgName, sPid;

    // 判断是否符合规范，不符合规范直接放行
    if (sList_Request.count() < 3) {
        m_resourceManagerInter->AllowRequest(m_uuid, msg, true);
        return;
    } else {
        // 只处理远程访问管控
        if (MINOTOR_FLAG_CONTROL != sList_Request.at(0).toInt()) {
            return;
        }

        // 获取进程号
        sPid = sList_Request.at(sList_Request.count() - 1);
        sPkgName = getPkgNameByPid(sPid.toInt());
    }

    // 联网/远程 开关关闭，直接放行远程访问请求
    if (!m_isbRemControlSwitch || sPkgName.isEmpty()) {
        m_resourceManagerInter->AllowRequest(m_uuid, msg, true);
        return;
    }

    // 如果包名为空，放行
    if (m_mapRemControlAppName[sPkgName].isEmpty()) {
        m_resourceManagerInter->AllowRequest(m_uuid, msg, true);
        return;
    }

    // 如果是系统应用，放行
    if (m_remSystemApps.contains(sPkgName)) {
        m_resourceManagerInter->AllowRequest(m_uuid, msg, true);
        return;
    }

    if (NET_STATUS_ITEMALLOW == m_nRemOption) {
        m_disableDialog.removeOne(sPkgName);
        m_askDisableDialog.removeOne(sPkgName);
        m_oneAllowDialog.removeOne(sPkgName);
        m_resourceManagerInter->AllowRequest(m_uuid, msg, true);
    } else if (NET_STATUS_ITEMDISALLOW == m_nRemOption) {
        m_askDisableDialog.removeOne(sPkgName);
        m_oneAllowDialog.removeOne(sPkgName);
        m_resourceManagerInter->AllowRequest(m_uuid, msg, false);
        if (m_disableDialog.contains(sPkgName) && m_appsStartList.contains(sPkgName)) {
        } else {
            m_disableDialog.append(sPkgName);
            // 禁止弹框
            m_dataInterFace->showRemControlDialog(sPkgName, m_mapRemControlAppName.value(sPkgName));
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
            m_dataInterFace->showRemControlAskDialog(sPkgName, m_mapRemControlAppName.value(sPkgName), sPid);
        }
    }
}

// 远程访问列表 远程访问状态改变
void SocketListenManager::setRemRequestReply(QString sPkgName, QString sId, int nType)
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
            setRemControlDefaultStatus(NET_STATUS_ITEMALLOW);
        }
    }
}

// 通过pid获取进程的包名
QString SocketListenManager::getPkgNameByPid(int pid)
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
QString SocketListenManager::getProcExecPath(pid_t pid) const
{
    // 待返回的进程名
    QString retStr;

    // 获取执行路径
    QString sExeLinkPath = "/proc/" + QString::number(pid) + "/exe";
    retStr = getListenPackageFilePathByLink(sExeLinkPath.toStdString().c_str());
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

QString SocketListenManager::getProcEnvProperties(pid_t pid, QString sEvnKey)
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

QString SocketListenManager::getPkgNameByDesktopPath(QString sDesktopPath)
{
    // 初始包名为空
    QString sPkgName;

    QString sRealDesktopPath = getListenPackageFilePathByLink(sDesktopPath.toStdString().c_str());
    if (sRealDesktopPath.isEmpty()) {
        sRealDesktopPath = sDesktopPath;
    }

    // 获取包名的第一种方式 直接通过执行路径从数据库获取
    sPkgName = m_monitornetflow->QueryPackageName(sRealDesktopPath);
    return sPkgName;
}

QString SocketListenManager::getPPid(QString sPid)
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

/**************************远程访问数据处理**********************************/
// 刷新远程访问状态
void SocketListenManager::refreshRemControlStatus()
{
    QDBusPendingReply<LauncherItemInfoList> reply = m_launcherInter->GetAllItemInfos();
    reply.waitForFinished();
    if (reply.isError()) {
        qDebug() << reply.error();
    }

    m_remSystemApps.clear();
    for (LauncherItemInfo launcherItemInfo : reply.value()) {
        QString sDesktopPath = launcherItemInfo.Path;
        QString sPkgName = getPkgNameByDesktopPath(sDesktopPath);
        if (sPkgName.isEmpty()) {
            continue;
        }
        m_mapRemControlAppName[sPkgName] = launcherItemInfo.Name;

        // 获取执行路径
        QSettings readIniSettingMethod(launcherItemInfo.Path, QSettings::Format::IniFormat);
        readIniSettingMethod.beginGroup("Desktop Entry");
        QString sExecPath = readIniSettingMethod.value("Exec").toString();
        readIniSettingMethod.endGroup();

        // 判断是否是系统应用
        if (!sExecPath.contains("/opt/")) {
            m_remSystemApps.append(sPkgName);
        }
    }
}

// 设置/获取远程访问默认状态
void SocketListenManager::setRemControlDefaultStatus(int nDefaultStatus)
{
    // 设置联网管控默认状态
    m_gsRemControl->set(REM_CONTROL_DEFAULT_STATUS, QString::number(nDefaultStatus));
}

// 设置/获取远程访问默认状态
int SocketListenManager::getRemControlDefaultStatus()
{
    return m_gsRemControl->get(REM_CONTROL_DEFAULT_STATUS).toInt();
}

// 应用程序开启关闭槽
void SocketListenManager::doAppStartSlot(QStringList sAppList)
{
    m_appsStartList.clear();
    for (int i = 0; i < sAppList.count(); ++i) {
        m_appsStartList.append(sAppList.at(i));
    }
}

// 应用程序开启关闭槽
void SocketListenManager::doAppEndSlot(QStringList sAppList)
{
    for (int i = 0; i < sAppList.count(); ++i) {
        QString sPkgName = sAppList.at(i);
        m_disableDialog.removeOne(sPkgName);
        m_askDisableDialog.removeOne(sPkgName);
        m_oneAllowDialog.removeOne(sPkgName);
        m_appsStartList.removeOne(sPkgName);
    }
}
