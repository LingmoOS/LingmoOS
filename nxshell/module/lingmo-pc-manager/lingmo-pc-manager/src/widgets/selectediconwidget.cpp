// Copyright (C) 2019 ~ 2022 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "selectediconwidget.h"

#include <DStyleHelper>

#include <QDebug>
#include <QPaintEvent>
#include <QPainter>
#include <QStyle>

DWIDGET_USE_NAMESPACE

namespace def {
namespace widgets {
SelectedIconWidget::SelectedIconWidget(QWidget *parent)
    : QWidget(parent)
{
    QStyleOption option;
    option.initFrom(this);
    m_icon = DStyleHelper(style()).standardIcon(DStyle::SP_IndicatorChecked, &option, this);
}

SelectedIconWidget::~SelectedIconWidget() { }

void SelectedIconWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    QPalette pa = this->palette();
    QColor color = pa.color(QPalette::ColorGroup::Normal, QPalette::ColorRole::Highlight);
    painter.setPen(color);

    QRect iconRect(QPoint(0, 0), size());

    iconRect.moveCenter(rect().center());
    iconRect.moveLeft(rect().left());

    m_icon.paint(&painter, iconRect);
}

} // namespace widgets
} // namespace def
