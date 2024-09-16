// Copyright (C) 2019 ~ 2023 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "netcheckdbusinter.h"
#include "netchecksyssrvmodel.h"

#include <QtConcurrent/QtConcurrent>
#include <QDBusContext>
#include <QDebug>

// 合法调用进程路径列表
const QStringList ValidInvokerExePathList =
    {"/usr/bin/deepin-pc-manager",
     "/usr/bin/deepin-pc-manager-session-daemon",
     "/usr/bin/deepin-pc-manager-system-daemon"};

NetCheckDBusInter::NetCheckDBusInter(QObject *parent)
    : QObject(parent)
    , m_adaptor(new NetcheckAdaptor(this))
    , m_netCheckSrvModelThread(nullptr)
    , m_netCheckModel(new NetCheckSysSrvModel(this))
{
    if (!QDBusConnection::systemBus().registerService(NET_CHECK_DBUS_NAME) || !QDBusConnection::systemBus().registerObject(NET_CHECK_DBUS_PATH, this)) {
        exit(0);
    }

    // 文件审计线程
    m_netCheckSrvModelThread = new QThread;
    m_netCheckModel = new NetCheckSysSrvModel();
    m_netCheckModel->moveToThread(m_netCheckSrvModelThread);

    // 连接信号
    connect(m_netCheckSrvModelThread, &QThread::started, m_netCheckModel, &NetCheckSysSrvModel::initData);
    connect(this, &NetCheckDBusInter::sendCheckNetDevice, m_netCheckModel, &NetCheckSysSrvModel::startCheckNetDevice);
    connect(this, &NetCheckDBusInter::sendCheckNetConnSetting, m_netCheckModel, &NetCheckSysSrvModel::startCheckNetConnSetting);
    connect(this, &NetCheckDBusInter::sendCheckNetDHCP, m_netCheckModel, &NetCheckSysSrvModel::startCheckNetDHCP);
    connect(this, &NetCheckDBusInter::sendCheckNetDNS, m_netCheckModel, &NetCheckSysSrvModel::startCheckNetDNS);
    connect(this, &NetCheckDBusInter::sendCheckNetHost, m_netCheckModel, &NetCheckSysSrvModel::startCheckNetHost);
    connect(this, &NetCheckDBusInter::sendCheckNetConn, m_netCheckModel, &NetCheckSysSrvModel::startCheckNetConn);

    connect(m_netCheckModel, &NetCheckSysSrvModel::sendCheckNetDevice, this, &NetCheckDBusInter::acceptCheckNetDevice);
    connect(m_netCheckModel, &NetCheckSysSrvModel::sendCheckNetConnSetting, this, &NetCheckDBusInter::acceptCheckNetConnSetting);
    connect(m_netCheckModel, &NetCheckSysSrvModel::sendCheckNetDHCP, this, &NetCheckDBusInter::acceptCheckNetDHCP);
    connect(m_netCheckModel, &NetCheckSysSrvModel::sendCheckNetDNS, this, &NetCheckDBusInter::acceptCheckNetDNS);
    connect(m_netCheckModel, &NetCheckSysSrvModel::sendCheckNetHost, this, &NetCheckDBusInter::acceptCheckNetHost);
    connect(m_netCheckModel, &NetCheckSysSrvModel::sendCheckNetConn, this, &NetCheckDBusInter::acceptCheckNetConn);

    //启动线程
    m_netCheckSrvModelThread->start();
}

NetCheckDBusInter::~NetCheckDBusInter()
{
}

void NetCheckDBusInter::StartApp()
{
}

void NetCheckDBusInter::ExitApp()
{
    if (!isValidInvoker()) {
        qDebug() << Q_FUNC_INFO << "invalid invoker!";
        return;
    }
    qApp->exit();
}

