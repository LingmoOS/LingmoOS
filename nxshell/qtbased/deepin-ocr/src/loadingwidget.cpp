// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "loadingwidget.h"

loadingWidget::loadingWidget(QWidget *parent)
    : DLabel(parent)
{

}

void loadingWidget::resizeEvent(QResizeEvent *event)
{
    emit sigChangeSize();
    return DLabel::resizeEvent(event);
}
