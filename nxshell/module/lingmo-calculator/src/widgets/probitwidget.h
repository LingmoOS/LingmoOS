// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROBITWIDGET_H
#define PROBITWIDGET_H

#include "../control/bitbutton.h"

#include <DWidget>
#include <DLabel>
#include <DGuiApplicationHelper>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

/**
 * @brief 一块二进制键盘控件
 */
class ProBitWidget : public DWidget
{
    Q_OBJECT

public:
    explicit ProBitWidget(QWidget *parent = nullptr);
    ~ProBitWidget();
    void setButtonEnabled(bool enabled);
    BitButton *getbutton(int i);

private:
    BitButton *m_firstbtn;
    BitButton *m_secondbtn;
    BitButton *m_thirdbtn;
    BitButton *m_fourthbtn;
    DLabel *m_label;
};

#endif // PROBITWIDGET_H
