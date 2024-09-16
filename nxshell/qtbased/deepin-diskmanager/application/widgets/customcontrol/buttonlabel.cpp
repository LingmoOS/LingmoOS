// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "buttonlabel.h"

ButtonLabel::ButtonLabel(QWidget *parent) : QLabel(parent)
{

}

void ButtonLabel::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    emit clicked();
}
