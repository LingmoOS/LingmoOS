// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICECPU_H
#define DEVICECPU_H
#include "DeviceInfo.h"

/**
 * @brief The DeviceCpu class
 * 用来描述CPU的类
 */
class DeviceCpu : public DeviceBaseInfo
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceCpu)
public:
    DeviceCpu();

    /**
     * @brief setCpuInfo:设置cpu信息
     * @param mapLscpu:由lscpu获取的信息map
     * @param mapLshw:由lshw获取的信息map
     * @param mapDmidecode:由dmidecode -t 4获取的信息的map
     * @param catInfo:由cat /proc/cpuinfo获取的信息
     */
    void setCpuInfo(const QMap<QString, QString> &mapLscpu, const QMap<QString, QString> &mapLshw, const QMap<QString, QString> &mapDmidecode, int coreNum, int logicalNum);
    /**
     * @brief vendor:获取制造商属性值
     * @return QString:制造商属性值
     */
    const QString &vendor() const override;

    /**
     * @brief name:获取名称属性值
     * @return QString:名称属性值
     */
    const QString &name() const override;

    /**
     * @brief driver 返回驱动，重写纯虚函数
     * @return
     */
    const QString &driver() const override;

    /**
     * @brief available 返回是否可用
     * @return
     */
    bool available() override;

    /**
     * @brief frequencyIsRange:频率是否是范围
     * @return 布尔值:true-频率显示范围;false-频率不显示范围
     */
    bool frequencyIsRange()const;

    /**
     * @brief subTitle : 详细信息里面的子标题
     * @return
     */
    QString subTitle() override;

    /**
     * @brief getOverviewInfo:获取概况信息
     * @return 概况信息
     */
    const QString getOverviewInfo() override;

    /**
     * @brief setInfoFromLscpu:设置由lscpu获取的CPU信息
     * @param mapInfo:由lscpu获取的CPU信息map
     */
    void setInfoFromLscpu(const QMap<QString, QString> &mapInfo);

    /**
     * @brief setCurFreq
     * @param curFreq
     */
    void setCurFreq(const QString &curFreq);

    /**
     * @brief setFrequencyIsCur:设置频率显示是当前还是最大值
     * @param flag:频率显示是当前还是最大值
     */
    void setFrequencyIsCur(const bool &flag);

    /**
        * @brief setInfoFromTomlOneByOne:设置从toml里面获取的信息
        * @param mapInfo:由toml获取的信息map
        * @return枚举值
        */
    TomlFixMethod setInfoFromTomlOneByOne(const QMap<QString, QString> &mapInfo);

protected:

    /**
     * @brief initFilterKey:初始化可现实的可显示的属性,m_FilterKey
     */
    void initFilterKey() override;

    /**
     * @brief loadBaseDeviceInfo:加载基本信息
     */
    void loadBaseDeviceInfo() override;

    /**
     * @brief loadOtherDeviceInfo:加载基本信息
     */
    void loadOtherDeviceInfo() override;

    /**
     * @brief loadTableHeader : 加载表格的头部
     */
    void loadTableHeader() override;

    /**
     * @brief loadTableData:加载表格的内容
     */
    void loadTableData() override;

    void getTrNumber();

private:

    /**
     * @brief setInfoFromLshw:设置由lshw获取的CPU信息
     * @param mapInfo:由lshw获取的CPU信息map
     */
    void setInfoFromLshw(const QMap<QString, QString> &mapInfo);

    /**
     * @brief setInfoFromDmidecode:设置由dmidecode -t 4获取的CPU信息
     * @param mapInfo:由dmidecode -t 4获取的CPU信息map
     */
    void setInfoFromDmidecode(const QMap<QString, QString> &mapInfo);

    /**
     * @brief setRefreshInfoFromLscpu:刷新时重新获取cpu信息
     * @param mapInfo:由lscpu获取的信息
     */
    void setRefreshInfoFromLscpu(const QMap<QString, QString> &mapInfo);

private:

    QString           m_PhysicalID;         //<! 处理器ID
    QString           m_CoreID;             //<! 核心ID
    QString           m_ThreadNum;          //<! 线程数
    QString           m_Frequency;          //<! 频率
    QString           m_CurFrequency;       //<! 当前频率
    QString           m_MaxFrequency;       //<! 最大频率
    QString           m_BogoMIPS;           //<! BogoMIPS
    QString           m_Architecture;       //<! 架构
    QString           m_Familly;            //<! 家族
    QString           m_Model;              //<! 型号
    QString           m_Step;               //<! 步进
    QString           m_CacheL1Data;        //<! L1缓存（数据）
    QString           m_CacheL1Order;       //<! L1缓存（指令）
    QString           m_CacheL2;            //<! L2缓存
    QString           m_CacheL3;            //<! L3缓存
    QString           m_CacheL4;            //<! L4缓存
    QString           m_Extensions;         //<! 扩展指令集
    QString           m_Flags;              //<! 特性
    QString           m_HardwareVirtual;    //<! 硬件虚拟化

    // 这个是cpu的逻辑处理器个数，而不是core的逻辑处理器个数
    int               m_LogicalCPUNum;      //<! 逻辑处理器个数
    int               m_CPUCoreNum;         //<! cpu核数
    bool              m_FrequencyIsRange;   //<! 频率显示的是范围还是最大值
    bool              m_FrequencyIsCur;     //<! 频率显示是当前还是最大值

    QMap<int, QString>        m_trNumber;
};

#endif // DEVICECPU_H
