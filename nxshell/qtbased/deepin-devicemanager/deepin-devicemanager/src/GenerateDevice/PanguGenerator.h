// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PANGUGENERATOR_H
#define PANGUGENERATOR_H
#include"DeviceGenerator.h"

/**
 * @brief The PanguGenerator class
 * 将获取的设备信息生成设备对象，pangu下的生成器
 */

class PanguGenerator : public DeviceGenerator
{
public:
    PanguGenerator();

    /**
     * @brief generatorComputerDevice:生成概况设备信息
     */
    virtual void generatorComputerDevice() override;
};

#endif // PANGUGENERATOR_H
