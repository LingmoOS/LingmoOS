// Copyright (C) 2019 ~ 2024 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PANGUXGENERATOR_H
#define PANGUXGENERATOR_H

#include "HWGenerator.h"

/**
 * @brief The PanguXGenerator class
 * 将获取的设备信息生成设备对象，panguX下的生成器
 */

class PanguXGenerator : public HWGenerator
{
public:
    PanguXGenerator();

protected:
    /**
     * @brief generatorAudioDevice:生成音频适配器
     */
    virtual void getAudioInfoFromCatAudio() override;
    /**
     * @brief generatorBluetoothDevice:生成蓝牙设备
     */
    virtual void generatorBluetoothDevice() override;
    /**
     * @brief generatorCpuDevice:生成CPU设备
     */
    virtual void generatorCpuDevice() override;
};

#endif // PANGUXGENERATOR_H
