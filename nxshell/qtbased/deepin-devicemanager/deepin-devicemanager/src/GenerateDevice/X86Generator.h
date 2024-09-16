// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef X86GENERATOR_H
#define X86GENERATOR_H

#include "DeviceGenerator.h"

/**
 * @brief The X86Generator class
 * 将获取的设备信息生成设备对象，x86架构下的生成器
 */

class X86Generator : public DeviceGenerator
{
public:
    X86Generator();
};

#endif // X86GENERATOR_H
