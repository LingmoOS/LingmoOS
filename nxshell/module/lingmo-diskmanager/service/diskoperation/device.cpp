// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "device.h"

#include <QDebug>

namespace DiskManager {

Device::Device()
{
    qDBusRegisterMetaType<DeviceInfo>();
    qDBusRegisterMetaType<DeviceInfoMap>();
    qDBusRegisterMetaType<PartitionInfo>();
    qDBusRegisterMetaType<PartitionVec>();
    qDBusRegisterMetaType<stCustest>();
    qDBusRegisterMetaType<HardDiskInfo>();
    qDBusRegisterMetaType<HardDiskStatusInfo>();
    qDBusRegisterMetaType<HardDiskStatusInfoList>();

    //注册结构体 lvm dbus通信使用
    qDBusRegisterMetaType<PVRanges>();
    qDBusRegisterMetaType<LVAction>();
    qDBusRegisterMetaType<QVector<PVRanges>>();
    qDBusRegisterMetaType<VGDATA>();
    qDBusRegisterMetaType<PVData>();
    qDBusRegisterMetaType<LVDATA>();
    qDBusRegisterMetaType<LVInfo>();
    qDBusRegisterMetaType<VGInfo>();
    qDBusRegisterMetaType<PVInfo>();
    qDBusRegisterMetaType<LVMInfo>();
    qDBusRegisterMetaType<QVector<QString>>();
    qDBusRegisterMetaType<QList<PVData>>();
    qDBusRegisterMetaType<QList<LVAction>>();

    //注册结构体 luks dbus通信使用
    qDBusRegisterMetaType<LUKS_INFO>();
    qDBusRegisterMetaType<LUKS_MapperInfo>();
    qDBusRegisterMetaType<CRYPT_CIPHER_Support>();
    qDBusRegisterMetaType<LUKSInfoMap>();
    qDBusRegisterMetaType<LUKSMap>();
    qDBusRegisterMetaType<WipeAction>();

    m_sectorSize = 0;
    m_maxPrims = 0;
    m_highestBusy = 0;
    m_readonly = false;
    m_maxPartitionNameLength = 0;
}

void Device::enablePartitionNaming(int length)
{
    if (length > 0) {
        m_maxPartitionNameLength = length;
    } else {
        m_maxPartitionNameLength = 0;
    }
}

bool Device::partitionNamingSupported() const
{
    return m_maxPartitionNameLength > 0;
}

//int Device::getMaxPartitionNameLength() const
//{
//    return m_maxPartitionNameLength;
//}

DeviceInfo Device::getDeviceInfo()
{
    DeviceInfo info;
    info.m_length = m_length;
    info.m_heads = m_heads;
    info.m_path = m_path;
    info.m_sectors = m_sectors;
    info.m_cylinders = m_cylinders;
    info.m_cylsize = m_cylsize;
    info.m_model = m_model;
    info.m_serialNumber = m_serialNumber;
    info.m_disktype = m_diskType;
    info.m_sectorSize = m_sectorSize;
    info.m_maxPrims = m_maxPrims;
    info.m_highestBusy = m_highestBusy;
    info.m_maxPartitionNameLength = m_maxPartitionNameLength;
    info.m_interface = m_interface;
    info.m_mediaType = m_mediaType;

//        qDebug() << __FUNCTION__ << info.m_path << info.length << info.heads << info.sectors
//                 << info.cylinders << info.cylsize << info.model << info.serial_number << info.disktype
//                 << info.sector_size << info.max_prims << info.highest_busy << info.readonly
//                 << info.max_partition_name_length;
    return info;
}

} // namespace DiskManager
