// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "getprocnetflow.h"

#include <QMutex>
#include <QThread>
#include <QTimer>

#include <proc/readproc.h>
#include <iostream>
#include <unistd.h>
#include <dirent.h>

GetProcNetFlow::GetProcNetFlow()
    : m_isInited(false)
    , m_networkInter(nullptr)
    , m_dispatchNetPkgJob(nullptr)
    , m_dispatchNetPkgJobThread(nullptr)
    , m_stopped(false)
{
    m_networkInter = new QDBusInterface("org.freedesktop.NetworkManager",
                                        "/org/freedesktop/NetworkManager",
                                        "org.freedesktop.NetworkManager",
                                        QDBusConnection::systemBus(), this);
}

GetProcNetFlow::~GetProcNetFlow()
{
    // 确保已初始化
    if (!m_isInited) {
        init();
    }

    m_dispatchNetPkgJob->stop();
    m_dispatchNetPkgJobThread->quit();
    m_dispatchNetPkgJobThread->wait();
    m_dispatchNetPkgJobThread->deleteLater();
    m_dispatchNetPkgJob->deleteLater();
}

void GetProcNetFlow::init()
{
    // 确保未初始化
    if (m_isInited) {
        return;
    }

    m_dispatchNetPkgJobThread = new QThread(this);
    m_dispatchNetPkgJob = new DispatchNetPkgJob;
    m_dispatchNetPkgJob->moveToThread(m_dispatchNetPkgJobThread);

    // connect
    connect(m_dispatchNetPkgJobThread, &QThread::started, m_dispatchNetPkgJob, &DispatchNetPkgJob::DispatchPackets);
    connect(m_dispatchNetPkgJob, &DispatchNetPkgJob::notifyAppendNetPkgInfo, this, &GetProcNetFlow::appendNetPkgInfo);
    m_dispatchNetPkgJobThread->start();

    // 更新所有进程网络信息
    QTimer *procInfosUpdateTimer = new QTimer(this);
    procInfosUpdateTimer->setInterval(NET_INFOS_UPDATE_INTERVAL);
    connect(procInfosUpdateTimer, &QTimer::timeout, this, &GetProcNetFlow::updateProNetInfos);
    procInfosUpdateTimer->start();

    // 计算各进程网络流量定时器
    QTimer *calcNetFlowTimer = new QTimer(this);
    calcNetFlowTimer->setInterval(PROC_FLOW_CALC_INTERVAL);
    // 连接到 计算单位时间内的网络流量
    connect(calcNetFlowTimer, &QTimer::timeout, this, &GetProcNetFlow::calcProcsPeriodNetFlow);
    calcNetFlowTimer->start();

    // 定时检查网络设备是否更改
    QTimer *netDevCheckTimer = new QTimer(this);
    netDevCheckTimer->setInterval(1000 * NET_DEV_CHECK_INTERVAL);
    connect(netDevCheckTimer, &QTimer::timeout, this, &GetProcNetFlow::checkNetDev);
    netDevCheckTimer->start();

    // 初始化完成
    m_isInited = true;

    // 获取当前网络设备
    checkNetDev();

    updateProNetInfos();
    calcProcsPeriodNetFlow();
}

// 使能流量监控器
void GetProcNetFlow::enableNetFlowMonitor(bool enable)
{
    // 确保已初始化
    if (!m_isInited) {
        init();
    }

    if (enable) {
        m_dispatchNetPkgJob->start();
        m_dispatchNetPkgJobThread->start();

        m_stopped = false;
    } else {
        m_dispatchNetPkgJob->stop();
        m_dispatchNetPkgJobThread->quit();
        m_dispatchNetPkgJobThread->wait();

        m_stopped = true;
    }
}

void GetProcNetFlow::updateProNetInfos()
{
    // 确保已初始化
    if (!m_isInited) {
        init();
    }

    initProcInfos();

    // 如果停止
    if (m_stopped) {
        // proc info list for sending
        DefenderProcInfoList procInfos;
        // 转换成列表
        changeProcInfosMapToLst(procInfos);
        Q_EMIT sendPocNetFlowInfos(procInfos);
        return;
    }

    updateMapOfInodeToPid();

    updateNetConnInfos();
}

