// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "monitornetflow_adaptor.h"
#include "../../deepin-defender/src/window/modules/common/gsettingkey.h"
#include "../../deepin-defender/src/window/modules/common/common.h"
#include "disk/disk.h"
#include "usbconnection/usbmanagemonitor.h"

#include <QObject>
#include <QThread>
#include <QSqlDatabase>

class NetManagerModel;
class SecurityLogSql;

#define HEART_BEAT_TIME_OUT 3 // 心跳间隔,s
// 检测ssh登陆端口启动标志
#define REM_SSH_STATUS "running"
const QString service = "com.deepin.defender.MonitorNetFlow";
const QString path = "/com/deepin/defender/MonitorNetFlow";

class SecurityPkgNameSql;
class WriteDBusData : public QObject
{
    Q_OBJECT
public:
    explicit WriteDBusData(QObject *parent = nullptr);
    ~WriteDBusData();

Q_SIGNALS:
    // **** caitao 2021.1.11 废弃 *****
    Q_DECL_DEPRECATED void notifyDBusKbData(QString, double, double, int, QString, QString);
    Q_DECL_DEPRECATED void notifyGlobalData(double, double);
    Q_DECL_DEPRECATED void notifyWorking();
    // *******************************

    // usb设备接入的信号，入参bflag 为false,表示限制状态
    void NotifyUsbConnectionInfo(bool bFlag, QString sUsbInfo);
    void SendUsbConnectionInfo(QString sUsbInfos);
    // 通知显示限制所有usb设备的系统通知
    void requestShowBlockAllSysNotify();
    // 通知显示限制非白名单usb设备的系统通知
    void requestShowOnlyWhiteListSysNotify();
    // 请求初始化进程流量信息获取线程
    void requestInitGetProcNetFlow();
    // 发送进程网络信息列表
    void SendPocNetFlowInfos(DefenderProcInfoList list);
    // ssh远程登陆端口状态改变信号
    void SSHStatusChanged(bool status);
    // 发送ssh远程登陆端口状态
    void SendSSHStatus(bool status);

    // 应用程序安装卸载刷新数据库信号
    void requestRefreshPackageTable();
    // 包名解析数据库插入完成信号
    void NotifySqlInsertFinish();

    // 发送进程信息列表
    void SendProcInfoList(const DefenderProcInfoList &list);
    // 发送某个应用流量列表
    void SendAppFlowList(const QString &sPkgName, const int timeRangeType, const DefenderProcInfoList &list);
    // 发送所有应用流量列表
    void SendAllAppFlowList(const int timeRangeType, const DefenderProcInfoList &list);

public Q_SLOTS:
    void StartApp();
    void ExitApp();
    void onNotifyHeartbeat();

    // 设置/获取ssh远程登陆端口状态
    void setRemRegisterStatus(bool bStatus);
    bool getRemRegisterStatus();
    // 异步设置/获取ssh远程登陆端口状态
    void AsyncSetRemRegisterStatus(bool bStatus);
    void AsyncGetRemRegisterStatus();
    // 当接收到ssh状态
    void recSSHStatus(bool status);

    // 开启/关闭进程流量监控功能
    void EnableNetFlowMonitor(bool enable);

    // 获取磁盘设备路径列表
    QStringList GetDiskDevicePathList();

    // 清理选中的垃圾文件
    void CleanSelectFile(QStringList filePaths);

    // 清理journal目录下的日志文件
    void CleanJournal();

    // 添加安全日志
    void AddSecurityLog(int nType, QString sInfo);
    // 删除过滤条件下的安全日志
    bool DeleteSecurityLog(int nLastDate, int nType);

    // ****** usb管控 *******
    // 将待发送设备信息整合成字符串
    inline QString MergeDevSavingInfoToStr(QString sName, QString sStatus, QString sSerial, QString sVendorID,
                                           QString sProductID, QString sParentSize, QString sSize)
    {
        return m_usbManageMonitor->mergeDevSavingInfoToStr(sName, sStatus, sSerial, sVendorID,
                                                           sProductID, sParentSize, sSize);
    }
    inline void WriteConnectionRecord(QString sName, QString sDatetime, QString sStatus,
                                      QString sSerial, QString sVendorID, QString sProductID,
                                      QString sParentSize, QString sSize)
    {
        m_usbManageMonitor->writeConnectionRecord(sName, sDatetime, sStatus, sSerial,
                                                  sVendorID, sProductID, sParentSize, sSize);
    }

    inline void ClearUsbConnectionLog()
    {
        m_usbManageMonitor->clearUsbConnectionLog();
    }

    inline void WriteConnectionWhiteLst(QString sName, QString sDatetime, QString sStatus,
                                        QString sSerial, QString sVendorID, QString sProductID,
                                        QString sParentSize, QString sSize)
    {
        m_usbManageMonitor->writeConnectionWhiteLst(sName, sDatetime, sStatus, sSerial,
                                                    sVendorID, sProductID, sParentSize, sSize);
    }

    inline void RemoveFromWhiteLst(QString sName, QString sSerial, QString sVendorID, QString sProductID)
    {
        // 暂不使用设备名称
        Q_UNUSED(sName);
        m_usbManageMonitor->removeFromWhiteLst(sSerial, sVendorID, sProductID);
    }

    inline void ChangeIsbSaveRecord(bool change) { m_usbManageMonitor->changeIsbSaveRecord(change); }
    inline void SelectLimitationModel(QString modelStr) { m_usbManageMonitor->selectLimitationModel(modelStr); }
    inline bool GetRecordSaveSwitch() { return m_usbManageMonitor->getRecordSaveSwitch(); }
    inline QString GetLimitModel() { return m_usbManageMonitor->getLimitModel(); }
    inline QString GetUsbConnectionRecords() { return m_usbManageMonitor->getUsbConnectionRecords(); }
    inline QString GetWhiteList() { return m_usbManageMonitor->getWhiteList(); }

    // 递归函数-获取/usr/share/deepin-defender/config/目录下所有的lock文件
    void scanFile(const QString &path);

    // 初始化localcache.db数据库
    void initLocalCache();
    // 查询相关联文件对应的包名信息
    QString QueryPackageName(const QString sBinaryFile);
    // 应用程序安装卸载刷新数据库
    void RefreshPackageTable();
    // 接受数据库插入完成信号
    void acceptSqlInsertFinish();
    /// 流量统计数据查询
    // 请求获取某个应用流量列表
    void AsyncGetAppFlowList(const QString &sPkgName, const int timeRangeType);
    // 请求获取所有应用流量列表
    void AsyncGetAllAppFlowList(const int timeRangeType);

private:
    MonitorNetFlowAdaptor *m_adaptor;

    QString m_device;

    UsbManageMonitor *m_usbManageMonitor;
    QThread *m_usbManageMonitorThread;

    QSqlDatabase m_localCacheDb;
    SecurityPkgNameSql *m_securityPkgNameSql;
    QThread *m_securityPkgNameSqlThread;
    QMap<QString, QString> m_sPkgNameMap;

    NetManagerModel *m_netManagerModel;

    SecurityLogSql *m_securityLogSql;
    // 是否正在获取ssh状态
    bool m_inGettingSSHStatus;
    bool m_isInitSqlDatabase;
};
