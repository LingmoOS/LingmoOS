// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "homepagemodel.h"
#include "../common/comdata.h"
#include "../src/window/modules/common/invokers/invokerinterface.h"

#include <QGSettings>
#include <QStorageInfo>
#include <QDebug>
#include <QApplication>
#include <QtConcurrent>

#define DIALOG_WIDTH 380 //dialog  宽度
#define DIALOG_HEIGHT 145 //dialog  高度

DWIDGET_USE_NAMESPACE

HomePageModel::HomePageModel(InvokerFactoryInterface *factory, QObject *parent)
    : QObject(parent)
    , m_gSettingsInvokerInter(nullptr)
    , m_defenderDataInvokerInter(nullptr)
    , m_controlCenterInvokerInter(nullptr)
    , m_helperInvokerInter(nullptr)
    , m_diskManagerInvokerInter(nullptr)
    , m_netFlowMonitorInvokerInter(nullptr)
    , m_sysUpdaterModel(nullptr)
    , m_autoStartAppChecking(false)
    , m_sysVersionChecking(false)
    , m_trashChecking(false)
    , m_diskChecking(false)
    , m_developModeChecking(false)
{
    // invok inter
    if (factory) {
        //使用GSetting读取参数
        m_gSettingsInvokerInter = factory->CreateSettings(DEEPIN_DEFENDER_GSETTING_PATH, "", this);
        m_defenderDataInvokerInter = factory->CreateInvoker("com.deepin.defender.datainterface",
                                                            "/com/deepin/defender/datainterface",
                                                            "com.deepin.defender.datainterface",
                                                            ConnectType::SESSION, this);
        // 控制中心dbus服务对象
        m_controlCenterInvokerInter = factory->CreateInvoker("com.deepin.dde.ControlCenter",
                                                             "/com/deepin/dde/ControlCenter",
                                                             "com.deepin.dde.ControlCenter",
                                                             ConnectType::SESSION, this);
        // 安全中心dbus服务对象
        m_defenderWindowInvokerInter = factory->CreateInvoker("com.deepin.defender.hmiscreen",
                                                              "/com/deepin/defender/hmiscreen",
                                                              "com.deepin.defender.hmiscreen",
                                                              ConnectType::SESSION, this);
        // 同步助手dbus服务，用于查询是否处于开发者模式
        m_helperInvokerInter = factory->CreateInvoker("com.deepin.sync.Helper",
                                                      "/com/deepin/sync/Helper",
                                                      "com.deepin.sync.Helper",
                                                      ConnectType::SYSTEM, this);
        // 磁盘管理dbus服务
        m_diskManagerInvokerInter = factory->CreateInvoker("com.deepin.diskmanager",
                                                           "/com/deepin/diskmanager",
                                                           "com.deepin.diskmanager",
                                                           ConnectType::SYSTEM, this);
        // 流量监控系统dbus服务
        m_netFlowMonitorInvokerInter = factory->CreateInvoker("com.deepin.defender.MonitorNetFlow",
                                                              "/com/deepin/defender/MonitorNetFlow",
                                                              "com.deepin.defender.MonitorNetFlow",
                                                              ConnectType::SYSTEM, this);
        // 系统语言选择器dbus服务
        m_langSelectInvokerInter = factory->CreateInvoker("com.deepin.daemon.LangSelector",
                                                          "/com/deepin/daemon/LangSelector",
                                                          "com.deepin.daemon.LangSelector",
                                                          ConnectType::SESSION, this);
    }

    // 系统更新数据处理对象
    m_sysUpdaterModel = new SysUpdaterModel(factory, this);
}

// 设置上次体检分数
void HomePageModel::setSafetyScore(const int &score)
{
    m_gSettingsInvokerInter->SetValue(SAFETY_SCORE, score);
}

// 获取上次体检分数
int HomePageModel::GetSafetyScore()
{
    return m_gSettingsInvokerInter->GetValue(SAFETY_SCORE).toInt();
}

// 设置上次体检时间
void HomePageModel::SetLastCheckTime(QDateTime dateTime)
{
    m_gSettingsInvokerInter->SetValue(LAST_CHECK_TIME, dateTime.toString("yyyy-MM-dd-hh-mm-ss"));
}

// 获取上次体检时间
QDateTime HomePageModel::GetLastCheckTime()
{
    QString timeStr = m_gSettingsInvokerInter->GetValue(LAST_CHECK_TIME).toString();
    return QDateTime::fromString(timeStr, "yyyy-MM-dd-hh-mm-ss");
}

void HomePageModel::openStartupControlPage()
{
    m_defenderWindowInvokerInter->Invoke("ShowPage", {MODULE_SECURITY_TOOLS_NAME, STARTUP_CONTROL_NAME}, BlockMode::BLOCK);
    addSecurityLog(tr("Startup programs checked"));
}

