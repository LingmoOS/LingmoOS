/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2021, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "float-pane-widget.h"
#include <QVariantAnimation>

FloatPaneWidget::FloatPaneWidget(QWidget *mainWidget, QWidget *floatWidget, QWidget *parent) : QWidget(parent)
{
    m_mainWidget = mainWidget;
    mainWidget->setParent(this);
    m_floatWidget = floatWidget;
    floatWidget->setParent(this);
    floatWidget->setVisible(false);
    m_floatWidgetWidthAnimation = new QVariantAnimation(this);
    m_floatWidgetWidthAnimation->setStartValue(qreal(0.0));
    m_floatWidgetWidthAnimation->setEndValue(qreal(1.0));
    m_floatWidgetWidthAnimation->setDuration(500);
    m_floatWidgetWidthAnimation->setEasingCurve(QEasingCurve::OutCubic);

    connect(m_floatWidgetWidthAnimation, &QVariantAnimation::valueChanged, this, [=](){
        if (m_floatWidgetWidthAnimation->state() == QVariantAnimation::Running) {
            qreal currentValue = m_floatWidgetWidthAnimation->currentValue().toReal();
            int floatWidgetWidth = m_floatWidgetWidthHint * currentValue;
            m_floatWidget->move(0, 0);
            m_floatWidget->resize(floatWidgetWidth, this->height());
        }
    });

    connect(m_floatWidgetWidthAnimation, &QVariantAnimation::finished, this, [=](){
        if (m_floatWidgetWidthAnimation->currentValue().toReal() == 0) {
            m_floatWidget->setVisible(false);
            m_mainWidget->setGeometry(this->rect());
        } else {
            m_floatWidget->setVisible(true);
        }
    });
    setMinimumWidth(FLOAT_WIDGET_MIN_WIDTH);
}

void FloatPaneWidget::setFloatWidgetWidthHint(int widthHint)
{
    m_floatWidgetWidthHint = widthHint;
}

QSize FloatPaneWidget::sizeHint() const
{
    if (m_floatWidget->isVisible()) {
        return QSize(m_mainWidget->sizeHint().width() + m_floatWidgetWidthHint, m_mainWidget->sizeHint().height());
    } else {
        return QSize(m_mainWidget->sizeHint());
    }
}

void FloatPaneWidget::setFloatWidgetVisible(bool visible)
{
    if (visible) {
        m_floatWidget->setVisible(true);
//        resize(this->width() + m_floatWidgetWidthHint, this->height());
        m_mainWidget->move(m_floatWidgetWidthHint, 0);
        m_mainWidget->resize(size().width() - m_floatWidgetWidthHint, size().height());
    }
    m_floatWidgetWidthAnimation->setDirection(visible? QVariantAnimation::Forward: QVariantAnimation::Backward);
    m_floatWidgetWidthAnimation->start();
}

void FloatPaneWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (m_floatWidget->isVisible()) {
        m_mainWidget->move(m_floatWidgetWidthHint, 0);
        m_mainWidget->resize(this->width() - m_floatWidgetWidthHint, this->height());
        m_floatWidget->move(0, 0);
        m_floatWidget->resize(m_floatWidget->width(), this->height());
    } else {
        m_mainWidget->move(0, 0);
        m_mainWidget->resize(this->size());
    }
}
