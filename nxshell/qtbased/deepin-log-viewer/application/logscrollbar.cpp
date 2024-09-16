// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logscrollbar.h"


LogScrollBar::LogScrollBar(QWidget *parent)
    : QScrollBar(parent)
{

}

LogScrollBar::LogScrollBar(Qt::Orientation o, QWidget *parent)
    : QScrollBar(o, parent)
{

}

void LogScrollBar::mousePressEvent(QMouseEvent *event)
{
    m_isOnPress = true;
    QScrollBar::mousePressEvent(event);
}
