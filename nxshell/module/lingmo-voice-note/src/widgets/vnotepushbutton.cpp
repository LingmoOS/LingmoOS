// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QWidget>
#include "vnotepushbutton.h"
VNotePushbutton::VNotePushbutton(QWidget *parent) : DPushButton (parent) {
    setCheckable(true);
    setFlat(true);
}

void VNotePushbutton::enterEvent(QEvent *e)
{
    if (this->isEnabled()) {
        setFlat(false);
    }
    DPushButton::enterEvent(e);
}

void VNotePushbutton::leaveEvent(QEvent *e)
{
    setFlat(true);
    DPushButton::leaveEvent(e);
}
void VNotePushbutton::mousePressEvent(QMouseEvent *e)
{
    DPushButton::mousePressEvent(e);
}


