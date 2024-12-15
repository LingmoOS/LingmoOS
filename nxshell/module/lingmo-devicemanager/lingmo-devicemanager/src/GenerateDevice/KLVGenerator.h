// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KLVGENERATOR_H
#define KLVGENERATOR_H

#include"HWGenerator.h"

/**
 * @brief The KLVGenerator class
 * 将获取的设备信息生成设备对象，KLU笔记本下的生成器
 */
class KLVGenerator : public HWGenerator
{
public:
    KLVGenerator();

protected:
    /**
     * @brief generatorMonitorDevice:生成显示设备
     */
    virtual void generatorMonitorDevice() override;

    /**@brief:generator network info*/
    virtual void generatorNetworkDevice() override;

protected:
    /**@brief:generator network info*/
    virtual void getNetworkInfoFromCatWifiInfo();

    /**
     * @brief getDiskInfoFromLshw:从lshw获取存储设备信息
     */
    virtual void getDiskInfoFromLshw() override;

    /**
     * @brief getImageInfoFromHwinfo:从hwinfo中获取图像设备信息
     */
    virtual void getImageInfoFromHwinfo() override;

    /**
     * @brief getImageInfoFromLshw:从lshw中获取图像设备信息
     */
    virtual void getImageInfoFromLshw() override;
};

#endif // KLVGENERATOR_H
