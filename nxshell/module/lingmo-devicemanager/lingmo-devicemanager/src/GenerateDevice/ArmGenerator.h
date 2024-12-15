// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ARMGENERATOR_H
#define ARMGENERATOR_H
#include "DeviceGenerator.h"

/**
 * @brief The ArmGenerator class
 * 将获取的设备信息生成设备对象，arm架构下的生成器
 */
class ArmGenerator : public DeviceGenerator
{
public:
    ArmGenerator();
};

#endif // ARMGENERATOR_H
