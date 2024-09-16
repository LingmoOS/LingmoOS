// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICEPOWER_H
#define DEVICEPOWER_H
#include"DeviceInfo.h"

/**
 * @brief The DevicePower class
 * 用来描述电源设备的类
 */

class DevicePower : public DeviceBaseInfo
{
    Q_OBJECT
    Q_DISABLE_COPY(DevicePower)
public:
    DevicePower();

  /**
      * @brief setInfoFromTomlOneByOne:设置从toml里面获取的信息
      * @param mapInfo:由toml获取的信息map
      * @return枚举值
      */
   TomlFixMethod setInfoFromTomlOneByOne(const QMap<QString, QString> &mapInfo);
   
    /**
     * @brief setInfoFromUpower:设置从upower获取的电池信息
     * @param mapInfo:由upower --dump获取的信息map
     * @return 布尔值，true-信息设置成功；false-信息设置失败
     */
    bool setInfoFromUpower(const QMap<QString, QString> &mapInfo);

    /**
     * @brief setDaemonInfo:设置从upower获取的与电池有关的守护进程信息
     * @param mapInfo:由upower --dump获取的守护进程信息信息map
     */
    void setDaemonInfo(const QMap<QString, QString> &mapInfo);

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
     * @brief driver:获取驱动名称
     * @return  QString 驱动名称
     */
    const QString &driver()const override;

    /**
     * @brief available
     * @return
     */
    bool available() override;

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
     * @brief loadTableData:加载表头信息
     */
    void loadTableData() override;

private:
    QString             m_Model;                    //<! 【设备类型】
    QString             m_Type;                     //<! 【设备类型，电池还是电源】
    QString             m_SerialNumber;             //<! 【序列号】
    QString             m_ElectricType;             //<! 【电流类型】
    QString             m_MaxPower;                 //<! 【最大功率】
    QString             m_Status;                   //<! 【状态】
    QString             m_Enabled;                  //<! 【是否启用】
    QString             m_HotSwitch;                //<! 【热切换】
    QString             m_Capacity;                 //<! 【容量】
    QString             m_Voltage;                  //<! 【电压】
    QString             m_Slot;                     //<! 【插槽】
    QString             m_DesignCapacity;           //<! 【设计容量】
    QString             m_DesignVoltage;            //<! 【设计电压】
    QString             m_SBDSChemistry;            //<! 【SBDS材料】
    QString             m_SBDSManufactureDate;      //<! 【SBDS制造日期】
    QString             m_SBDSSerialNumber;         //<! 【SBDS序列号】
    QString             m_SBDSVersion;              //<! 【SBDS版本】
    QString             m_Temp;                     //<! 【温度】

};

#endif // DEVICEPOWER_H
