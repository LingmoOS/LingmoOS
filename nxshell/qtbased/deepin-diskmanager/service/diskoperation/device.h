// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef DEVICE_H
#define DEVICE_H
#include "deviceinfo.h"
#include "partition.h"

#include <QtDBus/QtDBus>
#include <QVector>

namespace DiskManager {


/**
 * @class Device
 * @brief 设备信息类
 */

class Device
{
public:
    Device();

    /**
     * @brief 启用分区命名
     * @param length:长度
     */
    void enablePartitionNaming(int length); // > 0 => enable partition naming support

    /**
     * @brief 支持分区命名
     * @return true成功false失败
     */
    bool partitionNamingSupported() const;

//    /**
//     * @brief 获得最大分区命名长度
//     * @return 最大分区命名长度
//     */
//    int getMaxPartitionNameLength() const;

    /**
     * @brief 获得设备信息
     * @return 设备信息
     */
    DeviceInfo getDeviceInfo();

public:
    Sector m_length;        //长度
    Sector m_heads;           //扇区头
    QString m_path;         //路径
    Sector m_sectors;         //扇区
    Sector m_cylinders;       //柱面
    Sector m_cylsize;         //柱面大小
    QString m_model;          //模型
    QString m_serialNumber;  //序列号
    QString m_diskType;       //磁盘类型
    QString m_mediaType;        //介质类型
    QString m_interface;                //接口
    int m_sectorSize;        //扇区大小
    int m_maxPrims;          //最大分区个数
    int m_highestBusy;       //挂载
    bool m_readonly;          //是否只读
    QVector<Partition *> m_partitions; //分区信息
private:
    int m_maxPartitionNameLength; //最大分区命名长度
};

} //namespace DiskManager
#endif // DEVICE_H
