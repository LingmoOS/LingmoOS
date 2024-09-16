// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// project self
#include "datadealsingleton.h"
#include "system/diskio_info.h"
#include "system/mem.h"
#include "system/net_info.h"
#include "system/system_monitor.h"
#include "system/device_db.h"
#include "model/cpu_info_model.h"
#include "system/block_device_info_db.h"
#include "common/datacommon.h"
#include "dbus/dbuscallmaininterface.h"
#include "config.h"
#include "helper.hpp"
#include "ddlog.h"

//Qt
#include <QProcess>
#include <QDBusMessage>
#include <QDBusConnection>

#ifdef IS_LOONGARCH_TYPE
#    define POPUP_WAITING_TIME 1000
#else
#    define POPUP_WAITING_TIME 500
#endif
using namespace DDLog;

QMutex DataDealSingleton::mutex;
QAtomicPointer<DataDealSingleton> DataDealSingleton::instance;

DataDealSingleton &DataDealSingleton::getInstance()
{
    if (instance.testAndSetOrdered(nullptr, nullptr)) {
        QMutexLocker locker(&mutex);

        instance.testAndSetOrdered(nullptr, new DataDealSingleton);
    }
    return *instance;
}

bool DataDealSingleton::readCpuPer(qreal &cpuPer)
{
    internalMutex.lockForRead();
    cpuPer = CPUInfoModel::instance()->cpuAllPercent();
    internalMutex.unlock();
    return true;
}

bool DataDealSingleton::readMemInfo(QString &memUsage, QString &memTotal, QString &memPercent, QString &swapUsage, QString &swapTotal, QString &swapPercent)
{
    internalMutex.lockForRead();
    MemInfo *curMemInfo = core::system::DeviceDB::instance()->memInfo();
    memUsage = formatUnit_memory_disk((curMemInfo->memTotal() - curMemInfo->memAvailable()) << 10, B, 1);
    memTotal = formatUnit_memory_disk(curMemInfo->memTotal() << 10, B, 1);
    memPercent = QString::number((curMemInfo->memTotal() - curMemInfo->memAvailable()) * 1. / curMemInfo->memTotal() * 100, 'f', 1);

    swapUsage = formatUnit_memory_disk((curMemInfo->swapTotal() - curMemInfo->swapFree()) << 10, B, 1);
    swapTotal = formatUnit_memory_disk(curMemInfo->swapTotal() << 10, B, 1);
    swapPercent = QString::number((curMemInfo->swapTotal() - curMemInfo->swapFree()) * 1. / curMemInfo->swapTotal() * 100, 'f', 1);
    internalMutex.unlock();

    return true;
}

bool DataDealSingleton::readNetInfo(QString &netReceive, QString &netTotalReceive, QString &netSend, QString &totalSend)
{
    internalMutex.lockForRead();
    NetInfo *netInfo = core::system::DeviceDB::instance()->netInfo();
    netReceive = formatUnit_net(netInfo->recvBps() * 8, B, 1, true);
    netSend = formatUnit_net(netInfo->sentBps() * 8, B, 1, true);
    netTotalReceive = formatUnit_net(netInfo->totalRecvBytes() * 8, B, 1);
    totalSend = formatUnit_net(netInfo->totalSentBytes() * 8, B, 1);
    internalMutex.unlock();

    return true;
}

bool DataDealSingleton::readDiskInfo(QString &diskRead, QString &diskTotalSize, QString &diskWrite, QString &diskAvailable)
{
    internalMutex.lockForRead();
    core::system::DiskIOInfo *diskIOInfo = core::system::DeviceDB::instance()->diskIoInfo();

    diskRead = formatUnit_memory_disk(diskIOInfo->diskIoReadBps(), B, 1, true);
    diskWrite = formatUnit_memory_disk(diskIOInfo->diskIoWriteBps(), B, 1, true);

    const QList<BlockDevice> &infoDB = DeviceDB::instance()->blockDeviceInfoDB()->deviceList();
    QMap<QString, BlockDevice> mapInfo;
    qulonglong totalDiskAva = 0;
    for (int i = 0; i < infoDB.size(); ++i) {
        mapInfo.insert(infoDB[i].deviceName(), infoDB[i]);

        totalDiskAva += infoDB[i].capacity();
    }

    diskTotalSize = formatUnit_memory_disk(totalDiskAva, B, 1);
    internalMutex.unlock();

    return true;
}

bool DataDealSingleton::sendJumpWidgetMessage(const QString &dbusMessage)
{
    //1000ms内重复点击,不响应
    if (m_popupTrickTimer->isActive()) {
        return false;
    }
    m_popupTrickTimer->start();

    //1.先唤醒主进程
    bool rt = false;
    if (common::systemInfo().isOldVersion()) {
        rt = QProcess::startDetached(Globals::DEEPIN_SYSTEM_MONITOR_PATH);
    } else {
        rt = launchMainProcessByAM();
    }
    if (true == rt) {
        //2.跳转DBUS
        QTimer::singleShot(POPUP_WAITING_TIME, this, [=]() {
            DbusCallMainInterface::getInstance()->jumpWidget(dbusMessage);
        });
    }
    return rt;
}

DataDealSingleton::DataDealSingleton(QObject *parent)
    : QObject(parent), m_popupTrickTimer(new QTimer(this))
{
    m_popupTrickTimer->setInterval(1000);
    m_popupTrickTimer->setSingleShot(true);

    connect(SystemMonitor::instance(), &SystemMonitor::statInfoUpdated, this, &DataDealSingleton::sigDataUpdate);
}

DataDealSingleton::~DataDealSingleton()
{
}

bool DataDealSingleton::launchMainProcessByAM() const
{
    QDBusMessage message = QDBusMessage::createMethodCall(
            "org.desktopspec.ApplicationManager1",
            "/org/desktopspec/ApplicationManager1/deepin_2dsystem_2dmonitor",
            "org.desktopspec.ApplicationManager1.Application",
            "Launch");

    message << QString("") << QStringList() << QVariantMap();

    QDBusMessage reply = QDBusConnection::sessionBus().call(message);
    if (reply.type() == QDBusMessage::ReplyMessage) {
        qCDebug(app) << "Method call successful!";
        return true;
    } else {
        qCWarning(app) << "Launch deepin-system-monitor main process error:" << reply.errorMessage();
        return false;
    }
}
