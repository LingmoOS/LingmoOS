// Copyright (C) 2019 ~ 2022 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYSCHECKRESULTHEADERWIDGET_H
#define SYSCHECKRESULTHEADERWIDGET_H

#include "systemcheckdefinition.h"

#include <DFrame>
#include <DLabel>
#include <DTipLabel>

#include <QWidget>

class SysCheckResultHeaderWidget : public Dtk::Widget::DFrame
{
    Q_OBJECT
public:
    explicit SysCheckResultHeaderWidget(QWidget *parent = nullptr);

    ~SysCheckResultHeaderWidget() { }

public Q_SLOTS:
    void onPointChanged(int, int);
    void onFastFixFinished();
    void onFastFixStarted();

private:
    void initUI();
    void hideAll();

private:
    Dtk::Widget::DLabel *m_pointValue;
    Dtk::Widget::DLabel *m_pointText;
    Dtk::Widget::DLabel *m_title;
    Dtk::Widget::DLabel *m_logo;
    Dtk::Widget::DTipLabel *m_tips;
    Dtk::Widget::DLabel *m_fixingTitle;
    Dtk::Widget::DTipLabel *m_fixingTips;
};

#endif // SYSCHECKRESULTHEADERWIDGET_H
