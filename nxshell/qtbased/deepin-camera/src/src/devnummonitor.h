// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVNUMMONITOR_H
#define DEVNUMMONITOR_H

#include <QThread>
#include <QTimer>

#ifdef __cplusplus
extern "C" {
#endif
#include "LPF_V4L2.h"
#include "v4l2_core.h"
#include"v4l2_devices.h"
#include "stdlib.h"
#include "malloc.h"
#include "stdio.h"
#include "math.h"

#ifdef __cplusplus
}
#endif

/**
* @brief DevNumMonitor　设备数目监视器
*/
class DevNumMonitor: public QObject
{
    Q_OBJECT
public:
    explicit DevNumMonitor();

    ~DevNumMonitor();

    /**
    * @brief startCheck 开始检测
    */
    void startCheck();
signals:
    /**
    * @brief seltBtnStateEnable 相机选择按钮可用信号
    */
    void seltBtnStateEnable();

    /**
    * @brief seltBtnStateDisable 相机选择按钮不可用信号
    */
    void seltBtnStateDisable();

    /**
    * @brief noDeviceFound 未发现相机设备信号
    */
    void noDeviceFound();

    /**
    * @brief existDevice 存在相机设备信号
    */
    void existDevice();

protected:
    /**
    * @brief run 运行
    */
    //void run();

private slots:
    /**
    * @brief timeOutSlot 监视设备数目槽，根据相机数目执行相应的处理
    */
    void timeOutSlot();

private:
    QTimer             *m_pTimer;
    bool               m_noDevice;

};

#endif // DEVNUMMONITOR_H