// 计算单位时间内的网络流量
void GetProcNetFlow::calcProcsPeriodNetFlow()
{
    // 确保已初始化
    if (!m_isInited) {
        init();
    }

    // 如果停止
    if (m_stopped) {
        return;
    }

    // proc info list for sending
    DefenderProcInfoList procInfos;

    // 计算此段时间内各进程流量信息
    // 时间差，只当缓冲数据量大于1时，才计算单位时间内进程流量数据
    if (2 > m_netPkgInfoCache.size()) {
        // 转换成列表
        changeProcInfosMapToLst(procInfos);
        Q_EMIT sendPocNetFlowInfos(procInfos);
        return;
    }
    long startTimeStampS = m_netPkgInfoCache.first().timeStampS;
    long startTimeStampUS = m_netPkgInfoCache.first().timeStampUS;
    long endTimeStampS = m_netPkgInfoCache.last().timeStampS;
    long endTimeStampUS = m_netPkgInfoCache.last().timeStampUS;

    double deltaTimeS = endTimeStampS - startTimeStampS + (endTimeStampUS - startTimeStampUS) / 1000000.0;

    // 时间戳改变，清空缓存
    if (0 > deltaTimeS) {
        m_netPkgInfoCache.clear();
    }

    // 只当缓冲数据时间差大于0.1s时，才计算单位时间内进程流量数据
    if (0.1 > deltaTimeS) {
        // 转换成列表
        changeProcInfosMapToLst(procInfos);
        Q_EMIT sendPocNetFlowInfos(procInfos);
        return;
    }

    QMap<int, DefenderProcInfo> procNetFlowInfosTmp;
    while (!m_netPkgInfoCache.isEmpty()) {
        NetPkgInfo netPkgInfo = m_netPkgInfoCache.head();
        m_netPkgInfoCache.pop_front();

        DefenderProcInfo procInfo {};
        getProcNetFlowInfo(procInfo, netPkgInfo);

        // find the same proc info
        if (procNetFlowInfosTmp.keys().contains(procInfo.nPid)) {
            DefenderProcInfo procInfoTmp = procNetFlowInfosTmp[procInfo.nPid];
            procInfoTmp.dDownloads += procInfo.dDownloads;
            procInfoTmp.dUploads += procInfo.dUploads;
            procNetFlowInfosTmp[procInfo.nPid] = procInfoTmp;
        } else {
            procNetFlowInfosTmp[procInfo.nPid] = procInfo;
        }
    }

    // 遍历进程流量信息表，计算各进程网络速度
    QMap<int, DefenderProcInfo>::iterator iter = procNetFlowInfosTmp.begin();
    for (; iter != procNetFlowInfosTmp.end(); ++iter) {
        int pid = iter.key();
        DefenderProcInfo info = iter.value();
        info.dUploadSpeed = info.dUploads / deltaTimeS;
        info.dDownloadSpeed = info.dDownloads / deltaTimeS;
        procNetFlowInfosTmp[pid] = info;

        m_mapOfPidToProcInfo[pid] = info;
    }

    // 转换成列表
    changeProcInfosMapToLst(procInfos);

    Q_EMIT sendPocNetFlowInfos(procInfos);
}

// 检查网络设备
void GetProcNetFlow::checkNetDev()
{
    // 确保已初始化
    if (!m_isInited) {
        init();
    }

    // 获取当前主要使用的网络设备接口名称
    QString devStr = getPrimaryNetDev();
    if (devStr != m_currentNetDev) {
        // 启用的网络设备改变
        qDebug() << "openning net dev changed" << m_currentNetDev << "->" << devStr;
        m_currentNetDev = devStr;
        // 让线程更新网络设备
        m_dispatchNetPkgJob->setCurrentNetDev(m_currentNetDev);
        m_dispatchNetPkgJob->setUpdateNetDevFlag(true);
    }
}

