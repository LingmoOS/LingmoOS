// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICEFACTORY_H
#define DEVICEFACTORY_H

#include<QString>
class DeviceGenerator;

/**
 * @brief The DeviceFactory class
 * 用于获取当前的系统架构，并生成对应的生成器
 */

class DeviceFactory
{
public:
    /**
     * @brief getDeviceGenerator:获取设备生成器指针
     * @return  设备生成器指针:
     */
    static DeviceGenerator *getDeviceGenerator();

protected:
    DeviceFactory();
};

#endif // DEVICEFACTORY_H
