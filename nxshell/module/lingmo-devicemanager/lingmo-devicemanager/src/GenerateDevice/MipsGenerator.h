// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MIPSGENERATOR_H
#define MIPSGENERATOR_H
#include "DeviceGenerator.h"

/**
 * @brief The MipsGenerator class
 * 将获取的设备信息生成设备对象，mips架构下的生成器
 */

class MipsGenerator : public DeviceGenerator
{
public:
    MipsGenerator();

    /**
     * @brief generatorComputerDevice:生成计算机概况
     */
    virtual void generatorComputerDevice() override;
};

#endif // MIPSGENERATOR_H
