// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "colorpickwidget.h"
#include "attributewidget.h"
#include "ccolorpanel.h"

ColorPickWidget::ColorPickWidget(QWidget *parent): DArrowRectangle(ArrowRight, parent)
{
    setWindowFlags(Qt::Popup);
    setFocusPolicy(Qt::NoFocus);
    setRadius(20);

    m_colorSettingWgt = new ColorSettingWgt(this);
    m_colorSettingWgt->setFocusPolicy(Qt::NoFocus);
    QVBoxLayout *l = new QVBoxLayout(this);
    l->setContentsMargins(10, 0, 35, 0);
    l->addWidget(m_colorSettingWgt);
    m_colorSettingWgt->show();
    setLayout(l);
    setRadiusArrowStyleEnable(true);
    setLeftRightRadius(true);
    connect(m_colorSettingWgt, &ColorSettingWgt::colorChanged, this, [ = ](const QColor & color, int phase) {
        emit colorChanged(color, phase);
    });

}

void ColorPickWidget::setVar(const QVariant &var)
{
    m_colorSettingWgt->setVar(var);
}

QWidget *ColorPickWidget::caller()
{
    return m_caller;
}

void ColorPickWidget::setShowPos(const QPoint &pos)
{
    m_point = pos;
}

void ColorPickWidget::setCaller(QWidget *pCaller)
{
    m_caller = pCaller;
}

void ColorPickWidget::setColor(const QColor &color, int phase)
{
    m_colorSettingWgt->setColor(color, phase);
}

void ColorPickWidget::paintEvent(QPaintEvent *e)
{
    resize(sizeHint());
    DArrowRectangle::paintEvent(e);
}

void ColorPickWidget::resizeEvent(QResizeEvent *e)
{
    //播放动画调整窗口大小，需要移动窗口位置
    move(m_point.x(), m_point.y());
    DArrowRectangle::resizeEvent(e);
}


