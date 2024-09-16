// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "homepagemodel.h"

#include "src/window/modules/common/gsettingkey.h"
#include "src/window/modules/common/invokers/invokerfactory.h"

#include <DWidget>

#include <QApplication>
#include <QDebug>
#include <QStorageInfo>
#include <QtConcurrent>

#define DIALOG_WIDTH 380  // dialog  宽度
#define DIALOG_HEIGHT 145 // dialog  高度

DWIDGET_USE_NAMESPACE

HomePageModel::HomePageModel(QObject *parent)
    : QObject(parent)
    , m_gSettingsInvokerInter(nullptr)
    , m_defenderDataInvokerInter(nullptr)
    , m_controlCenterInvokerInter(nullptr)
    , m_helperInvokerInter(nullptr)
    , m_diskManagerInvokerInter(nullptr)
    , m_netFlowMonitorInvokerInter(nullptr)
    , m_securityCenterInter(nullptr)
    , m_sysUpdaterModel(nullptr)
    , m_autoStartAppChecking(false)
    , m_sysVersionChecking(false)
    , m_trashChecking(false)
    , m_diskChecking(false)
    , m_developModeChecking(false)
{
    // 使用GSetting读取参数
    m_gSettingsInvokerInter =
        InvokerFactory::GetInstance().CreateSettings(DEEPIN_PC_MANAGER_GSETTING_PATH, "", this);
    m_defenderDataInvokerInter =
        InvokerFactory::GetInstance().CreateInvoker("com.deepin.pc.manager.session.daemon",
                                                    "/com/deepin/pc/manager/session/daemon",
                                                    "com.deepin.pc.manager.session.daemon",
                                                    ConnectType::SESSION,
                                                    this);
    // 控制中心dbus服务对象
    m_controlCenterInvokerInter =
        InvokerFactory::GetInstance().CreateInvoker("com.deepin.dde.ControlCenter",
                                                    "/com/deepin/dde/ControlCenter",
                                                    "com.deepin.dde.ControlCenter",
                                                    ConnectType::SESSION,
                                                    this);
    // 安全中心dbus服务对象
    m_defenderWindowInvokerInter =
        InvokerFactory::GetInstance().CreateInvoker("com.deepin.pc.manager",
                                                    "/com/deepin/pc/manager",
                                                    "com.deepin.pc.manager",
                                                    ConnectType::SESSION,
                                                    this);
    // 同步助手dbus服务，用于查询是否处于开发者模式
    m_helperInvokerInter = InvokerFactory::GetInstance().CreateInvoker("com.deepin.sync.Helper",
                                                                       "/com/deepin/sync/Helper",
                                                                       "com.deepin.sync.Helper",
                                                                       ConnectType::SYSTEM,
                                                                       this);
    // 磁盘管理dbus服务
    m_diskManagerInvokerInter =
        InvokerFactory::GetInstance().CreateInvoker("com.deepin.diskmanager",
                                                    "/com/deepin/diskmanager",
                                                    "com.deepin.diskmanager",
                                                    ConnectType::SYSTEM,
                                                    this);
    // 流量监控系统dbus服务
    m_netFlowMonitorInvokerInter =
        InvokerFactory::GetInstance().CreateInvoker("com.deepin.pc.manager.MonitorNetFlow",
                                                    "/com/deepin/pc/manager/MonitorNetFlow",
                                                    "com.deepin.pc.manager.MonitorNetFlow",
                                                    ConnectType::SYSTEM,
                                                    this);

    m_securityCenterInter =
        InvokerFactory::GetInstance().CreateInvoker("com.deepin.pc.manager.datainterface",
                                                    "/com/deepin/pc/manager/securitytooldialog",
                                                    "com.deepin.pc.manager.securitytooldialog",
                                                    ConnectType::SESSION,
                                                    this);

    // 系统更新数据处理对象
    m_sysUpdaterModel = new SysUpdaterModel(this);

    // 预启动磁盘管理器服务，避免磁盘检测时调用接口卡顿
    DBUS_NOBLOCK_INVOKE(m_diskManagerInvokerInter, "Start");

    //// 自启动
    // 更新自启动应用个数
    m_defenderDataInvokerInter->Connect("AccessRefreshData",
                                        this,
                                        SLOT(updateStartUpAppCount(bool, QString)));
    //// ssh
    // 当ssh状态改变时
    m_netFlowMonitorInvokerInter->Connect("SendSSHStatus", this, SIGNAL(sendSSHStatus(bool)));
    //// 系统版本
    // 通知去检查系统更新
    connect(this,
            &HomePageModel::NotifyCheckSysUpdate,
            m_sysUpdaterModel,
            &SysUpdaterModel::CheckForUpdates);
    // 通知去停止检查系统更新
    connect(this,
            &HomePageModel::notifyStopCheckSysUpdate,
            m_sysUpdaterModel,
            &SysUpdaterModel::stopCheckingSysVer);
    connect(this,
            &HomePageModel::notifyTimeroutSysUpdate,
            m_sysUpdaterModel,
            &SysUpdaterModel::onCheckJobStatusChanged);
    // 检查系统更新完成
    connect(m_sysUpdaterModel,
            &SysUpdaterModel::SendHaveUpdates,
            this,
            &HomePageModel::CheckSysUpdateFinished);
    /// 垃圾清理
    // 请求开始垃圾文件扫描
    connect(this, &HomePageModel::requestStartTrashScan, this, [this] {
        DBUS_NOBLOCK_INVOKE(m_defenderDataInvokerInter, "RequestStartTrashScan");
    });

    // 垃圾文件扫描完成时
    m_defenderDataInvokerInter->Connect("TrashScanFinished",
                                        this,
                                        SIGNAL(trashScanFinished(double)));
    // 请求清理选中的垃圾文件
    connect(this, &HomePageModel::requestCleanSelectTrash, this, [this] {
        DBUS_NOBLOCK_INVOKE(m_defenderDataInvokerInter, "RequestCleanSelectTrash");
    });
    // 清理选中的垃圾文件完成时
    m_defenderDataInvokerInter->Connect("CleanSelectTrashFinished",
                                        this,
                                        SIGNAL(cleanSelectTrashFinished()));
}

