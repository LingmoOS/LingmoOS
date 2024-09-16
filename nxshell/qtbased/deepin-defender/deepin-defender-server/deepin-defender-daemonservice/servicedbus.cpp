// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "servicedbus.h"

const QString service = "com.deepin.defender.daemonservice";
const QString path = "/com/deepin/defender/daemonservice";

#define HEAET_BEAT_INTERVAL 3 // 心跳间隔/s

ServiceDBus::ServiceDBus(QObject *parent)
    : QObject(parent)
    , m_adaptor(nullptr)
    , m_monitornetflow(new MonitorNetFlow("com.deepin.defender.MonitorNetFlow", "/com/deepin/defender/MonitorNetFlow",
                                          QDBusConnection::systemBus(), this))
    , m_hmiscreenServer(new HmiscreenServer("com.deepin.defender.hmiscreen", "/com/deepin/defender/hmiscreen",
                                            QDBusConnection::sessionBus(), this))
    , m_DataInterFaceServer(new DataInterFaceServer("com.deepin.defender.datainterface", "/com/deepin/defender/datainterface",
                                                    QDBusConnection::sessionBus(), this))
    , m_defenderSetting(new QGSettings(DEEPIN_DEFENDER_GSETTING_PATH, QByteArray(), this))
    , m_restartHmiscreenFlag(false)
{
    m_adaptor = new DaemonserviceAdaptor(this);
    if (!QDBusConnection::sessionBus().registerService(service) || !QDBusConnection::sessionBus().registerObject(path, this)) {
        exit(0);
    }

    //每隔3秒调启动一次各应用
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ServiceDBus::StartAllService);
    m_timer->start(1000 * HEAET_BEAT_INTERVAL);
    StartAllService();
}

void ServiceDBus::ExitApp()
{
    qApp->quit();
}

void ServiceDBus::StartApp()
{
}

void ServiceDBus::StartAllService()
{
    m_timer->stop();
    // 安全中心root服务启动
    m_monitornetflow->StartApp();

    // 安全中心数据交互服务启动
    m_DataInterFaceServer->StartApp();

    // 重启安全中心后 启动界面服务
    if (m_restartHmiscreenFlag) {
        m_hmiscreenServer->Show();
        m_restartHmiscreenFlag = false;
    }
    m_timer->start(1000 * HEAET_BEAT_INTERVAL);
}

/*********************1030旧版安全中心接口******************/
//退出任何一个服务进程，都需要将daemon退出，否则3秒后该服务进程又会重启
void ServiceDBus::ExitAllService()
{
    m_timer->stop();
    m_restartHmiscreenFlag = true;
    m_hmiscreenServer->ExitApp();

    // 安全中心数据交互服务关闭
    m_DataInterFaceServer->ExitApp();

    // 安全中心root服务关闭
    m_monitornetflow->ExitApp();
    m_timer->start(1000 * HEAET_BEAT_INTERVAL);
}

void ServiceDBus::ExitMonitorNetFloService()
{
    m_monitornetflow->ExitApp();
    ExitApp();
}

// 获取密码修改错误提示文字
QString ServiceDBus::GetPwdError() const
{
    return m_DataInterFaceServer->GetPwdChangeError();
}

// 获取密码安全等级
int ServiceDBus::GetPwdLimitLevel() const
{
    return m_defenderSetting->get(PWD_LIMIT_LEVEL).toInt();
}
