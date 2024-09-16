// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "netutils.h"
#include "dispatchnetpkgjob.h"
#include "defenderprocinfolist.h"

#include <QObject>

#include <pcap.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <sys/stat.h>

#define NET_DEV_CHECK_INTERVAL 2 // 网络设备变动检查间隔,s
#define NET_INFOS_UPDATE_INTERVAL 1500 // 网络信息更新间隔,ms
#define PROC_FLOW_CALC_INTERVAL 2000 // 单位时间内进程流量信息计算间隔,ms

// getNetConnInfos
#define PROC_PATH_SOCK_TCP "/proc/net/tcp"
#define PROC_PATH_SOCK_TCP6 "/proc/net/tcp6"
#define PROC_PATH_SOCK_UDP "/proc/net/udp"
#define PROC_PATH_SOCK_UDP6 "/proc/net/udp6"
// updateNetConnInfos
#define PROC_FD_PATH "/proc/%u/fd"
#define PROC_FD_NAME_PATH "/proc/%u/fd/%s"

// 网络连接信息结构体
struct NetConnInfo {
    uint etherType {};
    // 协议
    uint protocol {};
    // 本地ipv4地址
    in_addr_t localIpv4Addr {};
    // 本地ipv6地址
    uint8_t localIpv6Addr[16] {};
    // 本地端口号
    uint16_t localPort {};
    // 远程ipv4地址
    in_addr_t remIpv4Addr {};
    // 远程ipv6地址
    uint8_t remIpv6Addr[16] {};
    // 远程端口号
    uint16_t remPort {};
    // 节点
    unsigned long inode {};
    // 进程号
    int pid;
    // 初始化
    NetConnInfo()
    {
        etherType = 0;
        protocol = 0;
        localIpv4Addr = 0;
        memset(localIpv6Addr, 0, sizeof(localIpv6Addr));
        localPort = 0;
        remIpv4Addr = 0;
        memset(remIpv6Addr, 0, sizeof(remIpv6Addr));
        remPort = 0;
        inode = 0;
        pid = 0;
    }
};

class QMutex;
class QThread;

class GetProcNetFlow : public QObject
{
    Q_OBJECT

public:
    GetProcNetFlow();
    ~GetProcNetFlow();

    void init();
    // 添加网络包信息到缓冲队列
    inline void appendNetPkgInfo(const NetPkgInfo info)
    {
        m_netPkgInfoCache.push_back(info);
    }

Q_SIGNALS:
    // 发送进程流量信息列表
    void sendPocNetFlowInfos(const DefenderProcInfoList &list);

public Q_SLOTS:
    // 使能流量监控器
    void enableNetFlowMonitor(bool enable);
    // 更新所有进程网络信息
    void updateProNetInfos();
    // 计算单位时间内的网络流量
    void calcProcsPeriodNetFlow();
    // 检查网络设备
    void checkNetDev();

private:
    // 获取网络连接信息
    bool getNetConnInfos(uint etherType, uint protocol,
                         QList<NetConnInfo> &netConnInfoList);
    // 更新网络连接信息
    void updateNetConnInfos();
    // 通过pid读取进程所有的socket节点
    // ino_t <=> unsigned long
    bool readProcSocketInodes(int pid, QList<unsigned long> &socketInodes);
    // 初始化进程信息map
    void initProcInfos();
    // 更新socket节点到pid的map
    void updateMapOfInodeToPid();
    // 获取网络包对应的进程信息
    void getProcNetFlowInfo(DefenderProcInfo &procInfo, const NetPkgInfo &pkgInfo);
    // 将进程信息map转成list
    void changeProcInfosMapToLst(DefenderProcInfoList &list);
    // 获取当前主要使用的网络设备接口名称
    QString getPrimaryNetDev();

private:
    bool m_isInited; // 是否初始化
    // 系统网络设备管理dbus接口
    QDBusInterface *m_networkInter;
    // 拆解网络数据包对象
    DispatchNetPkgJob *m_dispatchNetPkgJob;
    QThread *m_dispatchNetPkgJobThread;
    QString m_currentNetDev;
    // 是否停止计算网络流量
    bool m_stopped;
    // 网络连接信息列表
    // 从 /proc/net/tcp* or /proc/net/udp* 文件中获取
    QList<NetConnInfo> m_netConnInfoList;
    // 节点到pid的映射表
    QMap<unsigned long, int> m_mapOfInodeToPid;
    // 网络数据包信息缓冲队列
    QQueue<NetPkgInfo> m_netPkgInfoCache;
    // pid到进程信息的映射表
    QMap<int, DefenderProcInfo> m_mapOfPidToProcInfo;
};
