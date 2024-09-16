// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "previousbutton.h"

previousButton::previousButton(QWidget *parent)
    : DIconButton(parent)
{
    setFixedSize(36, 36);
}

void previousButton::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Left ||
        event->key() == Qt::Key_Right ||
        event->key() == Qt::Key_Up ||
        event->key() == Qt::Key_Down)
        return;

    DIconButton::keyPressEvent(event);
}