// 设置上次体检分数
void HomePageModel::setSafetyScore(const int &score)
{
    qInfo() << "[HomePageModel] [setSafetyScore] homecheck score set " << score;
    m_gSettingsInvokerInter->SetValue(SAFETY_SCORE, score);
}

// 获取上次体检分数
int HomePageModel::GetSafetyScore()
{
    int score = m_gSettingsInvokerInter->GetValue(SAFETY_SCORE).toInt();
    qInfo() << "[HomePageModel] [GetSafetyScore] homecheck score get " << score;
    return score;
}

// 设置上次体检时间
void HomePageModel::SetLastCheckTime(QDateTime dateTime)
{
    qInfo() << "[HomePageModel] [SetLastCheckTime] homecheck time set "
            << dateTime.toString("yyyy-MM-dd-hh-mm-ss");
    m_gSettingsInvokerInter->SetValue(LAST_CHECK_TIME, dateTime.toString("yyyy-MM-dd-hh-mm-ss"));
}

// 获取上次体检时间
QDateTime HomePageModel::GetLastCheckTime()
{
    QString timeStr = m_gSettingsInvokerInter->GetValue(LAST_CHECK_TIME).toString();
    qInfo() << "[HomePageModel] [GetLastCheckTime] homecheck time get " << timeStr;
    return QDateTime::fromString(timeStr, "yyyy-MM-dd-hh-mm-ss");
}

void HomePageModel::openStartupControlPage()
{
    DBUS_NOBLOCK_INVOKE(m_securityCenterInter, "ShowStartup");
    addSecurityLog(tr("Startup programs checked"));
}

// 打开系统更新界面
void HomePageModel::openSysUpdatePage()
{
    DBUS_NOBLOCK_INVOKE(m_controlCenterInvokerInter, "Show");
    DBUS_NOBLOCK_INVOKE(m_controlCenterInvokerInter, "ShowPage", "update", "");
    addSecurityLog(tr("System updated"));
}

// 磁盘管理器 是否已安装
bool HomePageModel::isDiskManagerInterValid()
{
    QProcess proc;
    // 查询是否已经安装 磁盘管理器
    proc.start("dpkg", { "-s", "deepin-diskmanager" });
    proc.waitForFinished();
    QString status = proc.readAllStandardOutput();
    qInfo() << "[HomePageModel] [isDiskManagerInterValid] disk status " << status;

    // 若返回结果以 "Package: deepin-diskmanager\nStatus: install"，则说明已安装
    return status.startsWith("Package: deepin-diskmanager\nStatus: install");
}

