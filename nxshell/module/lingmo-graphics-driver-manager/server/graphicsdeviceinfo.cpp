// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "graphicsdeviceinfo.h"
#include <QProcess>
#include <QDebug>
#include <QDBusInterface>
#include <QFile>

extern "C" {
    #include <pci/pci.h>
}

#include <cstring>

#define CLASSNAME_VGA   "VGA compatible controller"
#define CLASSNAME_3D    "3D controller"

// NOTE: see more info /usr/share/misc/pci.ids
inline bool is_display_controller(const uint devClass)
{
    //qDebug() << "device_class：　"<< devClass;
    return (devClass & 0xff00) == 0x0300 || devClass == 0x0400;
}

DeviceFlag GraphicsDeviceInfo::deviceType(const QString &devInfo)
{
    if (devInfo.contains("Intel"))
        return DeviceFlag::INTEL;
    else if (devInfo.contains("NVIDIA"))
        return DeviceFlag::NVIDIA;
    else if (devInfo.contains("AMD"))
        return DeviceFlag::AMD;
    else
        return DeviceFlag::NoDevice;
}

GraphicsDeviceInfo::GraphicsDeviceInfo()
{
    init();
}


bool GraphicsDeviceInfo::isNotebook()
{
    QDBusInterface upowerInterface("org.freedesktop.UPower",
                                            "/org/freedesktop/UPower",
                                            "org.freedesktop.UPower",
                                            QDBusConnection::systemBus());
    QVariant  defaultDevice(upowerInterface.property("LidIsPresent"));
    if (defaultDevice.type() != QVariant::Type::Bool) {
        qCritical() << "failed get pc type.  " << defaultDevice.type();
        return false;
    }
    qDebug() << "defaultDevice.toBool() = " << defaultDevice.toBool();
    return defaultDevice.toBool();
}



void GraphicsDeviceInfo::init()
{
    struct pci_access *pacc;
    struct pci_dev *dev;
    char namebuf[1024];
    char path[1024], *base;
    int n;
    pacc = pci_alloc();
    pci_init(pacc);
    pci_scan_bus(pacc);
    for (dev = pacc->devices; dev; dev = dev->next)
    {
        pci_fill_info(dev, PCI_FILL_PHYS_SLOT | PCI_FILL_IDENT);
        if (!is_display_controller(dev->device_class))
            continue;
        base = pci_get_param(dev->access, "sysfs.path");
        if (!base || !base[0]){
            qWarning()<<"Get sysfs.path failed!";
            return;
        }

        n = snprintf(path, sizeof(path), "%s/devices/%04x:%02x:%02x.%d/",
            base, dev->domain, dev->bus, dev->dev, dev->func);
        if (n < 0 || n >= (int)sizeof(path)){
            qWarning("sysfs device name too long, why?");
            return;
        }
        qDebug()<<"path:" <<path;

        QFile file1(QString(path)+"driver");

        QString filename = file1.readLink();
        QString driver = filename.split("/")[filename.split("/").length()-1];

        bool boot_vga = false;
        QFile file2(QString(path)+"boot_vga");
        if (file2.exists()){
            file2.open(QIODevice::ReadOnly);
            if (!file2.isOpen()) {
                return;
            }
            QString str = file2.readAll();
            str.remove("\n");
            if( "1" == str){
                boot_vga = true;
             }
        }

        QString devName;
        DeviceFlag flag = DeviceFlag::NoDevice;
        const QString devInfo = pci_lookup_name(pacc, namebuf, sizeof(namebuf), PCI_LOOKUP_VENDOR | PCI_LOOKUP_DEVICE, dev->vendor_id, dev->device_id);
        if (devInfo.contains("Intel")){
            devName = "INTEL";
            flag = DeviceFlag::INTEL;
        }else if (devInfo.contains("NVIDIA")){
            devName = "NVIDIA";
            flag= DeviceFlag::NVIDIA;
        }else if (devInfo.contains("AMD")){
            devName = "AMD";
            flag= DeviceFlag::AMD;
        }
        m_sysDevFlag |= flag;

        if (!isNotebook() && !boot_vga) {
            continue;
        }
        qDebug("devName=%s, devInfo=%s, driver=%s, boot_vga=%d, m_sysDevFlag=%d", qPrintable(devName), qPrintable(devInfo), qPrintable(driver), boot_vga, m_sysDevFlag);
        Device device = Device(devName, devInfo, driver, boot_vga, flag);
        m_devices.append(device);
        if(boot_vga){
            m_curDevice = device;
        } 
    }
    pci_cleanup(pacc);
}