// 打开系统更新界面
void HomePageModel::openSysUpdatePage()
{
    m_controlCenterInvokerInter->Invoke("Show", {}, BlockMode::NOBLOCK);
    m_controlCenterInvokerInter->Invoke("ShowPage", {"update", ""}, BlockMode::NOBLOCK);
    addSecurityLog(tr("System updated"));
}

// 磁盘管理器 是否已安装
bool HomePageModel::isDiskManagerInterValid()
{
    QProcess proc;
    // 查询是否已经安装 磁盘管理器
    proc.start("dpkg -s deepin-diskmanager");
    proc.waitForFinished();
    QString status = proc.readAllStandardOutput();

    // 若返回结果以 "Package: deepin-diskmanager\nStatus: install"，则说明已安装
    return status.startsWith("Package: deepin-diskmanager\nStatus: install");
}

// 获得改变地址的状态
bool HomePageModel::getChangeAddressStatus()
{
    return m_gSettingsInvokerInter->GetValue(VRIUS_ADDRESS_CHANGHE).toBool();
}

// 获取磁盘状态
bool HomePageModel::getDiskStatus()
{
    // 遍历所有磁盘设备路径
    QVariantList args = m_netFlowMonitorInvokerInter->Invoke("GetDiskDevicePathList", {}, BlockMode::BLOCK).arguments();
    if (args.isEmpty()) {
        return true;
    }
    QStringList devPaths = args.takeFirst().toStringList();
    for (QString devPath : devPaths) {
        QVariantList statusArgs = m_diskManagerInvokerInter->Invoke("onGetDeviceHardStatus", {devPath}, BlockMode::BLOCK).arguments();
        QString data;
        if (!statusArgs.isEmpty()) {
            data = statusArgs.takeFirst().toString();
        }
        // 状态结果转小写
        QString status = data.toLower();
        // 若状态为异常，返回false
        if ("passed" != status) {
            return false;
        }
    }

    // 所有磁盘正常
    return true;
}

// 是否处于开发者模式
bool HomePageModel::IsDeveloperMode()
{
    QVariantList args = m_helperInvokerInter->Invoke("IsDeveloperMode", {}, BlockMode::BLOCK).arguments();
    if (args.isEmpty()) {
        return true;
    }
    return args.takeFirst().toBool();
}

bool HomePageModel::ignoreDevModeChecking() const
{
    return m_gSettingsInvokerInter->GetValue(IGNORE_DEV_MODE_CHECKING).toBool();
}

void HomePageModel::setIgnoreDevModeChecking(bool ignore)
{
    m_gSettingsInvokerInter->SetValue(IGNORE_DEV_MODE_CHECKING, ignore);
}

QString HomePageModel::getLocalLang()
{
    QString lang;
    QVariantList args = m_langSelectInvokerInter->Invoke(DBUS_PROPERTY_INVOKER_NAME, {"CurrentLocale"}, BlockMode::BLOCK).arguments();
    if (args.isEmpty()) {
        return lang;
    }
    lang = Utils::checkAndTransQDbusVarIntoQVar(args.first()).toString();
    return lang;
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
    m_netFlowMonitorInvokerInter->Invoke("AsyncGetRemRegisterStatus", {}, BlockMode::BLOCK);
}

void HomePageModel::asyncSetSSHStatus(bool status)
{
    m_netFlowMonitorInvokerInter->Invoke("AsyncSetRemRegisterStatus", {status}, BlockMode::BLOCK);
}

// 获取允许自启动应用个数
int HomePageModel::getAutoStartAppCount() const
{
    int nEnableCount = 0;
    bool success;
    QVariantList successArgs = m_defenderDataInvokerInter->Invoke("startLauncherManage", {}, BlockMode::BLOCK).arguments();
    if (successArgs.isEmpty()) {
        success = false;
    } else {
        success = successArgs.takeFirst().toBool();
    }
    if (success) {
        QVariantList args = m_defenderDataInvokerInter->Invoke("getAppsInfoEnable", {}, BlockMode::BLOCK).arguments();
        if (args.isEmpty()) {
            return nEnableCount;
        }
        QString sEnableInfo = args.takeFirst().toString();
        QApplication::processEvents();
        if (!sEnableInfo.isEmpty())
            nEnableCount = sEnableInfo.split("|").count();
    }

    return nEnableCount;
}

// 添加安全日志
void HomePageModel::addSecurityLog(QString sInfo)
{
    m_netFlowMonitorInvokerInter->Invoke("AddSecurityLog", {SECURITY_LOG_TYPE_HOME, sInfo}, BlockMode::NOBLOCK);
}
