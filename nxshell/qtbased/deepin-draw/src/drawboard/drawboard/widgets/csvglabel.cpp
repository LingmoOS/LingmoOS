// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "csvglabel.h"
#include <QWidget>
#include <QPainter>
CSvgLabel::CSvgLabel(const QString &str, QWidget *parent)
    : QLabel(parent)
    , m_svgRender(str)
{

}

void CSvgLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    m_svgRender.render(&painter);
}
