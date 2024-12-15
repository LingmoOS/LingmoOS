// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include "utils.h"
#include "partitioninfo.h"
#include "lvmstruct.h"
#include "luksstruct.h"
#include <QVector>

//dubs使用自定义数据结构中的成员变量建议初始化，在测试中发现数据结构中包含的bool类型变量未赋值，该数据结构作为槽函数返回值通过dbus调用可能导致应用崩溃退出
class QDBusArgument;

/**
 * @class CUSSTRUCTTEST
 * @brief qdbus测试结构体
 */
typedef struct CUSSTRUCTTEST {
    Sector m_length;
    Sector m_heads;
    QString m_path;
} stCustest;
DBUSStructEnd(stCustest)



//new by li 2020/7/20

/**
 * @class HARDDISKINFO
 * @brief 设备硬件信息结构体
 */
typedef struct HARDDISKINFO {
    QString m_model;              //<! 【型号】1
    QString m_vendor;             //<! 【制造商】2 //有可能会没有
    QString m_mediaType;          //<! 【介质类型】3
    QString m_size;               //<! 【大小】4
    QString m_rotationRate;       //<! 【转速】
    QString m_interface;          //<! 【接口】6
    QString m_serialNumber;       //<! 【序列号】7
    QString m_version;            //<! 【版本】
    QString m_capabilities;       //<! 【功能】
    QString m_description;        //<! 【描述】
    QString m_powerOnHours;       //<! 【通电时间】9
    QString m_powerCycleCount;    //<! 【通电次数】10
    QString m_firmwareVersion;    //<! 【固件版本】8
    QString m_speed;              //<! 【速度】5
} HardDiskInfo;
DBUSStructEnd(HardDiskInfo)



/**
 * @class HARDDISKSTATUSINFO
 * @brief 设备健康信息结构体
 */
typedef struct HARDDISKSTATUSINFO {
    QString m_id;                   //ID
    QString m_attributeName;        //硬盘制造商定义的属性名
    QString m_flag;                 //属性操作标志
    QString m_value;                //当前值
    QString m_worst;                //最差值
    QString m_thresh;               //临界值
    QString m_type;                 //属性的类型（Pre-fail或Oldage）
    QString m_updated;              //表示属性的更新频率
    QString m_whenFailed;           //如果VALUE小于等于THRESH，会被设置成“FAILING_NOW”；如果WORST小于等于THRESH会被设置成“In_the_past”；如果都不是，会被设置成“-”。在“FAILING_NOW”情况下，需要尽快备份重要 文件，特别是属性是Pre-fail类型时。“In_the_past”代表属性已经故障了，但在运行测试的时候没问题。“-”代表这个属性从没故障过。
    QString m_rawValue;             //原始值
    //int m_flag;                   //标记位
} HardDiskStatusInfo;
DBUSStructEnd(HardDiskStatusInfo)



/**
 * @class DeviceInfo
 * @brief 设备信息类
 */
class DeviceInfo
{
public:
    DeviceInfo();

public:
    Sector m_length;
    Sector m_heads;
    QString m_path;
    Sector m_sectors;
    Sector m_cylinders;
    Sector m_cylsize;
    QString m_model;
    QString m_serialNumber;
    QString m_disktype;
    QString m_mediaType;        //介质类型
    QString m_interface;        //接口
    int m_sectorSize;
    int m_maxPrims;
    int m_highestBusy;
    bool m_readonly;
    int m_maxPartitionNameLength;
    PartitionVec m_partition;
    QVector<VGData>m_vglist;
    LVMFlag m_vgFlag;           //vg标志位
    LUKSFlag m_luksFlag;        //luks 标志位
    CRYPT_CIPHER_Support m_crySupport;
};
DBUSStructEnd(DeviceInfo)

typedef QMap<QString, DeviceInfo> DeviceInfoMap;
Q_DECLARE_METATYPE(DeviceInfoMap)

typedef QList<HardDiskStatusInfo> HardDiskStatusInfoList;
Q_DECLARE_METATYPE(HardDiskStatusInfoList)



//new by liuwh 2022/5/17
/**
 * @class WipeAction
 * @brief 分区清除 格式化
 */
struct WipeAction {
    QString m_fstype;
    QString m_path;
    QString m_fileSystemLabel ;
    QString m_user;
    int m_diskType{0};
    int m_clearType{0};
    LUKSFlag m_luksFlag{LUKSFlag::NOT_CRYPT_LUKS};
    CRYPT_CIPHER m_crypt{CRYPT_CIPHER::NOT_CRYPT};
    QStringList m_tokenList;                                //密钥提示   luks####提示信息####(创建分区时该属性有效)
    QString m_decryptStr;                                   //用户解密密码字符串(创建分区时该属性有效)
    QString m_dmName;
};
DBUSStructEnd(WipeAction)



#endif // DEVICEINFO_H
