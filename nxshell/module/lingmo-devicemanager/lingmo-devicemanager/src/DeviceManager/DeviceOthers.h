// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICEOTHERS_H
#define DEVICEOTHERS_H
#include"DeviceInfo.h"

/**
 * @brief The DeviceOthers class
 * 用来描述其它设备的类
 */

class DeviceOthers : public DeviceBaseInfo
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceOthers)
public:
    DeviceOthers();

    /**@brief:设置从lshw获取的信息*/
    void setInfoFromLshw(const QMap<QString, QString> &mapInfo);

      /**
      * @brief setInfoFromTomlOneByOne:设置从toml里面获取的信息
      * @param mapInfo:由toml获取的信息map
      * @return枚举值
      */
   TomlFixMethod setInfoFromTomlOneByOne(const QMap<QString, QString> &mapInfo);
   
    /**@brief:设置从hwinfo获取的信息*/
    void setInfoFromHwinfo(const QMap<QString, QString> &mapInfo);

    /**
     * @brief setEnable 设置设备的禁用状态
     * @param enable 是否禁用
     * @return 禁用是否成功
     */
    EnableDeviceStatus setEnable(bool enable) override;

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
     * @brief busInfo:获取总线信息属性值
     * @return QString:总线信息属性值
     */
    const QString &busInfo()const;


    /**
     * @brief driver:获取驱动属性值
     * @return QString:驱动属性值
     */
    const QString &driver()const override;

    /**
     * @brief logicalName:获取逻辑名称属性值
     * @return QString:逻辑名称属性值
     */
    const QString &logicalName()const;

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
     * @brief availble 获取是否可用
     * @return 返回是否可用
     */
    virtual bool available() override;
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

    QString             m_Model;                        //<! 【型号】
    QString             m_BusInfo;                      //<! 【总线信息】
    QString             m_Capabilities;                 //<! 【功能】
    QString             m_MaximumPower;                 //<! 【最大功耗】
    QString             m_Speed;                        //<! 【速度】
    QString             m_BusID;                        //<! 【总线ID】
    QString             m_LogicalName;
    QString             m_Avail;                        //<! 【Config Status:avail=yes】
};

#endif // DEVICEOTHERS_H
