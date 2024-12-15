// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PAGEBOARDINFO_H
#define PAGEBOARDINFO_H

#include <QObject>
#include "PageSingleInfo.h"

class DeviceBios;

/**
 * @brief The PageBoardInfo class
 * 主板的界面展示类
 */

class PageBoardInfo : public PageSingleInfo
{
    Q_OBJECT
public:
    explicit PageBoardInfo(QWidget *parent = nullptr);

    /**
     * @brief updateInfo:更新主板信息
     * @param lst:主板信息lst
     */
    void updateInfo(const QList<DeviceBaseInfo *> &lst)override;

    /**
     * @brief loadDeviceInfo:加载设备信息
     * @param devices:设备列表
     * @param lst:设备信息列表
     */
    void loadDeviceInfo(const QList<DeviceBaseInfo *> &devices, const QList<QPair<QString, QString>> &lst);

    /**
     * @brief getOtherInfoPair:获取其他信息列表
     * @param lst:所有信息列表
     * @param lstPair:其他信息列表
     */
    void getOtherInfoPair(const QList<DeviceBaseInfo *> &lst, QList<QPair<QString, QString>> &lstPair);

    /**
     * @brief getKeyValueInfo 将设备信息直接转换为key-value值
     * @param device:设备指针
     * @param pair:信息键值对
     */
    void getValueInfo(DeviceBaseInfo *device, QPair<QString, QString> &pair);

    /**
     * @brief isBaseBoard:是否是主板界面
     * @return true:是主板界面
     */
    bool isBaseBoard() override {return true;}

    /**
     * @brief setFontChangeFlag: 设置字体变化标志
     */
    void setFontChangeFlag();

private:
    bool                 m_FontChangeFlag;     // 设置字体变化标志
};

#endif // PAGEBOARDINFO_H
