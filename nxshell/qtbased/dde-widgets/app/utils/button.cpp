// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "button.h"
#include <QDebug>
#include <QKeyEvent>

WIDGETS_FRAME_BEGIN_NAMESPACE
DWIDGET_USE_NAMESPACE
TransparentButton::TransparentButton(QWidget *parent)
    : DPushButton(parent)
{
}

void TransparentButton::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if (hasFocus()) {
            clicked();
            break;
        }
        Q_FALLTHROUGH();
    default:
        break;
    }
    return DPushButton::keyPressEvent(event);
}

WIDGETS_FRAME_END_NAMESPACE
