// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#include "partitioninfo.h"

#include <QDBusArgument>

PartitionInfo::PartitionInfo()
    : m_insideExtended(false)
    , m_busy(false)
    , m_fileSystemReadOnly(false)
{
    //inside_extended = busy = fs_readonly = false;
    m_devicePath = m_uuid = m_name = m_path = m_fileSystemLabel = "";
    m_partitionNumber = m_type = m_status = m_alignment = m_fileSystemType = 0;
    m_sectorStart = m_sectorEnd = m_sectorsUsed = m_sectorsUnused = m_sectorsUnallocated = m_significantThreshold = m_freeSpaceBefore = 0;
    m_flag = m_sectorSize = m_fileSystemBlockSize = 0;
    m_vgFlag = LVM_FLAG_NOT_PV;
    // mountpoints.clear();
}

Sector PartitionInfo::getSectorLength() const
{
    if (m_sectorStart >= 0 && m_sectorEnd >= 0)
        return m_sectorEnd - m_sectorStart + 1;
    else
        return -1;
}

Byte_Value PartitionInfo::getByteLength() const
{
    if (getSectorLength() >= 0)
        return getSectorLength() * m_sectorSize;
    else
        return -1;
}

bool PartitionInfo::operator==(const PartitionInfo &info) const
{
    return m_devicePath == info.m_devicePath && m_partitionNumber == info.m_partitionNumber && m_sectorStart == info.m_sectorStart && m_type == info.m_type;
}

QDBusArgument &operator<<(QDBusArgument &argument, const PartitionInfo &info)
{
    argument.beginStructure();
    argument << info.m_devicePath
             << info.m_partitionNumber
             << info.m_type
             << info.m_status
             << info.m_alignment
             << info.m_fileSystemType
             << info.m_uuid
             << info.m_name
             << info.m_sectorStart
             << info.m_sectorEnd
             << info.m_sectorsUsed
             << info.m_sectorsUnused
             << info.m_sectorsUnallocated
             << info.m_significantThreshold
             << info.m_freeSpaceBefore
             << info.m_sectorSize
             << info.m_fileSystemBlockSize
             << info.m_path
             << info.m_fileSystemLabel
             << info.m_insideExtended
             << info.m_busy
             << info.m_fileSystemReadOnly
             << info.m_flag
             << info.m_mountPoints
             << static_cast<int>(info.m_vgFlag)
             << info.m_vgData
             << info.m_fsLimits.min_size
             << info.m_fsLimits.max_size
             << static_cast<int>(info.m_luksFlag)
             << static_cast<int>(info.m_crypt)
             << info.m_tokenList
             << info.m_decryptStr
             << info.m_dmName;;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, PartitionInfo &info)
{
    argument.beginStructure();
    int flag = 0;
    int flag2 = 0;
    int crypt = 0;
    argument >> info.m_devicePath
             >> info.m_partitionNumber
             >> info.m_type
             >> info.m_status
             >> info.m_alignment
             >> info.m_fileSystemType
             >> info.m_uuid
             >> info.m_name
             >> info.m_sectorStart
             >> info.m_sectorEnd
             >> info.m_sectorsUsed
             >> info.m_sectorsUnused
             >> info.m_sectorsUnallocated
             >> info.m_significantThreshold
             >> info.m_freeSpaceBefore
             >> info.m_sectorSize
             >> info.m_fileSystemBlockSize
             >> info.m_path
             >> info.m_fileSystemLabel
             >> info.m_insideExtended
             >> info.m_busy
             >> info.m_fileSystemReadOnly
             >> info.m_flag
             >> info.m_mountPoints
             >> flag
             >> info.m_vgData
             >> info.m_fsLimits.min_size
             >> info.m_fsLimits.max_size
             >> flag2
             >> crypt
             >> info.m_tokenList
             >> info.m_decryptStr
             >> info.m_dmName;
    info.m_vgFlag = static_cast<LVMFlag>(flag);
    info.m_luksFlag = static_cast<LUKSFlag>(flag2);
    info.m_crypt = static_cast<CRYPT_CIPHER>(crypt);
    argument.endStructure();

    return argument;
}
