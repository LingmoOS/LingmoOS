// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PANGUVGENERATOR_H
#define PANGUVGENERATOR_H

#include <QObject>
#include "HWGenerator.h"

/**
 * @brief The PanguVGenerator class
 * 将获取的设备信息生成设备对象，panguV下的生成器
 */

class PanguVGenerator : public HWGenerator
{
public:
    PanguVGenerator();

    /**
     * @brief generatorMonitorDevice:生成显示设备信息
     */
    virtual void generatorMonitorDevice() override;
protected:
    /**@brief:generator network info*/
    virtual void generatorNetworkDevice() override;
    /**@brief:generator network info*/
    virtual QStringList getNetworkInfoFromifconfig();
};

#endif // PANGUVGENERATOR_H
