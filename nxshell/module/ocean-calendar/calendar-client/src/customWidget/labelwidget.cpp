// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "labelwidget.h"
#include <QPainter>
#include <QStyleOptionFocusRect>

LabelWidget::LabelWidget(QWidget *parent)
    : QLabel(parent)
{
    //设置焦点选中类型
    setFocusPolicy(Qt::FocusPolicy::TabFocus);
}

LabelWidget::~LabelWidget()
{

}

void LabelWidget::paintEvent(QPaintEvent *ev)
{
    QPainter painter(this);
    if (hasFocus()) {
        //有焦点，绘制焦点
        QStyleOptionFocusRect option;
        option.initFrom(this);
        option.backgroundColor = palette().color(QPalette::Background);
        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &option, &painter, this);
    }
    QLabel::paintEvent(ev);
}
