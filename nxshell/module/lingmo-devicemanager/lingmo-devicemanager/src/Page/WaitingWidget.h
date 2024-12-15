// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WAITINGWIDGET_H
#define WAITINGWIDGET_H

#include <QObject>
#include <DWidget>
#include <DSpinner>
#include <DLabel>

using namespace Dtk::Widget;

/**
 * @brief The WaitingWidget class
 * 等待窗口
 */
class WaitingWidget : public DWidget
{
    Q_OBJECT
public:
    explicit WaitingWidget(QWidget *parent = nullptr);
    ~WaitingWidget();

    /**
     * @brief start开始转动小圈圈
     */
    void start();

    /**
     * @brief stop:停止转动小圈圈
     */
    void stop();

private:
    DSpinner        *mp_Spinner;       // 小圈圈
    DLabel          *mp_Label;         // 小圈圈下面的label
};

#endif // WAITINGWIDGET_H
