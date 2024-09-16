// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef PARTITIONINFO_H
#define PARTITIONINFO_H
#include "commondef.h"
#include "lvmstruct.h"
#include "luksstruct.h"

/**
 * @class PartitionInfo
 * @brief 分区信息类
 */

class PartitionInfo
{
public:
    PartitionInfo();

    /**
     * @brief 获取扇区长度
     * @return 扇区长度
     */
    Sector getSectorLength() const;

    /**
     * @brief 获取字节长度
     * @return 字节长度
     */
    Byte_Value getByteLength() const;

    bool operator==(const PartitionInfo &info) const;

public:

    QString m_devicePath;
    int m_partitionNumber;
    int m_type; //PartitionType: UNALLOCATED, PRIMARY, LOGICAL, etc...
    int m_status; //PartitionStatus: STAT_REAL, STAT_NEW, etc..
    int m_alignment; //PartitionAlignment: ALIGN_CYLINDER, ALIGN_STRICT, etc
    int m_fileSystemType;
    int m_flag;     //hidden flag
    QString m_uuid;
    QString m_name;
    Sector m_sectorStart;
    Sector m_sectorEnd;
    Sector m_sectorsUsed;
    Sector m_sectorsUnused;
    Sector m_sectorsUnallocated; //Difference between the size of the partition and the file system
    Sector m_significantThreshold; //Threshold from intrinsic to significant unallocated sectors
    Sector m_freeSpaceBefore; //Free space preceding partition value
    Byte_Value m_sectorSize; //Sector size of the disk device needed for converting to/from sectors and bytes.
    Byte_Value m_fileSystemBlockSize; // Block size of of the file system, or -1 when unknown.
    QString m_path; //partition path
    QString m_fileSystemLabel;
    bool m_insideExtended;
    bool m_busy;
    bool m_fileSystemReadOnly;
    QVector<QString> m_mountPoints;
    LVMFlag m_vgFlag; //lv是否加入标志位
    VGData m_vgData;//vg数据 lv信息可从vg中读取
    FS_Limits m_fsLimits; //分区上文件系统限制 该属性在没有文件系统存在时无效

    LUKSFlag m_luksFlag{LUKSFlag::NOT_CRYPT_LUKS};          //创建分区时 该属性为确认是否创建加密  其余情况为确认该分区是否为加密分区
    CRYPT_CIPHER m_crypt{CRYPT_CIPHER::NOT_CRYPT};          //加密算法(创建分区时该属性有效)
    QStringList m_tokenList;                                //密钥提示   luks####提示信息####(创建分区时该属性有效)
    QString m_decryptStr;                                   //用户解密密码字符串(创建分区时该属性有效)
    QString m_dmName;
};
DBUSStructEnd(PartitionInfo)

typedef QVector<PartitionInfo> PartitionVec;
Q_DECLARE_METATYPE(PartitionVec)

#endif // PARTITIONINFO_H
