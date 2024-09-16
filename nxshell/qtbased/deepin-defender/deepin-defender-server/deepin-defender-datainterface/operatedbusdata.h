// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "datainterface_adaptor.h"
#include "monitornetflow_interface.h"
// 登录安全
#include "window/modules/loginsafety/loginsafetysrvmodel.h"

#include <com_deepin_dde_daemon_launcherd.h>
#include <com_deepin_lastore_jobmanager.h>

#include <DGuiApplicationHelper>

#include <QObject>

class SocketCreateManager;
class SocketListenManager;
class NetDisableDialog;
class NetAskDialog;
class RemDisableDialog;
class RemAskDialog;
class DBusStartManager;
class QGSettings;

using Launcherd = com::deepin::dde::daemon::Launcher;
using LastoreManager = com::deepin::lastore::Manager;
using NetFlowMonitorInter = com::deepin::defender::MonitorNetFlow;

// 安恒/瑞星 标识
const int Das_Security_Type = 0;
const int Rising_Type = 1;

DCORE_USE_NAMESPACE
class AppsStartToEnd;
class InvokerFactoryInterface;
class DBusInvokerInterface;
class OperateDBusData : public QObject
{
    Q_OBJECT

public:
    explicit OperateDBusData(QObject *parent = nullptr);

    ~OperateDBusData();

public Q_SLOTS:
    // 接口
    void StartApp();
    void ExitApp();
    void preInitialize();

    // 初始化
    void initialize();
    void initNetControlSocket();

    Q_DECL_DEPRECATED inline bool getFireWallSwitchStatus() { return false; }
    Q_DECL_DEPRECATED inline bool getRemRegisterStatus() { return false; }

    // 新联网管控/远程访问
    void setNetControlSwitchStatus(bool);
    void setRemControlSwitchStatus(bool);
    void setAppStartEndStatus();

    void setNetAppsChange(QList<DefenderProcInfo>);
    void getNetAppsInfo();
    void setNetAppStatusChange(QString, int);
    void setNetControlDefaultStatus(int);
    int getNetControlDefaultStatus();
    void setNetRequestReply(QString, QString, int);

    void setRemAppsChange(QList<DefenderProcInfo>);
    void getRemAppsInfo();
    void setRemAppStatusChange(QString, int);
    void setRemControlDefaultStatus(int);
    int getRemControlDefaultStatus();
    void setRemRequestReply(QString, QString, int);
    void setRemDefaultAllowStatus();

    // 联网管控禁止/询问弹框
    void showNetControlDialog(const QString &sPkgName, const QString &sAppName);
    void showNetControlAskDialog(const QString &sPkgName, const QString &sAppName, const QString &sId);

    // 远程访问禁止/询问弹框
    void showRemControlDialog(const QString &sPkgName, const QString &sAppName);
    void showRemControlAskDialog(const QString &sPkgName, const QString &sAppName, const QString &sId);

    // 开机自启动管理
    bool startLauncherManage();
    QString getAppsInfoEnable();
    QString getAppsInfoDisable();
    bool isAutostart(QString sPath);
    bool exeAutostart(int nStatus, QString sPath);
    void refreshData(bool bAdd, QString sID);

    //usb管控：显示限制所有设备系统通知
    void showBlockAllSysNotify();
    //usb管控：显示限制非白名单设备系统通知
    void showOnlyWhiteListSysNotify();

    // 垃圾清理 回收站/日志/缓存/痕迹
    QStringList GetTrashInfoList();
    QStringList GetLogInfoList();
    QStringList GetCacheInfoList();
    QStringList GetHistoryInfoList();
    QString GetBrowserCookiesInfoList();
    QString GetAppTrashInfoList();
    void DeleteSpecifiedFiles(QStringList);

    // 递归函数-获取目录下所有文件
    void scanFile(const QString &path);
    // 首页体检垃圾清理项大小
    double GetCleanerSum();

    // 首页体检清理所有垃圾
    void DeleteAllCleaner();

    // 请求开始垃圾文件扫描
    void RequestStartTrashScan();
    // 请求清理选中的垃圾文件
    void RequestCleanSelectTrash();

    // 获取密码限制策略开启/关闭 状态
    bool GetPwdLimitPolicyEnable() const;
    // 开启/关闭密码限制策略
    void SetPwdLimitPolicyEnable(const bool &enable);
    // 设置密码限制等级
    void SetPwdLimitedLevel(const int &level);
    // 获得密码安全错误信息
    QString GetPwdChangeError() const;

    // 获取uos-resource-manager此时是否为异常状态
    bool GetUosResourceStatus();

Q_SIGNALS:
    // 联网管控/远程访问
    void notifyStartAppCheckTimer();
    void notifyStopAppCheckTimer();
    void netChangeAsk();
    void remChangeAsk();

    // 开机自启动管理
    void AccessRefreshData(bool bAdd, QString sID);
    // 开启/关闭密码限制策略完成信号
    void SetPwdLimitPolicyEnableFinished(const bool &enable);
    // 设置密码限制等级完成信号
    void SetPwdLimitedLevelFinished(const int &level);

private:
    QGSettings *m_gsetting; // gsetting配置对象
    DatainterfaceAdaptor *m_adaptpr;

    // 联网管控/远程访问
    AppsStartToEnd *m_appsStartToEnd;
    QThread *m_appsStartToEndThread;
    SocketCreateManager *m_socketCreateManage;
    SocketListenManager *m_socketListenManage;
    bool m_isAppsStartEndStatus;

    // 联网管控、远程访问弹框
    NetDisableDialog *m_netDisableDialog;
    NetAskDialog *m_netAskDialog;
    RemDisableDialog *m_remDisableDialog;
    RemAskDialog *m_remAskDialog;

    // 开机自启动管理
    QMap<QString, QString> m_mapEnable; // 不允许自启动数据容器
    QMap<QString, QString> m_mapDisable; // 允许自启动数据容器
    QStringList m_enableData;
    QStringList m_disableData;
    QString m_enableAppsInfo;
    QString m_disableAppsInfo;
    Launcherd *m_launcherInter; // 取应用数据接口
    QDBusPendingReply<LauncherItemInfoList> m_reply; // 应用信息接口
    DBusStartManager *m_startManagerInterface; // 自启动接口

    // 系统服务
    NetFlowMonitorInter *m_monitorNetFlowInter;

    // 垃圾清理
    LastoreManager *m_managerInter; // 检查/var/cache/apt/archives下，可安全删除的deb包
    QStringList m_fileList;
    QStringList m_fileListSum;
    double m_dbCleanerSum;
    /// 首页垃圾清理
    // 首页垃圾清理扫描结果可清理文件路径列表
    QStringList m_homepageCleanerResultFileList;
    // 首页垃圾清理扫描结果可清理文件总大小，byte
    double m_homepageCleanerResultSize;

    // 登录安全
    LoginSafetySrvModel *m_loginSafetyModel;

    // 新版应用商店接口
    DBusInvokerInterface *m_appStoreClientInvokerInter;
    // 公共资源接口服务对象
    DBusInvokerInterface *m_resourceManagerInvokerInter;
};
