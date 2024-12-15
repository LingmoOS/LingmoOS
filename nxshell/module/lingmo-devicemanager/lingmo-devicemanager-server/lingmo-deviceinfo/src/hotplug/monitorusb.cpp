// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "monitorusb.h"
#include "controlinterface.h"
#include "mainjob.h"

#include <QLoggingCategory>
#include <QProcess>
#include <QFile>
#include <QDateTime>

MonitorUsb::MonitorUsb()
    : m_Udev(nullptr)
    , mp_Timer(new QTimer(this))
    , m_UsbChangeTime()
    , m_UsbChanged(false)
    , m_workingFlag(true)
{
    m_Udev = udev_new();
    if (!m_Udev) {
        printf("error!!!\n");
    }
    // 创建一个新的monitor
    mon = udev_monitor_new_from_netlink(m_Udev, "udev");
    // 增加一个udev事件过滤器
    udev_monitor_filter_add_match_subsystem_devtype(mon, "usb", nullptr);
    udev_monitor_filter_add_match_subsystem_devtype(mon, "bluetooth", nullptr);
    // 启动监控
    udev_monitor_enable_receiving(mon);
    // 获取该监控的文件描述符，fd就代表了这个监控
    fd = udev_monitor_get_fd(mon);

    // 定时器发送消息
    connect(mp_Timer, &QTimer::timeout, this, &MonitorUsb::slotTimeout);
    mp_Timer->start(1000);
}

void MonitorUsb::monitor()
{
    char buf[10];
    fd_set fds;
    struct timeval tv;
    while (true) {
        if (!m_workingFlag)
            break;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 10000;
        int ret = select(fd + 1, &fds, nullptr, nullptr, &tv);

        // 判断是否有事件产生
        if (!ret)
            continue;
        if (! FD_ISSET(fd, &fds))
            continue;

        // 获取产生事件的设备映射
        struct udev_device *dev = udev_monitor_receive_device(mon);
        if (!dev)
            continue;

        // 监测蓝牙设备
        if (0 == strcmp(udev_device_get_devtype(dev), "link") && m_workingFlag) {
            emit usbChanged();
            continue;
        }

        // 获取事件并判断是否是插拔
        unsigned long long curNum = udev_device_get_devnum(dev);
        if (0 == curNum) {
            udev_device_unref(dev);
            continue;
        }

        // 只有add和remove事件才会更新缓存信息
        strcpy(buf, udev_device_get_action(dev));
        if ((0 == strcmp("add", buf) || 0 == strcmp("remove", buf)) && m_workingFlag) {
            QProcess process;
            process.start("hwinfo --usb");
            process.waitForFinished(-1);
            QString info = process.readAllStandardOutput();
            if (0 == strcmp("add", buf)) {
                ControlInterface::getInstance()->disableOutDevice(info);
            }
            ControlInterface::getInstance()->updateWakeup(info);
            m_UsbChanged = true;
            m_UsbChangeTime = QDateTime::currentMSecsSinceEpoch();
        }

        udev_device_unref(dev);
    }
}

void MonitorUsb::setWorkingFlag(bool flag)
{
    m_workingFlag = flag;
}

void MonitorUsb::slotTimeout()
{
    if (!m_UsbChanged || !m_workingFlag)
        return;
    if (QDateTime::currentMSecsSinceEpoch() - m_UsbChangeTime < 1000)
        return;
    m_UsbChanged = false;
    emit usbChanged();
}
