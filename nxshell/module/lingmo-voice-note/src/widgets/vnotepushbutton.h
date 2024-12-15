// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VNOTEPUSHBUTTON_H
#define VNOTEPUSHBUTTON_H

#include <DPushButton>
#include <DPalette>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

class VNotePushbutton : public DPushButton
{
    Q_OBJECT
public:
    explicit VNotePushbutton(QWidget *parent = nullptr);
    ~VNotePushbutton() override {}
protected:
    void enterEvent(QEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
};
#endif // TOOLBUTTON_H
