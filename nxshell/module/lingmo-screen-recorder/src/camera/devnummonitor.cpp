// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "devnummonitor.h"

#include <QDebug>
#include <QCameraInfo>
#include <QString>
extern "C"
{
#include "v4l2_devices.h"
#include "camview.h"
}

DevNumMonitor::DevNumMonitor()
{
    m_noDevice = false;
    m_pTimer = nullptr;
    m_canUse = true;
    init();
}

/**
 * @brief 之前的代码timer 在子线程里面创建，timeOutSlot在主线程运行，修改为在主线程创建timer，无需开启一个挂起的线程。
 * 注意： timeOutSlot函数会修改摄像机管理链表内存、摄像机链表未加锁，并且发送信号的几个处理函数都将使用到摄像机链表，所以timeOutSlot必须在主线中使用
 */
void DevNumMonitor::init()
{
    m_pTimer = new QTimer;
    m_pTimer->setInterval(500);
    connect(m_pTimer, &QTimer::timeout, this, &DevNumMonitor::timeOutSlot);
}

//void DevNumMonitor::run()
//{
//    m_pTimer = new QTimer;
//    m_pTimer->setInterval(500);
//    connect(m_pTimer, &QTimer::timeout, this, &DevNumMonitor::timeOutSlot);
//    QObject::connect(this, &QThread::destroyed, m_pTimer, &QTimer::deleteLater);
//    m_pTimer->start();
//    this->exec();
//    qDebug() << "Start monitoring the number of devices!";
//}

void DevNumMonitor::timeOutSlot()
{
    check_device_list_events(get_v4l2_device_handler());
    if (get_device_list()->num_devices < 1) {
        //没有设备发送信号
        if (!m_noDevice) {
            emit existDevice(false);
            m_noDevice = true;
        }
        //qDebug() << "There is no camera connected!";
    } else {
        //v23特有处理，由于QCameraInfo::availableCameras().count()在v23上无法获取正确结果故更换为此方式
        int isExistAvailableCameras = 0;
        //qDebug() << "m_canUse: " << m_canUse;
        if (m_canUse) {
            for (int i = 0; i < get_device_list()->num_devices; i++) {
                int ret = camInit(get_device_list()->list_devices[i].device);
                camUnInit();
                if (ret == 0) {
                    isExistAvailableCameras++;
                    m_availableCamera = get_device_list()->list_devices[i].device;
                }
            }
        }
        if (m_canUse && isExistAvailableCameras == 0) {
            emit existDevice(false);
            m_noDevice = true;
            qDebug() << "There is no camera free!";
        } else {
            m_noDevice = false;
            emit existDevice(true);
            //qDebug() << "There are " << isExistAvailableCameras << " camera connected and free!";
        }
    }
}
void DevNumMonitor::setWatch(const bool isWatcher)
{
    if (isWatcher) {
        m_pTimer->start(1000);
    } else {
        m_pTimer->stop();
    }
}

void DevNumMonitor::setCanUse(bool canUse)
{
    m_canUse = canUse;
}

QString DevNumMonitor::availableCamera()
{
    return m_availableCamera;
}
DevNumMonitor::~DevNumMonitor()
{
    if (nullptr != m_pTimer) {
        m_pTimer->stop();
        m_pTimer->deleteLater();
        m_pTimer = nullptr;
    }
}
