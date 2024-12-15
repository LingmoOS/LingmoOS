// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICEAUDIO_H
#define DEVICEAUDIO_H
#include"DeviceInfo.h"

/**
 * @brief The DeviceAudio class
 * 用来描述音频适配器的类
 */
class DeviceAudio : public DeviceBaseInfo
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceAudio)
public:
    DeviceAudio();

    /**
       * @brief setInfoFromHwinfo:设置由hwinfo --sound命令获取的设备信息
       * @param mapInfo:由hwinfo获取的信息map
       */
    bool  setInfoFromHwinfo(const QMap<QString, QString> &mapInfo);

    /**
       * @brief setInfoFromLshw:设置从lshw里面获取的信息
       * @param mapInfo:由lshw获取的信息map
       * @return 布尔值，true:信息设置成功；false:信息设置失败
       */
    bool setInfoFromLshw(const QMap<QString, QString> &mapInfo);

   /**
      * @brief setInfoFromTomlOneByOne:设置从toml里面获取的信息
      * @param mapInfo:由toml获取的信息map
      * @return枚举值
      */
   TomlFixMethod setInfoFromTomlOneByOne(const QMap<QString, QString> &mapInfo);

    /**
       * @brief setInfoFromCatDevices:设置从cat /proc/bus/input/devices里面获取的信息
       * @param mapInfo:由cat /proc/bus/input/devices获取的信息map
       * @return 布尔值，true:信息设置成功；false:信息设置失败
       */
    bool setInfoFromCatDevices(const QMap<QString, QString> &mapInfo);

    /**
       * @brief setInfoFrom_sysFS:设置从cat /sys/class/sound里面获取的信息
       * @param mapInfo:由cat /sys/class/sound获取的信息map
       * @param ii: /sys/class/sound /card0  card1 ...n ,i-=1~n
       * @return 布尔值，true:获取设备信息成功；false:获取设备信息失败
       */
    bool setInfoFrom_sysFS(QMap<QString, QString> &mapInfo, int ii);

    /**
       * @brief setInfoFromCatAudio:设置从cat /proc/asound/card0/codec#0里面获取的信息 , KLU专用
       * @param mapInfo:由cat /proc/asound/card0/codec#0获取的信息map
       */
    void setInfoFromCatAudio(const QMap<QString, QString> &mapInfo);

    /**
       * @brief setAudioChipFromDmesg:设置从dmesg里面获取的有关声卡型号信息
       * @param mapInfo:由dmesg获取的有关声卡型号信息
       */
    bool setAudioChipFromDmesg(const QString &info);

    /**
       * @brief chip_name:获取名称属性值
       * @return QString:名称属性值
       */
    const QString &chip_name()const;

    /**
       * @brief name:获取名称属性值
       * @return QString:名称属性值
       */
    const QString &name()const override;

    /**
     * @brief name:获取制造商属性值
     * @return QString 制造商属性值
     */
    const QString &vendor()const override;

    /**
     * @brief driver
     * @return
     */
    const QString &driver()const override;

    /**
     * @brief uniqueID 获取唯一标识
     * @return
     */
    const QString &uniqueID() const;

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

    QString             m_Model;                        //<! 【型号】
    QString             m_BusInfo;                      //<! 【总线信息】
    QString             m_Irq;                          //<! 【中断】
    QString             m_Memory;                       //<! 【内存地址】
    QString             m_Width;                        //<! 【位宽】
    QString             m_Clock;                        //<! 【时钟频率】
    QString             m_Capabilities;                 //<! 【功能】
    QString             m_Chip;                         //<! 【芯片型号】
    QString             m_DriverModules;                //<!  driver modules
    bool                m_IsCatDevice;                  //<!  标识该设备是从cat /input/devices里面获取的
};

#endif // DEVICEAUDIO_H