// 获取磁盘状态
bool HomePageModel::getDiskStatus()
{
    // 遍历所有磁盘设备路径
    QDBusMessage msg;
    msg = DBUS_BLOCK_INVOKE(m_netFlowMonitorInvokerInter, "GetDiskDevicePathList");
    GET_MESSAGE_VALUE(QStringList, devPaths, msg);

    for (QString devPath : devPaths) {
        msg = DBUS_BLOCK_INVOKE(m_diskManagerInvokerInter, "onGetDeviceHardStatus", devPath);
        GET_MESSAGE_VALUE(QString, data, msg);

        // 状态结果转小写
        QString status = data.toLower();
        // 若状态为异常，返回false
        if ("passed" != status) {
            qWarning() << "[HomePageModel] [getDiskStatus] no passed";
            return false;
        }
    }

    // 所有磁盘正常
    return true;
}

// 是否处于开发者模式
bool HomePageModel::IsDeveloperMode()
{
    QDBusMessage msg = DBUS_BLOCK_INVOKE(m_helperInvokerInter, "IsDeveloperMode");
    GET_MESSAGE_VALUE(bool, rst, msg);
    qInfo() << "[HomePageModel] [IsDeveloperMode] " << rst;

    return rst;
}

bool HomePageModel::ignoreDevModeChecking() const
{
    // 服务器版差异修改
    if (DSysInfo::UosType::UosServer == SystemType) {
        return true;
    }

    return m_gSettingsInvokerInter->GetValue(IGNORE_DEV_MODE_CHECKING).toBool();
}

void HomePageModel::setIgnoreDevModeChecking(bool ignore)
{
    m_gSettingsInvokerInter->SetValue(IGNORE_DEV_MODE_CHECKING, ignore);
}

// 更新自启动应用个数
void HomePageModel::updateStartUpAppCount(bool bAdd, const QString &sID)
{
    Q_UNUSED(bAdd);
    Q_UNUSED(sID);
    QtConcurrent::run([this] {
        // 判断自启动应用个数
        int count = getAutoStartAppCount();
        Q_EMIT autoStartAppCountChanged(count);
    });
}

void HomePageModel::onTrashScanFinished(double sum)
{
    Q_EMIT trashScanFinished(sum);
}

void HomePageModel::checkAutoStart()
{
    QtConcurrent::run([this] {
        int nEnableCount = getAutoStartAppCount();
        // 若当前正在检查自启动
        if (m_autoStartAppChecking) {
            Q_EMIT checkAutoStartFinished(nEnableCount);
        }
    });
}

bool HomePageModel::ignoreAutoStartChecking() const
{
    return m_gSettingsInvokerInter->GetValue(IGNORE_AUTO_START_CHECKING).toBool();
}

void HomePageModel::setIgnoreAutoStartChecking(bool ignore)
{
    m_gSettingsInvokerInter->SetValue(IGNORE_AUTO_START_CHECKING, ignore);
}

// 异步获取ssh状态
void HomePageModel::asyncGetSSHStatus() const
{
    DBUS_BLOCK_INVOKE(m_netFlowMonitorInvokerInter, "AsyncGetRemRegisterStatus");
}

void HomePageModel::asyncSetSSHStatus(bool status)
{
    DBUS_BLOCK_INVOKE(m_netFlowMonitorInvokerInter, "AsyncSetRemRegisterStatus", status);
}

// 获取允许自启动应用个数
int HomePageModel::getAutoStartAppCount() const
{
    int nEnableCount = 0;
    QDBusMessage msg = DBUS_BLOCK_INVOKE(m_defenderDataInvokerInter, "startLauncherManage");
    GET_MESSAGE_VALUE(bool, success, msg);
    if (success) {
        QDBusMessage msgEnable = DBUS_BLOCK_INVOKE(m_defenderDataInvokerInter, "getAppsInfoEnable");
        GET_MESSAGE_VALUE(QString, sEnableInfo, msgEnable);
        QApplication::processEvents();
        if (!sEnableInfo.isEmpty()) {
            nEnableCount = sEnableInfo.split("|").count();
            qInfo() << "[HomePageModel] [getAutoStartAppCount] " << nEnableCount;
        }
    }

    return nEnableCount;
}

// 添加安全日志
void HomePageModel::addSecurityLog(QString sInfo)
{
    DBUS_NOBLOCK_INVOKE(m_netFlowMonitorInvokerInter,
                        "AddSecurityLog",
                        SECURITY_LOG_TYPE_HOME,
                        sInfo);
}
