// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef THREAD_H
#define THREAD_H

#include "sigtype.h"
#include "device.h"
#include "deviceinfo.h"
#include <QObject>
#include <parted/parted.h>
#include <parted/device.h>

namespace DiskManager {

/**
 * @class ProbeThread
 * @brief 硬件信息刷新类
 */
class ProbeThread : public QObject
{
    Q_OBJECT
public:
    ProbeThread(QObject *parent = nullptr);


public slots:
    /**
     * @brief 刷新硬件信息
     */
    void probeDeviceInfo();

    /**
     * @brief 返回硬件信息
     */
    QMap<QString, Device> getDeviceMap();
signals:
    /**
     * @brief 更新硬件信息信号
     * @param 要同步的硬件信息
     */
    void updateDeviceInfo(/*const QMap<QString, Device> m_deviceMap, */const DeviceInfoMap infomap, const LVMInfo lvmInfo, const LUKSMap &luks);

private:
    QMap<QString, Device> m_deviceMap; //设备对应信息表
    DeviceInfoMap m_inforesult;        //全部设备分区信息
    LVMInfo m_lvmInfo;                 //lvm 属性信息
    LUKSMap m_luksInfo;                //luks 属性信息
};

/**
 * @class WorkThread
 * @brief 坏道检测类
 */
class WorkThread : public QObject
{
    Q_OBJECT
public:
    WorkThread(QObject *parent = nullptr);

    /**
     * @brief 设置检测次数信息
     * @param devicePath：设备路径
     * @param blockStart：开始柱面信息号
     * @param blockEnd：检测结束柱面号
     * @param checkConut：检测次数
     * @param checkSize：检测柱面范围大小
     */
    void setCountInfo(const QString &devicePath, int blockStart, int blockEnd, int checkConut, int checkSize);

    /**
     * @brief 设置检测超时时间信息
     * @param devicePath：设备路径
     * @param blockStart：开始柱面信息号
     * @param blockEnd：检测结束柱面号
     * @param checkTime：检测超时时间
     * @param checkSize：检测柱面范围大小
     */
    void setTimeInfo(const QString &devicePath, int blockStart, int blockEnd, QString checkTime, int checkSize);

    /**
     * @brief 设置修复数据
     * @param devicePath：设备路径
     * @param list: 修复柱面集合
     * @param checkSize：检测柱面范围大小
     */
    void setFixBadBlocksInfo(const QString &devicePath, QStringList list, int checkSize);

    /**
     * @brief 设置停止状态
     * @param flag：停止状态
     */
    void setStopFlag(int flag);

public slots:

    /**
     * @brief 坏道检测线程(检测次数方式)
     */
    void runCount();

    /**
     * @brief 坏道检测线程(超时时间方式)
     */
    void runTime();

signals:

    /**
     * @brief 坏道检测检测信息信号(次数检测)
     * @param cylinderNumber：检测柱面号
     * @param cylinderTimeConsuming：柱面耗时
     * @param cylinderStatus：柱面状态
     * @param cylinderErrorInfo：柱面错误信息
     */
    void checkBadBlocksInfo(const QString &cylinderNumber, const QString &cylinderTimeConsuming, const QString &cylinderStatus, const QString &cylinderErrorInfo);

    /**
     * @brief 坏道检测完成信号
     */
    void checkBadBlocksFinished();

private:
    QString m_devicePath;   //设备路径
    int m_blockStart;       //开始检测柱面号
    int m_blockEnd;         //检测结束柱面号
    int m_checkConut;       //检测次数
    int m_checkSize;        //检测柱面大小
    QString m_checkTime;    //检测超时时间
    int m_stopFlag;         //暂停状态
};

/**
 * @class fixthread
 * @brief 坏道修复类
 */
class FixThread : public QObject
{
    Q_OBJECT
public:
    FixThread(QObject *parent = nullptr);

    /**
     * @brief 设置停止状态
     * @param flag：停止状态
     */
    void setStopFlag(int flag);

    /**
     * @brief 设置修复数据
     * @param devicePath：设备路径
     * @param list: 修复柱面集合
     * @param checkSize：检测柱面范围大小
     */
    void setFixBadBlocksInfo(const QString &devicePath, QStringList list, int checkSize);

public slots:
    /**
     * @brief 坏道修复线程
     */
    void runFix();

signals:

    /**
     * @brief 坏道修复信息信号
     * @param cylinderNumber：检测柱面号
     * @param cylinderStatus：柱面状态
     */
    void fixBadBlocksInfo(const QString &cylinderNumber, const QString &cylinderStatus, const QString &cylinderTimeConsuming);


    /**
     * @brief 坏道修复完成信号
     */
    void fixBadBlocksFinished();

private:
    QString m_devicePath;   //设备路径
    int m_stopFlag;         //暂停状态
    QStringList m_list;     //需要修复柱面集合
    int m_checkSize;        //检测柱面大小
};

class LVMThread: public QObject
{
    Q_OBJECT
public:
    LVMThread(QObject *parent = nullptr);

    /**
     * @brief 从vg中 删除pv设备 如果vg所有pv都被删除 将删除vg
     * @param lvmInfo:lvm属性结构体
     * @param devList: 待删除pv设备列表
     */
    void deletePVList(LVMInfo lvmInfo, QList<PVData> devList);

    /**
     * @brief vg空间调整
     * @param lvmInfo:lvm属性结构体
     * @param vgName: vg名称
     * @param devList:vg调整后的设备
     * @param size: vg调整后大小 byte字节
     */
    void resizeVG(LVMInfo lvmInfo, QString vgName, QList<PVData>devList, long long size);

signals:

    /**
     * @brief 删除pv完成信号
     * @param flag: 完成结果 true 为成功 false 为失败
     */
    void deletePVListFinished(bool flag);

    /**
     * @brief vg空间调整完成信号
     * @param flag: 完成结果 true 为成功 false 为失败
     */
    void resizeVGFinished(bool);
};

}
#endif // THREAD_H
