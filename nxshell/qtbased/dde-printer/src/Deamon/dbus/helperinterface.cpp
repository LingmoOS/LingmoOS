// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "helperinterface.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingReply>
#include <QProcess>
#include <QCoreApplication>

#define TIMEOUT 1000 * 60 * 5

HelperInterface::HelperInterface(CupsMonitor *pCupsMonitor, QObject *parent)
    : QObject(parent)
    , m_pCupsMonitor(pCupsMonitor)
    , m_pSystemTray(new QSystemTrayIcon(QIcon(":/images/dde-printer.svg"), this))
{
    if (m_pCupsMonitor) {
        connect(m_pCupsMonitor, &CupsMonitor::signalJobStateChanged, this, &HelperInterface::signalJobStateChanged);
        connect(m_pCupsMonitor, &CupsMonitor::signalPrinterStateChanged, this, &HelperInterface::signalPrinterStateChanged);
        connect(m_pCupsMonitor, &CupsMonitor::signalPrinterDelete, this, &HelperInterface::signalPrinterDelete);
        connect(m_pCupsMonitor, &CupsMonitor::signalPrinterAdd, this, &HelperInterface::signalPrinterAdd);
        connect(m_pCupsMonitor, &CupsMonitor::signalShowTrayIcon, this, &HelperInterface::slotShowTrayIcon);

        /*托盘需要在主线程创建*/
        connect(m_pSystemTray, &QSystemTrayIcon::activated, this, [&](QSystemTrayIcon::ActivationReason reason) {
            if (reason == QSystemTrayIcon::Trigger) {
                showJobsWindow();
            }
        });
    }

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, QCoreApplication::instance(), &QCoreApplication::quit);
    m_timer->start(TIMEOUT);
}

HelperInterface::~HelperInterface()
{

}

bool HelperInterface::isJobPurged(int id)
{
    if (m_pCupsMonitor) {
        return m_pCupsMonitor->isJobPurged(id);
    }
    qWarning() << "m_pCupsMonitor is null";
    return false;
}

QString HelperInterface::getJobNotify(const QMap<QString, QVariant> &job)
{
    if (m_pCupsMonitor) {
        return m_pCupsMonitor->getJobNotify(job);
    }
    qWarning() << "m_pCupsMonitor is null";
    return "";
}

QString HelperInterface::getStateString(int iState)
{
    if (m_pCupsMonitor) {
        return m_pCupsMonitor->getStateString(iState);
    }
    qWarning() << "m_pCupsMonitor is null";
    return "";
}

void HelperInterface::slotShowTrayIcon(bool bShow)
{
    if (bShow)
        m_pSystemTray->show();
    else
        m_pSystemTray->hide();
}

void HelperInterface::registerDBus()
{
    //注册dbus服务
    QDBusConnection dbus = QDBusConnection::sessionBus();
    bool ret = dbus.registerService(SERVICE_INTERFACE_NAME);
    if (ret == false) {
        qWarning("Register QDBus Service Error!");
    }
    ret = dbus.registerObject(SERVICE_INTERFACE_PATH, this,
                              QDBusConnection::ExportAllSlots |
                              QDBusConnection::ExportAllSignals);
    if (ret == false) {
        qDebug("Register QDBus Object Error!");
    }

}

void HelperInterface::unRegisterDBus()
{
    //注销dbus服务
    QDBusConnection dbus = QDBusConnection::sessionBus();
    bool ret = dbus.unregisterService(SERVICE_INTERFACE_NAME);
    if (ret == false) {
        qWarning("unregister QDBus Service Error!");

    }
    dbus.unregisterObject(SERVICE_INTERFACE_PATH);
    dbus.disconnectFromBus(dbus.name());
}

void HelperInterface::showJobsWindow()
{
    QProcess process;
    QString cmd = "dde-printer";
    QStringList args;
    args << "-m" << "4";
    if (!process.startDetached(cmd, args)) {
        qWarning() << QString("showJobsWindow failed because %1").arg(process.errorString());
    }
}

void HelperInterface::setTypeAndState(int status)
{
    m_timer->start(TIMEOUT);

    if (status == 2) { // usb打印设备
        emit usbDeviceProcess();
    }
}


