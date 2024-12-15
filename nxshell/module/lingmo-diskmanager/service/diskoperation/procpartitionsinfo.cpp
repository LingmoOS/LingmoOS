// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "procpartitionsinfo.h"
#include "blockspecial.h"
#include "utils.h"

#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>

#include <fstream>

namespace DiskManager {

bool ProcPartitionsInfo::procPartitionsInfoCacheInitialized = false;
QVector<QString> ProcPartitionsInfo::devicePathsCache;

void ProcPartitionsInfo::loadCache()
{
    loadProcPartitionsInfoCache();
    procPartitionsInfoCacheInitialized = true;
}

const QVector<QString> &ProcPartitionsInfo::getDevicePaths()
{
    initializeIfRequired();
    return devicePathsCache;
}

void ProcPartitionsInfo::initializeIfRequired()
{
    if (!procPartitionsInfoCacheInitialized) {
        loadProcPartitionsInfoCache();
        procPartitionsInfoCacheInitialized = true;
    }
}

void ProcPartitionsInfo::loadProcPartitionsInfoCache()
{
    devicePathsCache.clear();
    QFile file("/proc/partitions");

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        in.skipWhiteSpace();
        QString line = in.readLine();
        QString device;
        while (!in.atEnd() || !line.isEmpty()) {
            qDebug() << __FUNCTION__ << "-----++++++------";
            QStringList strlist = line.split(" ");
            unsigned long maj = 0;
            unsigned long min = 0;
            if (sscanf(line.toStdString().c_str(), "%lu %lu", &maj, &min) != 2) {
                line = in.readLine();
                continue;
            }
            QString name;
            foreach (QString str, strlist) {
                if (!str.isEmpty() && str.indexOf(QRegularExpression("[a-z]+\\d?")) != -1) {
                    name = str;
                    break;
                }
            }
//            qDebug() << name;

            BlockSpecial::registerBlockSpecial("/dev/" + name, maj, min);

            // Recognise only whole disk device names, excluding partitions,
            // from /proc/partitions and save in this cache.

            //Whole disk devices are the ones we want.
            //Device names without a trailing digit refer to the whole disk.
            device = Utils::regexpLabel(name, "^([^0-9]+)$");

            //Recognize /dev/md* devices (Linux software RAID - mdadm).
            //E.g., device = /dev/md127, partition = /dev/md127p1
            if (device == "") {
                device = Utils::regexpLabel(name, "^(md[0-9]+)$");
            }

            //Recognize /dev/mmcblk* devices.
            //E.g., device = /dev/mmcblk0, partition = /dev/mmcblk0p1
            if (device == "") {
                device = Utils::regexpLabel(name, "^(mmcblk[0-9]+)$");
            }

            // Recognise /dev/nvme*n* devices
            // (Non-Volatile Memory Express devices.  SSD type devices which
            // plug directly into PCIe sockets).
            // E.g., device = /dev/nvme0n1, partition = /dev/nvme0n1p1
            if (device == "") {
                device = Utils::regexpLabel(name, "^(nvme[0-9]+n[0-9]+)$");
            }

            //Device names that end with a #[^p]# are HP Smart Array Devices (disks)
            //  E.g., device = /dev/cciss/c0d0, partition = /dev/cciss/c0d0p1
            //  (linux-x.y.z/Documentation/blockdev/cciss.txt)
            //Device names for Compaq SMART2 Intelligent Disk Array
            //  E.g., device = /dev/ida/c0d0, partition = /dev/ida/c0d0p1
            //  (linux-x.y.z/Documentation/blockdev/cpqarray.txt)
            //Device names for Mylex DAC960/AcceleRAID/eXtremeRAID PCI RAID
            //  E.g., device = /dev/rd/c0d0, partition = /dev/rd/c0d0p1
            //  (linux-x.y.z/Documentation/blockdev/README.DAC960)
            if (device == "") {
                device = Utils::regexpLabel(name, "^([a-z]+/c[0-9]+d[0-9]+)$");
            }

            if (device != "") {
                //add potential device to the list
                devicePathsCache.push_back("/dev/" + device);
            }
            line = in.readLine();
        }
    }
}

} // namespace DiskManager
