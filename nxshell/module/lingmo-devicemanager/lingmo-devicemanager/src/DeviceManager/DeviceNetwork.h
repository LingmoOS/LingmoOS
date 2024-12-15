// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICENETWORK_H
#define DEVICENETWORK_H
#include"DeviceInfo.h"

/**
 * @brief The DeviceNetwork class
 * 用来描述网络适配器的类
 */

class DeviceNetwork : public DeviceBaseInfo
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceNetwork)
public:
    DeviceNetwork();

    /**
     * @brief setInfoFromLshw:设置从lshw里面获取的信息
     * @param mapInfo:由lshw获取的信息map
     */
    void setInfoFromLshw(const QMap<QString, QString> &mapInfo);

    /**
    * @brief setInfoFromTomlOneByOne:设置从toml里面获取的信息
    * @param mapInfo:由toml获取的信息map
    * @return枚举值
    */
    TomlFixMethod setInfoFromTomlOneByOne(const QMap<QString, QString> &mapInfo);

    /**
     * @brief setInfoFromHwinfo:设置由hwinfo --network命令获取的设备信息
     * @param mapInfo:由hwinfo获取的信息map
     * @return 布尔值，true:信息设置成功；false:信息设置失败
     */
    bool setInfoFromHwinfo(const QMap<QString, QString> &mapInfo);

    /**
     * @brief setInfoFromWifiInfo:设置从cat /sys/hisys/wal/wifi_devices_info里面获取的信息
     * @param mapInfo: 由cat /sys/hisys/wal/wifi_devices_info获取的信息map
     * @return 布尔值，true：信息设置成功；false：信息设置失败
     */
    bool setInfoFromWifiInfo(const QMap<QString, QString> &mapInfo);

    /**
     * @brief setIsWireless: 设置是否是无线网
     * @param sysFSID: SysFS ID: /class/net/enp2s0
     */
    void setIsWireless(const QString &sysfs);

    /**
     * @brief name:获取名称属性值
     * @return QString 名称属性值
     */
    const QString &name()const override;

    /**
     * @brief name:获取制造商属性值
     * @return QString 制造商属性值
     */
    const QString &vendor()const override;

    /**
     * @brief driver:获取驱动属性值
     * @return QString:驱动属性值
     */
    const QString &driver()const override;

    /**
     * @brief subTitle:获取子标题
     * @return 子标题
     */
    QString subTitle() override;

    /**
     * @brief getOverviewInfo:获取概况信息
     * @return 概况信息
     */
    const QString getOverviewInfo() override;

    /**
     * @brief setEnable 设置设备的禁用状态
     * @param enable 是否禁用
     * @return 禁用是否成功
     */
    EnableDeviceStatus setEnable(bool enable) override;

    /**
     * @brief enable : 判断当前是否是禁用状态
     * @return
     */
    bool enable() override;

    /**
     * @brief correctCurrentLinkStatus
     * @param linkStatus
     */
    void correctCurrentLinkStatus(QString linkStatus);

    /**
     * @brief logicalName: 获取网卡逻辑名称
     * @return
     */
    QString logicalName();

    /**
     * @brief isWireless: 获取是否是无线网
     * @return
     */
    bool isWireless();

    /**
     * @brief hwAddress: 获取HW地址
     * @return HW地址
     */
    QString hwAddress();

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
     * @brief loadTableHeader : 过去表格的表头数据
     */
    void loadTableHeader() override;

    /**
     * @brief loadTableData:加载表头信息
     */
    void loadTableData() override;

private:

    QString             m_Model;                        //<! 【型号】
    QString             m_BusInfo;                      //<! 【总线信息】
    QString             m_LogicalName;                  //<! 【逻辑名称】
    QString             m_MACAddress;                   //<! 【物理地址】
    QString             m_Irq;                          //<! 【中断】
    QString             m_Memory;                       //<! 【内存地址】
    QString             m_Width;                        //<! 【位宽】
    QString             m_Clock;                        //<! 【时钟频率】
    QString             m_Capabilities;                 //<! 【功能】
    QString             m_Autonegotiation;              //<! 【自动协商】
    QString             m_Broadcast;                    //<! 【广播】
    QString             m_DriverModules;                //<!  driver modules
    QString             m_DriverVersion;                //<! 【驱动】
    QString             m_Duplex;                       //<! 【双工】
    QString             m_Firmware;                     //<! 【固件】
    QString             m_Port;                         //<! 【port】
    QString             m_Link;                         //<! 【链接】
    QString             m_Ip;                           //<! 【网址】
    QString             m_Speed;                        //<! 【速度】
    QString             m_Capacity;                     //<! 【最大速度】
    QString             m_Latency;                      //<! 【延迟】
    QString             m_Multicast;                    //<! 【组播】

    bool                m_IsWireless;                   //<! 【是否是无线网】
};

#endif // DEVICENETWORK_H