// 网络硬件检测
void NetCheckDBusInter::StartCheckNetDevice()
{
    if (!isValidInvoker()) {
        qDebug() << Q_FUNC_INFO << "invalid invoker!";
        return;
    }
    m_isNetChecking = true;
    Q_EMIT sendCheckNetDevice();
}
// 网络连接配置检测
void NetCheckDBusInter::StartCheckNetConnSetting()
{
    if (!isValidInvoker()) {
        qDebug() << Q_FUNC_INFO << "invalid invoker!";
        return;
    }
    Q_EMIT sendCheckNetConnSetting();
}
// DHCP检测
void NetCheckDBusInter::StartCheckNetDHCP()
{
    if (!isValidInvoker()) {
        qDebug() << Q_FUNC_INFO << "invalid invoker!";
        return;
    }
    Q_EMIT sendCheckNetDHCP();
}
// DNS检测
void NetCheckDBusInter::StartCheckNetDNS()
{
    if (!isValidInvoker()) {
        qDebug() << Q_FUNC_INFO << "invalid invoker!";
        return;
    }
    Q_EMIT sendCheckNetDNS();
}
// host检测
void NetCheckDBusInter::StartCheckNetHost()
{
    if (!isValidInvoker()) {
        qDebug() << Q_FUNC_INFO << "invalid invoker!";
        return;
    }
    Q_EMIT sendCheckNetHost();
}
// 网络连接访问检测
void NetCheckDBusInter::StartCheckNetConn(QString url)
{
    if (!isValidInvoker()) {
        qDebug() << Q_FUNC_INFO << "invalid invoker!";
        return;
    }
    Q_EMIT sendCheckNetConn(url);
}

// 接收网络硬件检测结果
void NetCheckDBusInter::acceptCheckNetDevice(int result)
{
    Q_EMIT NotifyCheckNetDevice(result);
}
// 接收网络连接配置检测结果
void NetCheckDBusInter::acceptCheckNetConnSetting(int result)
{
    Q_EMIT NotifyCheckNetConnSetting(result);
}
// 接收DHCP检测结果
void NetCheckDBusInter::acceptCheckNetDHCP(int result)
{
    Q_EMIT NotifyCheckNetDHCP(result);
}
// 接收DNS检测结果
void NetCheckDBusInter::acceptCheckNetDNS(int result)
{
    Q_EMIT NotifyCheckNetDNS(result);
}
// 接收host检测结果
void NetCheckDBusInter::acceptCheckNetHost(int result)
{
    Q_EMIT NotifyCheckNetHost(result);
}
// 接收网络连接访问检测结果
void NetCheckDBusInter::acceptCheckNetConn(int result)
{
    m_isNetChecking = false;
    Q_EMIT NotifyCheckNetConn(result);
}

// 获取网络检测状态
bool NetCheckDBusInter::GetNetCheckStatus()
{
    return m_isNetChecking;
}

void NetCheckDBusInter::SetNetCheckStatus(bool status)
{
    m_isNetChecking = status;
}

// 校验调用者
bool NetCheckDBusInter::isValidInvoker()
{
#ifdef QT_DEBUG
    return true;
#else
    // 判断是否位自身调用
    if (!calledFromDBus()) {
        return true;
    }

    bool valid = false;
    QDBusConnection conn = connection();
    QDBusMessage msg = message();

    // 判断是否存在执行路径且是否存在于可调用者名单中
    uint pid = conn.interface()->servicePid(msg.service()).value();
    QFileInfo f(QString("/proc/%1/exe").arg(pid));
    QString invokerPath = f.canonicalFilePath();

    // 防止读取的执行路径为空，为空时取cmdline内容
    if (invokerPath.isEmpty()) {
        QFile cmdlineFile(QString("/proc/%1/cmdline").arg(pid));
        if (cmdlineFile.open(QIODevice::OpenModeFlag::ReadOnly)) {
            QString sCmdline = cmdlineFile.readAll();
            invokerPath = sCmdline.split(" ").first();
        }
        cmdlineFile.close();
    }

    if (ValidInvokerExePathList.contains(invokerPath)) {
        valid = true;
    }

    // 如果是非法调用者，则输出错误
    if (!valid) {
        sendErrorReply(QDBusError::ErrorType::Failed,
                       QString("(pid: %1)[%2] is not allowed to transfer login inter")
                           .arg(pid)
                           .arg(invokerPath));
    }

    return valid;
#endif
}
