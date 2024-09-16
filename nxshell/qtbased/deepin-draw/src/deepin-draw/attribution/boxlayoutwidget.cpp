// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QPainter>
#include <QBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <DGuiApplicationHelper>

#include "boxlayoutwidget.h"
#include "application.h"

BoxLayoutWidget::BoxLayoutWidget(QWidget *parent) : QWidget(parent), m_color(Qt::white)
{

    init();
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [ = ](DGuiApplicationHelper::ColorType themeType) {
        if (themeType == DGuiApplicationHelper::LightType) {
            setColor(Qt::white);
        } else {
            setColor(QColor(0, 0, 0, int(0.1 * 255)));
        }

    });
}

void BoxLayoutWidget::setColor(QColor c)
{
    m_color = c;
}

void BoxLayoutWidget::setAlphaF(qreal alphaf)
{
    m_color.setAlphaF(alphaf);
}

void BoxLayoutWidget::addWidget(QWidget *w, int stretch)
{
    m_layout->addWidget(w, stretch);
}

void BoxLayoutWidget::resetLayout(QBoxLayout *l)
{
    if (nullptr == l) {
        return;
    }

    if (m_layout != nullptr && m_layout->count() > 0) {
        for (int i = m_layout->count(); i >= 0; i--) {
            l->addItem(m_layout->takeAt(i));
        }
    }

    m_layout = l;
    setContentsMargins(0, 0, 0, 0);
    setLayout(m_layout);
}

void BoxLayoutWidget::setContentsMargins(int left, int top, int right, int bottom)
{
    m_layout->setContentsMargins(left, top, right, bottom);
}

void BoxLayoutWidget::setMargins(int value)
{
    m_layout->setContentsMargins(value, value, value, value);
}

void BoxLayoutWidget::setBackgroundRect(bool roundedrect)
{
    m_backgroundrect = roundedrect;
}

void BoxLayoutWidget::setFrameRect(bool roundedrect)
{
    m_framerect = roundedrect;
}

void BoxLayoutWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    //绘制背景
    QPen pen(painter.pen());
    bool isNotValid = !m_color.isValid();
    const QColor borderColor = (isNotValid || m_color.alpha() == 0) ? QColor(77, 82, 93, int(0.8 * 255)) : QColor(255, 255, 255);
    bool   darkTheme = 1;
#ifdef USE_DTK
    darkTheme = (DGuiApplicationHelper::instance()->themeType()  == 2);
#endif
    QColor penColor  = darkTheme ? borderColor : QColor(0, 0, 0);
    painter.setRenderHint(QPainter::Antialiasing);
    if (m_backgroundrect) {
        penColor.setAlphaF(0.05);
        QBrush brush(penColor);
        pen.setColor(Qt::transparent);
        pen.setWidthF(2);
        painter.setPen(pen);
        painter.setBrush(brush);
        painter.drawRoundRect(rect(), 10, 30);
    }
    if (m_framerect) {
        //绘制边框
        const QColor frameBorderColor = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType ? QColor(0, 0, 0, int(0.1 * 255)) : QColor(255, 255, 255, int(0.08 * 255));
        painter.setPen(frameBorderColor);
        painter.drawRoundedRect(rect(), 8, 8);

    }
}

void BoxLayoutWidget::init()
{
    setAttribute(Qt::WA_TranslucentBackground);
    setFocusPolicy(Qt::NoFocus);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::Widget | Qt::NoDropShadowWindowHint);
    resetLayout(new QHBoxLayout);
}
