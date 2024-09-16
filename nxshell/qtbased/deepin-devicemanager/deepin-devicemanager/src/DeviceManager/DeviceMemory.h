// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICEMEMORY_H
#define DEVICEMEMORY_H
#include"DeviceInfo.h"

/**
 * @brief The DeviceMemory class
 * 用来描述内存条的类
 */

class DeviceMemory : public DeviceBaseInfo
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceMemory)
public:
    DeviceMemory();

    /**
     * @brief setInfoFromLshw:设置从 sudo lshw -C memory 获取的内存信息
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
     * @brief setInfoFromDmidecode:设置从 sudo dmideocde -t memory 获取的内存信息
     * @param mapInfo:由dmideocde获取的信息map
     * @return 布尔值，true:信息设置成功；false:信息设置失败
     */
    bool setInfoFromDmidecode(const QMap<QString, QString> &mapInfo);

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
     * @brief name:获取驱动属性值
     * @return QString 驱动属性值
     */
    const QString &driver()const override;

    /**
     * @brief available 返回是否可用
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
     * @brief loadTableHeader : 过去表格的表头数据
     */
    void loadTableHeader() override;

    /**
     * @brief loadTableData:加载表头信息
     */
    void loadTableData() override;

private:

    QString           m_Model;                  //<! 【型号】
    QString           m_Size;                   //<! 【大小】
    QString           m_Type;                   //<! 【类型】
    QString           m_Speed;                  //<! 【速度】
    QString           m_TotalBandwidth;         //<! 【总带宽】
    QString           m_DataBandwidth;          //<! 【数据带宽】
    QString           m_Locator;                //<! 【内存槽位】
    QString           m_SerialNumber;           //<! 【序列号】
    QString           m_ConfiguredSpeed;        //<! 【配置速度】
    QString           m_MinimumVoltage;         //<! 【最小电压】
    QString           m_MaximumVoltage;         //<! 【最大电压】
    QString           m_ConfiguredVoltage;      //<! 【配置电压】
    bool              m_MatchedFromDmi;         //<! 标识是否已经从dmidecode匹配过
};

#endif // DEVICEMEMORY_H
