// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HWGENERATOR_H
#define HWGENERATOR_H

#include <QObject>
#include "DeviceGenerator.h"

/**
 * @brief The PanguVGenerator class
 * 将获取的设备信息生成设备对象，HW下的生成器
 */

class HWGenerator : public DeviceGenerator
{
public:
    HWGenerator();

    /**
     * @brief generatorComputerDevice:生成计算机概况
     */
    virtual void generatorComputerDevice() override;

    /**
     * @brief generatorCpuDevice:生成CPU设备
     */
    virtual void generatorCpuDevice() override;

    /**
     * @brief generatorAudioDevice:生成音频适配器
     */
    virtual void generatorAudioDevice() override;

    /**
     * @brief generatorBluetoothDevice:生成蓝牙设备
     */
    virtual void generatorBluetoothDevice() override;

    /**
     * @brief generatorGpuDevice:生成显卡信息
     */
    virtual void generatorGpuDevice() override;

    /**
     * @brief generatorNetworkDevice:生成网卡
     */
    virtual void generatorNetworkDevice() override;

    /**
     * @brief generatorDiskDevice生成存储设备信息
     */
    virtual void generatorDiskDevice() override;

protected:
    /**
     * @brief getAudioInfoFromCatInput:从cat /proc/bus/input/devices获取音频设备信息
     */
    virtual void getAudioInfoFromCatAudio() ;

    /**
     * @brief getDiskInfoFromLshw:从lshw获取存储设备信息
     */
    virtual void getDiskInfoFromLshw() override;

    /**
     * @brief getDiskInfoFromSmartCtl:从smartctl获取存储设备信息
     */
    virtual void getDiskInfoFromSmartCtl() override;

    /**
     * @brief getBluetoothInfoFromHciconfig:hciconfig获取蓝牙信息
     */
    virtual void getBluetoothInfoFromHciconfig() override;

    /**
     * @brief getBlueToothInfoFromHwinfo:从hwinfo获取蓝牙信息
     */
    virtual void getBlueToothInfoFromHwinfo() override;

    /**
     * @brief getBluetoothInfoFromLshw:从lshw获取蓝牙信息
     */
    virtual void getBluetoothInfoFromLshw() override;

    /**@brief:generator bluetooth info*/
    virtual void getBluetoothInfoFromCatWifiInfo();

    /**
     * @brief getMemoryInfoFromLshw:从lshw获取内存信息
     */
    virtual void getMemoryInfoFromLshw() override;

    /**
     * @brief generatorMonitorDevice:生成显示设备信息
     */
    virtual void generatorMonitorDevice() override;
};

#endif // PANGUVGENERATOR_H
