// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KLUGENERATOR_H
#define KLUGENERATOR_H

#include"HWGenerator.h"

/**
 * @brief The KLUGenerator class
 * 将获取的设备信息生成设备对象，KLU笔记本下的生成器
 */
class KLUGenerator : public HWGenerator
{
public:
    KLUGenerator();

protected:

    /**
     * @brief getDiskInfoFromLshw:从lshw获取存储设备信息
     */
    virtual void getDiskInfoFromLshw() override;

    /**@brief:generator network info*/
    virtual void generatorNetworkDevice() override;

    /**@brief:generator network info*/
    virtual void getNetworkInfoFromCatWifiInfo();
    /**@brief:generatorMonitorDevice:生成显示设备*/
    virtual void generatorMonitorDevice();
};

#endif // KLUGENERATOR_H
