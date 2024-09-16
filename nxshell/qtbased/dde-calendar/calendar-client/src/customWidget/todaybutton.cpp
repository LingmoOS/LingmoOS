// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "todaybutton.h"
#include "constants.h"

#include <DPalette>

#include <QPainter>
#include <QMouseEvent>

DGUI_USE_NAMESPACE
CTodayButton::CTodayButton(QWidget *parent)
    : DPushButton(parent)
{
}





void CTodayButton::keyPressEvent(QKeyEvent *event)
{
    //添加回车点击效果处理
    if (event->key() == Qt::Key_Return) {
        click();
    }
    DPushButton::keyPressEvent(event);
}
