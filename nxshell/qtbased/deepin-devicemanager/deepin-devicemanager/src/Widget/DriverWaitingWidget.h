// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DRIVERWAITINGWIDGET_H
#define DRIVERWAITINGWIDGET_H

#include <DWidget>
#include <DWaterProgress>
#include <DLabel>

DWIDGET_USE_NAMESPACE
class DriverWaitingWidget : public DWidget
{
public:
    explicit DriverWaitingWidget(QString status, QWidget *parent = nullptr);

    /**
     * @brief setValue 设置进度条的数值
     * @param value 0 ~ 100
     */
    void setValue(int value);

    /**
     * @brief setText 设置进度条下面的字符串
     * @param text 例如：正在卸载、正在更新
     */
    void setText(const QString &text);
private:
    /**
     * @brief init 初始化界面
     */
    void init();
private slots:
    /**
     * @brief onUpdateTheme 更新主题
     */
    void onUpdateTheme();
private:
    DWaterProgress        *mp_Progress;
    DLabel                *mp_Label;
};
#endif // DRIVERWAITINGWIDGET_H
