// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "block_device_info_db.h"
#include "block_device.h"
#include "ddlog.h"
#include <QReadLocker>
#include <QWriteLocker>
#include <QDebug>
#include <QFile>
#include "diskio_info.h"
#include "common/common.h"
#include "system/sys_info.h"
#include <QDir>
#include <ctype.h>
#include <errno.h>
#include <sched.h>

#include <libudev.h>

using namespace DDLog;

#define SYSFS_PATH_VIRTUAL_BLOCK "/sys/devices/virtual/block"

namespace core {
namespace system {
/*static struct udev_device *
get_child(struct udev *udev, struct udev_device *parent, const char *subsystem)
{
    struct udev_device *child = nullptr;
    struct udev_enumerate *enumerate = udev_enumerate_new(udev);

    udev_enumerate_add_match_parent(enumerate, parent);
    udev_enumerate_add_match_subsystem(enumerate, subsystem);
    udev_enumerate_scan_devices(enumerate);

    struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
    struct udev_list_entry *entry;

    udev_list_entry_foreach(entry, devices) {
        const char *path = udev_list_entry_get_name(entry);
        child = udev_device_new_from_syspath(udev, path);
        break;
    }

    udev_enumerate_unref(enumerate);
    return child;
}*/

//static void enumerate_usb_mass_storage(struct udev *udev)
//{
//    struct udev_enumerate *enumerate = udev_enumerate_new(udev);

//    udev_enumerate_add_match_subsystem(enumerate, "scsi");
//    udev_enumerate_add_match_property(enumerate, "DEVTYPE", "scsi_device");
//    udev_enumerate_scan_devices(enumerate);

//    struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
//    struct udev_list_entry *entry;

//    udev_list_entry_foreach(entry, devices) {
//        const char *path = udev_list_entry_get_name(entry);
//        struct udev_device *scsi = udev_device_new_from_syspath(udev, path);

//        struct udev_device *block = get_child(udev, scsi, "block");
//        struct udev_device *scsi_disk = get_child(udev, scsi, "scsi_disk");

//        //        struct udev_device *usb
//        //            = udev_device_get_parent_with_subsystem_devtype(scsi, "usb", "usb_device");

//        if (block && scsi_disk) {
//            printf("block = %s, scsi = %s\n",
//                   udev_device_get_devnode(block),
//                   //                   udev_device_get_sysattr_value(usb, "idVendor"),
//                   //                   udev_device_get_sysattr_value(usb, "idProduct"),
//                   udev_device_get_sysattr_value(scsi, "vendor"));
//        }

//        if (block) {
//            udev_device_unref(block);
//        }

//        if (scsi_disk) {
//            udev_device_unref(scsi_disk);
//        }

//        udev_device_unref(scsi);
//    }

//    udev_enumerate_unref(enumerate);
//}

BlockDeviceInfoDB::BlockDeviceInfoDB()
    : m_deviceList {}
{
}

BlockDeviceInfoDB::~BlockDeviceInfoDB()
{
}

void BlockDeviceInfoDB::readDiskInfo()
{
    QDir dir(SYSFS_PATH_BLOCK);
    if (!dir.exists()) {
        return;
    }

    QFileInfoList list = dir.entryInfoList();
    //获取实体磁盘
    for (int i = 0; i < list.size(); ++i) {
        QString t_link = list.at(i).readLink();
        if (list[i].fileName() != "." && list[i].fileName() != ".." && !list[i].fileName().contains("ram") && !list[i].fileName().contains("loop") && !t_link.contains("virtual")) {
            int index = -1;
            //  查找当前的device是否存在
            for (int si = 0; si < m_deviceList.size(); ++si) {
                if (m_deviceList[si].deviceName() == list[i].fileName().toLocal8Bit()) {   // 存在
                    index = si;
                    break;
                }
            }
            if (index == -1) {   // 不存在的话将该disk存储起来
                BlockDevice bd;
                if (bd.readDeviceSize(list[i].fileName()) > 0) {
                    bd.setDeviceName(list[i].fileName().toLocal8Bit());
                    m_deviceList << bd;
                }
            } else {
                m_deviceList[index].setDeviceName(list[i].fileName().toLocal8Bit());   // 更新disk数据
            }
        }
    }

    //获取虚拟磁盘
    for (int i = 0; i < list.size(); ++i) {
        QString t_link = list.at(i).readLink();
        if (list[i].fileName() != "." && list[i].fileName() != ".." && !list[i].fileName().contains("ram") && !list[i].fileName().contains("loop") && t_link.contains("virtual")) {
            int index = -1;
            //  查找当前的device是否存在
            for (int si = 0; si < m_deviceList.size(); ++si) {
                if (m_deviceList[si].deviceName() == list[i].fileName().toLocal8Bit()) {   // 存在
                    index = si;
                    break;
                }
            }
            if (index == -1) {   // 不存在的话将该disk存储起来
                BlockDevice bd;
                if (bd.readDeviceSize(list[i].fileName()) > 0) {
                    bd.setDeviceName(list[i].fileName().toLocal8Bit());
                    m_deviceList << bd;
                }
            } else {
                m_deviceList[index].setDeviceName(list[i].fileName().toLocal8Bit());   // 更新disk数据
            }
        }
    }

    for (int i = 0; i < m_deviceList.size(); ++i) {
        bool isFind = false;
        for (int si = 0; si < list.size(); ++si) {
            if (list[si].fileName().toLocal8Bit() == m_deviceList[i].deviceName()) {
                isFind = true;
                break;
            }
        }
        if (!isFind) {
            m_deviceList.removeAt(i);
        }
    }
}

//static void enum_block()
//{
//    struct udev *udev;
//    struct udev_device *dev;
//    struct udev_enumerate *enumerate;
//    struct udev_list_entry *devices, *dev_list_entry;

//    /* create udev object */
//    udev = udev_new();
//    if (!udev) {
//        fprintf(stderr, "Cannot create udev context.\n");
//        return;
//    }

//    /* create enumerate object */
//    enumerate = udev_enumerate_new(udev);
//    if (!enumerate) {
//        fprintf(stderr, "Cannot create enumerate context.\n");
//        return;
//    }

//    udev_enumerate_add_match_subsystem(enumerate, "net");
//    udev_enumerate_scan_devices(enumerate);

//    /* fillup device list */
//    devices = udev_enumerate_get_list_entry(enumerate);
//    if (!devices) {
//        fprintf(stderr, "Failed to get device list.\n");
//        return;
//    }

//    udev_list_entry_foreach(dev_list_entry, devices) {
//        const char *path;
//        //unsigned long long disk_size = 0;
//        //unsigned short int block_size = 512;

//        path = udev_list_entry_get_name(dev_list_entry);
//        dev = udev_device_new_from_syspath(udev, path);

////        qCDebug(app) << "devNode" << udev_device_get_devnode(dev);
////        qCDebug(app) << "devType" << udev_device_get_devtype(dev);
////        qCDebug(app) << "sysName" << udev_device_get_sysname(dev);
////        qCDebug(app) << "devPath" << udev_device_get_devpath(dev);
////        qCDebug(app) << "sysPath" << udev_device_get_syspath(dev);
////        qCDebug(app) << "sysnum" << udev_device_get_sysnum(dev);
////        qCDebug(app) << "subsystem" << udev_device_get_subsystem(dev);

////        auto *parent = udev_device_get_parent(dev);
////        qCDebug(app) << "parent sysname: " << udev_device_get_sysname(parent);
////        qCDebug(app) << "parent devpath: " << udev_device_get_devpath(parent);
////        qCDebug(app) << "parent syspath: " << udev_device_get_syspath(parent);

//        //        /* skip if device/disk is a partition or loop device */
//        //        if (strncmp(udev_device_get_devtype(dev), "partition", 9) != 0 &&
//        //                strncmp(udev_device_get_sysname(dev), "loop", 4) != 0) {
//        //            printf("I: DEVNODE=%s\n", udev_device_get_devnode(dev));
//        //            printf("I: KERNEL=%s\n", udev_device_get_sysname(dev));
//        //            printf("I: DEVPATH=%s\n", udev_device_get_devpath(dev));
//        //            printf("I: DEVTYPE=%s\n", udev_device_get_devtype(dev));
//        //            fflush(stdout);

//        //            tmp = udev_device_get_sysattr_value(dev, "size");
//        //            if (tmp)
//        //                disk_size = strtoull(tmp, NULL, 10);

//        //            tmp = udev_device_get_sysattr_value(dev, "queue/logical_block_size");
//        //            if (tmp)<device>
//        //                block_size = atoi(tmp);

//        //            printf("I: DEVSIZE=");
//        //            if (strncmp(udev_device_get_sysname(dev), "sr", 2) != 0)
//        //                printf("%lld GB\n", (disk_size * block_size) / 1000000000);
//        //            else
//        //                printf("n/a\n");
//        //        }

//        /* free dev */
//        udev_device_unref(dev);
//    }
//    /* free enumerate */
//    udev_enumerate_unref(enumerate);
//    /* free udev */
//    udev_unref(udev);
//}

void BlockDeviceInfoDB::update()
{
    readDiskInfo();

    // TODO: enum device in /sys/block => phy & virtual

    // TEST begin (move udev logic to udev & udevice)
    //    enum_block();

    //    struct udev *udev = udev_new();

    //    enumerate_usb_mass_storage(udev);

    //    udev_unref(udev);
    //    // TEST end

    //    QWriteLocker lock(&m_rwlock);
    // update device list
}

}   // namespace system
}   // namespace core
