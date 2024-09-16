// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICEGPU_H
#define DEVICEGPU_H
#include"DeviceInfo.h"

/**
 * @brief The DeviceGpu class
 * 用来描述显示适配器的类
 */
class DeviceGpu: public DeviceBaseInfo
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceGpu)
public:
    DeviceGpu();

    /**
     * @brief setLshwInfo:设置从lshw里面获取的信息
     * @param mapInfo:由lshw获取的信息map
     */
    void setLshwInfo(const QMap<QString, QString> &mapInfo);
    
  /**
      * @brief setInfoFromTomlOneByOne:设置从toml里面获取的信息
      * @param mapInfo:由toml获取的信息map
      * @return枚举值
      */
   TomlFixMethod setInfoFromTomlOneByOne(const QMap<QString, QString> &mapInfo);

    /**
     * @brief setHwinfoInfo:设置由hwinfo --display命令获取的设备信息
     * @param mapInfo:由hwinfo获取的信息map
     * @return 布尔值，true:信息设置成功；false:信息设置失败
     */
    bool setHwinfoInfo(const QMap<QString, QString> &mapInfo);

    /**
     * @brief setXrandrInfo:设置从xrandr中获取的信息
     * @param mapInfo:由xrandr获取的信息map
     */
    void setXrandrInfo(const QMap<QString, QString> &mapInfo);

    /**
     * @brief setDmesgInfo:设置从dmesg中获取的显存信息
     * @param mapInfo: dmesg中获取的显存信息
     */
    void setDmesgInfo(const QMap<QString, QString> &mapInfo);

    /**
       * @brief setGpuInfo:设置从gpuinfo命令中获取的信息，华为KLU笔记本和PanGuV机器专用
       * @param mapInfo:由gpuinfo获取的信息map
       */
    void setGpuInfo(const QMap<QString, QString> &mapInfo);

    /**
     * @brief name:获取名称属性值
     * @return QString 名称属性值
     */
    const QString &name() const override;

    /**
     * @brief name:获取制造商属性值
     * @return QString 制造商属性值
     */
    const QString &vendor()const override;

    /**
     * @brief driver:获取驱动属性值
     * @return QString:驱动属性值
     */
    const QString &driver() const override;

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
    QString            m_Model;                   //<! 【型号】  
    QString            m_GraphicsMemory;          //<! 【显存】
    QString            m_Width;                   //<! 【位宽】
    QString            m_DisplayPort;             //<! 【DisplayPort】
    QString            m_Clock;                   //<! 【时钟】
    QString            m_IRQ;                     //<! 【中断】
    QString            m_Capabilities;            //<! 【功能】
    QString            m_DisplayOutput;           //<! 【显示输出】
    QString            m_VGA;                     //<! 【VGA】
    QString            m_HDMI;                    //<! 【HDMI】
    QString            m_eDP;
    QString            m_DVI;
    QString            m_Digital;                 // bug-105482添加新接口类型
    QString            m_CurrentResolution;       //<! 【当前分辨率】
    QString            m_MinimumResolution;       //<! 【最小分辨率】
    QString            m_MaximumResolution;       //<! 【最大分辨率】
    QString            m_Type;                    //<! 【类型】
    QString            m_BusInfo;                 //<! 【总线信息】
    QString            m_IOPort;                  //<! 【ioport】
    QString            m_MemAddress;              //<! 【内存地址】
    // QString            m_PhysID;                  //<! 【物理ID】
};

#endif // DEVICEGPU_H
