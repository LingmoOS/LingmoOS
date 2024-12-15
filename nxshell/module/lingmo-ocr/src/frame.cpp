// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "frame.h"

Frame::Frame(QWidget *parent, Qt::WindowFlags f)
    : QFrame(parent, f)
{

}

void Frame::resizeEvent(QResizeEvent *event)
{
    emit sigFrameResize();
    return QFrame::resizeEvent(event);
}
