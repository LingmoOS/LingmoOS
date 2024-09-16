// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainjob.h"
#include "deviceinterface.h"
#include "debugtimemanager.h"
#include "threadpool.h"
#include "detectthread.h"
#include "controlinterface.h"
#include "DDLog.h"

#include <QMutex>
#include <QProcess>
#include <QFile>
#include <QLoggingCategory>
#include <QTimer>

#include <DSysInfo>

using namespace DDLog;

static QMutex mainJobMutex;
static bool s_ServerIsUpdating = false;
static bool s_ClientIsUpdating = false;
const QString DEVICE_REPO_PATH = "/etc/apt/sources.list.d/devicemanager.list";
const QString DRIVER_REPO_PATH = "/etc/apt/sources.list.d/driver.list";

MainJob::MainJob(const char *name, QObject *parent)
    : QObject(parent)
    , m_pool(new ThreadPool)
    , m_firstUpdate(true)
{
    m_deviceInterface = new DeviceInterface(name, this);
    // 守护进程启动的时候加载所有信息
    updateAllDevice();

    // 启动线程监听USB是否有新的设备
    mp_DetectThread = new DetectThread(this);
    mp_DetectThread->setWorkingFlag(ControlInterface::getInstance()->monitorWorkingDBFlag());
    connect(mp_DetectThread, &DetectThread::usbChanged, this, &MainJob::slotUsbChanged, Qt::ConnectionType::QueuedConnection);

    // 在驱动管理延迟加载1000ms
    QTimer::singleShot(1000, this, [ = ]() {
#ifndef DISABLE_DRIVER
        DTK_CORE_NAMESPACE::DSysInfo::UosEdition type = DTK_CORE_NAMESPACE::DSysInfo::uosEditionType();
        if (DTK_CORE_NAMESPACE::DSysInfo::UosCommunity != type ) {
            initDriverRepoSource();
        }
#endif

        // 后台加载后先禁用设备
        QProcess process;
        QStringList options;
        options << "--netcard" << "--keyboard"  << "--mouse" <<  "--usb";
        process.start("hwinfo", options);
        process.waitForFinished(-1);
        QString info = process.readAllStandardOutput();
        process.close();

        ControlInterface::getInstance()->disableOutDevice(info);
        ControlInterface::getInstance()->disableInDevice();
        ControlInterface::getInstance()->updateWakeup(info);

        connect(m_deviceInterface, &DeviceInterface::sigUpdate, this, &MainJob::slotUsbChanged);
        connect(ControlInterface::getInstance(), &ControlInterface::sigUpdate, this, &MainJob::slotUsbChanged);
#ifndef DISABLE_DRIVER
        connect(ControlInterface::getInstance(), &ControlInterface::sigFinished, this, &MainJob::slotDriverControl);
#endif
    });
}

bool MainJob::serverIsRunning()
{
    return s_ServerIsUpdating;
}

bool MainJob::clientIsRunning()
{
    return s_ClientIsUpdating;
}

void MainJob::setWorkingFlag(bool flag)
{
    mp_DetectThread->setWorkingFlag(flag);
    ControlInterface::getInstance()->setMonitorWorkingDBFlag(flag);
}

void MainJob::slotUsbChanged()
{
    executeClientInstruction("DETECT");
}

void MainJob::slotDriverControl(bool success)
{
    if (success)
        executeClientInstruction("DETECT");
}

void MainJob::initDriverRepoSource()
{
    QFile fileDriver(DRIVER_REPO_PATH);
    if (fileDriver.open(QIODevice::ReadOnly)) {
        QString info = fileDriver.readAll();
        QStringList lines = info.split("\n");
        foreach (QString line, lines) {
            if (line.contains("pro-driver-packages")) {
                fileDriver.close();
                return;
            }
        }
        fileDriver.close();
    }

    QFile file(DEVICE_REPO_PATH);
    if (QFile::exists(DEVICE_REPO_PATH)) {
        return;
    }
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qCInfo(appLog) << file.errorString();
        return;
    }

    file.write("deb https://pro-driver-packages.uniontech.com eagle non-free\n");
    file.close();

    QString cmd = "apt update";
    QProcess process;
    process.start(cmd);
    process.waitForFinished(-1);
}

void MainJob::updateAllDevice()
{
    PERF_PRINT_BEGIN("POINT-01", "MainJob::updateAllDevice()");
    if (m_firstUpdate)
        m_pool->loadDeviceInfo();
    else
        m_pool->updateDeviceInfo();
    m_pool->waitForDone(-1);
    PERF_PRINT_END("POINT-01");
    m_firstUpdate = false;
}

void MainJob::executeClientInstruction(const QString &instructions)
{
    QMutexLocker locker(&mainJobMutex);
    s_ServerIsUpdating = true;

    if (instructions.startsWith("DETECT")) {
        this->thread()->msleep(1000);
        // 跟新缓存信息
        updateAllDevice();
    } else if (instructions.startsWith("START")) {
        if (m_firstUpdate) {
            updateAllDevice();
        }
    }
    s_ServerIsUpdating = false;
}
