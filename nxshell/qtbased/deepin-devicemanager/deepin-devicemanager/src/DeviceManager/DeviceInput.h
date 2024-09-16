// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICEINPUT_H
#define DEVICEINPUT_H

#include "DeviceInfo.h"

/**
 * @brief The DeviceInput class
 * 用来描述输入设备的类
 */
class DeviceInput : public DeviceBaseInfo
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceInput)

public:
    DeviceInput();

    /**
     * @brief setInfoFromlshw:设置从lshw里面获取的信息
     * @param mapInfo:由lshw获取的信息map
     * @return 布尔值，true:信息设置成功；false:信息设置失败
     */
    bool setInfoFromlshw(const QMap<QString, QString> &mapInfo);

  /**
      * @brief setInfoFromTomlOneByOne:设置从toml里面获取的信息
      * @param mapInfo:由toml获取的信息map
      * @return枚举值
      */
   TomlFixMethod setInfoFromTomlOneByOne(const QMap<QString, QString> &mapInfo);
   
    /**
     * @brief setInfoFromHwinfo:设置由hwinfo --keyboard 命令获取的设备信息
     * @param mapInfo:由hwinfo获取的信息map
     */
    void setInfoFromHwinfo(const QMap<QString, QString> &mapInfo);

    /**
     * @brief setKLUInfoFromHwinfo:特殊处理KLU设备,设置由hwinfo --keyboard 命令获取的设备信息
     * @param mapInfo由hwinfo获取的信息map
     */
    void setKLUInfoFromHwinfo(const QMap<QString, QString> &mapInfo);

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
     * @brief name:获取名称属性值
     * @return QString 名称属性值
     */
    const QString &driver()const override;

    /**
     * @brief available 判断是否可用
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

    /**
     * @brief setEnable : 设置禁用
     * @param enable : 启用禁用
     * @return 返回操作是否成功
     */
    EnableDeviceStatus setEnable(bool enable)override;

    /**
     * @brief enable : 判断当前是否是禁用状态
     * @return
     */
    bool enable() override;

    /**
     * @brief canWakeupMachine
     * @return
     */
    bool canWakeupMachine();

    /**
     * @brief isWakeupMachine
     * @return
     */
    bool isWakeupMachine();

    /**
     * @brief wakeupPath
     * @return
     */
    QString wakeupPath();

    /**
     * @brief wakeID
     * @return
     */
    const QString &wakeupID();

    /**
     * @brief bluetoothIsConnected 获取蓝牙的连接状态
     * @return
     */
    bool bluetoothIsConnected();

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

    /**
     * @brief setInfoFromBluetoothctl:由bluetoothctl paired-devices设置设备接口
     */
    void setInfoFromBluetoothctl();

    /**
     * @brief getPS2Syspath 获取ps2鼠标键盘的syspath
     * @param dfs : Device Files 属性
     */
    bool getPS2Syspath(const QString &dfs);

    /**
     * @brief isBluetoothDevice 判断是否是蓝牙设备
     * @param dfs 设备文件
     * @return
     */
    bool isBluetoothDevice(const QString &dfs);

    /**
     * @brief eventStrFromDeviceFiles 从Device Files中获取event字符串
     * @param dfs : Device Files 属性
     * @return
     */
    QString eventStrFromDeviceFiles(const QString &dfs);

private:

    QString             m_Model;                        //<! 【型号】
    QString             m_Interface;                    //<! 【接口】
    QString             m_BusInfo;                      //<! 【总线信息】
    QString             m_Capabilities;                 //<! 【功能】
    QString             m_MaximumPower;                 //<! 【最大功耗】
    QString             m_Speed;                        //<! 【速度】
    QString             m_KeyToLshw;                    //<!
    QString             m_WakeupID;                     //<!  wakeup id
    bool                m_BluetoothIsConnected;         //<!  记录蓝牙的连接状态
    bool                m_wakeupChanged = true;                //<!   记录鼠标的唤醒状态

    QString             m_keysToPairedDevice;           //<! 【用来标识蓝牙键盘】
    
};

#endif // DEVICEINPUT_H