// 获取网络连接信息
bool GetProcNetFlow::getNetConnInfos(uint etherType, uint protocol,
                                     QList<NetConnInfo> &netConnInfoList)
{
    bool ok {true};
    FILE *fp {};
    const size_t BLEN = 4096;
    QByteArray buffer {BLEN, 0};
    int nr {};

    char localPort[16] {};
    char remPort[16] {};
    unsigned long inode {};

    QString localAddrStr {}, localPortStr {};
    QString remAddrStr {}, remPortStr {};

    // netConnInfoPath
    const char *netConnInfoPath {};
    if (ETHERTYPE_IP == etherType) {
        if (IPPROTO_TCP == protocol) {
            netConnInfoPath = PROC_PATH_SOCK_TCP;
        } else if (IPPROTO_UDP == protocol) {
            netConnInfoPath = PROC_PATH_SOCK_UDP;
        }
    } else if (ETHERTYPE_IPV6 == etherType) {
        if (IPPROTO_TCP == protocol) {
            netConnInfoPath = PROC_PATH_SOCK_TCP6;
        } else if (IPPROTO_UDP == protocol) {
            netConnInfoPath = PROC_PATH_SOCK_UDP6;
        }
    }

    if (!(fp = fopen(netConnInfoPath, "r"))) {
        qDebug() << QString("open %1 failed").arg(netConnInfoPath);
        return !ok;
    }

    while (fgets(buffer.data(), BLEN, fp)) {
        nr = sscanf(buffer.data(), "%*s %*[^:]:%s %*[^:]:%s %*x %*s %*s %*s %*u %*u %ld",
                    localPort,
                    remPort,
                    &inode);

        // ignore first line
        if (nr == 0)
            continue;

        // socket still in waiting state
        if (inode == 0) {
            continue;
        }

        NetConnInfo connInfo {};
        connInfo.etherType = etherType;
        connInfo.protocol = protocol;

        localPortStr = QString(localPort);
        remPortStr = QString(remPort);
        getNetPort(localPortStr, connInfo.localPort);
        getNetPort(remPortStr, connInfo.remPort);

        connInfo.inode = inode;

        // getPidByInode() ............
        connInfo.pid = m_mapOfInodeToPid[connInfo.inode];

        netConnInfoList.append(connInfo);
    }

    if (ferror(fp)) {
        ok = !ok;
        qDebug() << QString("read %1 failed").arg(netConnInfoPath);
    }
    fclose(fp);

    return ok;
}

// 更新网络连接信息
void GetProcNetFlow::updateNetConnInfos()
{
    m_netConnInfoList.clear();
    getNetConnInfos(ETHERTYPE_IP, IPPROTO_TCP, m_netConnInfoList);
    getNetConnInfos(ETHERTYPE_IP, IPPROTO_UDP, m_netConnInfoList);
    getNetConnInfos(ETHERTYPE_IPV6, IPPROTO_TCP, m_netConnInfoList);
    getNetConnInfos(ETHERTYPE_IPV6, IPPROTO_UDP, m_netConnInfoList);
}

// 初始化进程流量信息map
void GetProcNetFlow::initProcInfos()
{
    m_mapOfPidToProcInfo.clear();
    DIR *procDir = opendir("/proc");

    if (procDir == nullptr) {
        std::cerr << "Error reading /proc, needed to get inode-to-pid-maping\n";
        exit(1);
    }

    dirent *entry;

    while ((entry = readdir(procDir))) {
        if (entry->d_type != DT_DIR)
            continue;

        if (!is_number_t(entry->d_name))
            continue;

        // 创建各进程流量信息初始数据
        DefenderProcInfo info {};
        info.nPid = atoi(entry->d_name);
        m_mapOfPidToProcInfo[info.nPid] = info;
    }
    closedir(procDir);
}

// 更新socket节点到pid的map
void GetProcNetFlow::updateMapOfInodeToPid()
{
    // 遍历所有进程键值表
    QMap<int, DefenderProcInfo>::iterator iter = m_mapOfPidToProcInfo.begin();

    for (; iter != m_mapOfPidToProcInfo.end(); ++iter) {
        int pid = iter.key();
        QList<unsigned long> socketInodes;
        readProcSocketInodes(pid, socketInodes);

        for (unsigned long inode : socketInodes) {
            m_mapOfInodeToPid[inode] = pid;
        }
    }
}

// 获取网络包对应的进程信息
void GetProcNetFlow::getProcNetFlowInfo(DefenderProcInfo &procInfo, const NetPkgInfo &pkgInfo)
{
    for (const NetConnInfo &connInfo : m_netConnInfoList) {
        // uploaded package
        if (pkgInfo.srcPort == connInfo.localPort) {
            procInfo.nPid = connInfo.pid;
            procInfo.dUploads = pkgInfo.packetSize;
            break;
        }
        // downloaded package
        else if (pkgInfo.dstPort == connInfo.localPort) {
            procInfo.nPid = connInfo.pid;
            procInfo.dDownloads = pkgInfo.packetSize;
            break;
        }
    }

    if (0 == procInfo.nPid) {
        procInfo.dDownloads = pkgInfo.packetSize;
    }
}

// 将进程信息map转成list
void GetProcNetFlow::changeProcInfosMapToLst(DefenderProcInfoList &procInfos)
{
    procInfos.clear();
    QMap<int, DefenderProcInfo>::iterator iter;
    iter = m_mapOfPidToProcInfo.begin();
    for (; iter != m_mapOfPidToProcInfo.end(); ++iter) {
        DefenderProcInfo info = iter.value();
        procInfos.append(info);
    }
}

QString GetProcNetFlow::getPrimaryNetDev()
{
    QString retDevName;
    // 获取当前主要使用的网络连接dbus
    if (!m_networkInter->isValid()) {
        return retDevName;
    }
    QVariant connectionDbusPathVariant = m_networkInter->property("PrimaryConnection");
    QDBusObjectPath connectionDbusPath = qvariant_cast<QDBusObjectPath>(connectionDbusPathVariant);

    // 检查dbus服务路径是否规范
    if (!checkDbusServicePath(connectionDbusPath.path())) {
        return retDevName;
    }

    QDBusInterface connectionInter("org.freedesktop.NetworkManager",
                                   connectionDbusPath.path(),
                                   "org.freedesktop.NetworkManager.Connection.Active",
                                   QDBusConnection::systemBus(), this);
    if (!connectionInter.isValid()) {
        return retDevName;
    }
    // 获取当前主要使用的网络设备dbus
    QVariant devDbusPathListVariant = connectionInter.property("Devices");
    QList<QDBusObjectPath> devDbusPathList = devDbusPathListVariant.value<QList<QDBusObjectPath>>();

    QDBusObjectPath devDbusPath;
    if (0 < devDbusPathList.size()) {
        devDbusPath = devDbusPathList.at(0);
    } else {
        return retDevName;
    }

    // 获取当前主要使用的网络设备接口名称
    // 检查dbus服务路径是否规范
    if (!checkDbusServicePath(devDbusPath.path())) {
        return retDevName;
    }

    QDBusInterface devInter("org.freedesktop.NetworkManager",
                            devDbusPath.path(),
                            "org.freedesktop.NetworkManager.Device",
                            QDBusConnection::systemBus(), this);
    if (!devInter.isValid()) {
        return retDevName;
    }

    return devInter.property("Interface").toString();
}

// 通过pid读取进程所有的socket节点
// ino_t <=> unsigned long
bool GetProcNetFlow::readProcSocketInodes(int pid, QList<unsigned long> &socketInodes)
{
    bool b = false;
    DIR *dir;
    struct dirent *dp;
    char path[128], fdp[256 + 32];
    struct stat sbuf;

    sprintf(path, PROC_FD_PATH, pid);

    errno = 0;
    if (!(dir = opendir(path))) {
        return b;
    }

    while ((dp = readdir(dir))) {
        if (isdigit(dp->d_name[0])) {
            sprintf(fdp, PROC_FD_NAME_PATH, pid, dp->d_name);
            memset(&sbuf, 0, sizeof(struct stat));
            const int linklen = 80;
            char linkname[linklen];
            ssize_t usedlen = readlink(fdp, linkname, linklen - 1);
            if (usedlen == -1) {
                continue;
            }

            assert(usedlen < linklen);
            linkname[usedlen] = '\0';

            if (!stat(fdp, &sbuf)) {
                if (S_ISSOCK(sbuf.st_mode)) {
                    socketInodes << sbuf.st_ino;
                }
            }
        }
    }
    if (errno) {
        // qDebug() << QString("read %1 failed").arg(path);
    } else {
        b = true;
    }

    closedir(dir);
    return b;
}
