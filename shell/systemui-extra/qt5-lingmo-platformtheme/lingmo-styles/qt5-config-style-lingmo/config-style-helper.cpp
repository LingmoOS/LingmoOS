/*
 * Qt5-LINGMO's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: xibowen <xibowen@kylinos.cn>
 *
 */

#include "config-style-helper.h"

#include <QPainter>
#include <QStyleOption>
#include <QWidget>
#include <QPainterPath>



static qreal configMixQreal(qreal a, qreal b, qreal bias)
{
    return a + (b - a) * bias;
}



const QRegion configGetRoundedRectRegion(const QRect &rect, qreal radius_x, qreal radius_y)
{
    QPainterPath path;
    path.addRoundedRect(rect, radius_x, radius_y);
    return path.toFillPolygon().toPolygon();
}



qreal configCalcRadial(const QStyleOptionSlider *dial, int position)
{
    int currentSliderPosition = position;
    if (!dial->upsideDown) {
        if (position == dial->minimum)
            currentSliderPosition = dial->maximum;
        else if (position == dial->maximum)
            currentSliderPosition = dial->minimum;
        else
            currentSliderPosition = dial->maximum - position;
    }
    qreal a = 0;
    if (dial->maximum == dial->minimum)
        a = M_PI / 2;
    else if (dial->dialWrapping)
        a = M_PI * 3 / 2 - (currentSliderPosition - dial->minimum) * 2 * M_PI / (dial->maximum - dial->minimum);
    else
        a = (M_PI * 8 - (currentSliderPosition - dial->minimum) * 10 * M_PI / (dial->maximum - dial->minimum)) / 6;
    return a;
}



QPolygonF configCalcLines(const QStyleOptionSlider *dial, int offset)
{
    QPolygonF poly(0);
    int width = dial->rect.width();
    int height = dial->rect.height();
    qreal r = qMin(width, height) / 2;

    qreal xc = width / 2;
    qreal yc = height / 2;
    const int ns = dial->tickInterval;
    if (!ns) // Invalid values may be set by Qt Designer.
        return poly;
    int notches = (dial->maximum + ns - 1 - dial->minimum) / ns;
    if (notches <= 0)
        return poly;
    if (dial->maximum < dial->minimum || dial->maximum - dial->minimum > 1000) {
        int maximum = dial->minimum + 1000;
        notches = (maximum + ns - 1 - dial->minimum) / ns;
    }

    poly.resize(2 + 2 * notches);
    int smallLineSize = offset / 2;
    for (int i = 0; i <= notches; ++i) {
        qreal angle = dial->dialWrapping ? M_PI * 3 / 2 - i * 2 * M_PI / notches
                                         : (M_PI * 8 - i * 10 * M_PI / notches) / 6;
        qreal s = qSin(angle);
        qreal c = qCos(angle);
        poly[2 * i] = QPointF(xc + (r + 0.5 - smallLineSize) * c,
                              yc - (r + 0.5 - smallLineSize) * s);
        poly[2 * i + 1] = QPointF(xc + r * c, yc - r * s);
    }
    return poly;
}



void configTabLayout(const QStyleOptionTab *tab, const QWidget *widget, const QStyle *style, QRect *textRect, QRect *iconRect)
{
    Q_ASSERT(textRect);
    Q_ASSERT(iconRect);

    QRect rect = tab->rect;
    bool verticalTabs = tab->shape == QTabBar::RoundedEast || tab->shape == QTabBar::RoundedWest
            || tab->shape == QTabBar::TriangularEast || tab->shape == QTabBar::TriangularWest;

    int iconExtent = style->pixelMetric(QStyle::PM_SmallIconSize);
    if (verticalTabs)
        rect.setRect(0, 0, rect.height(), rect.width());

    int hpadding = style->pixelMetric(QStyle::PM_TabBarTabHSpace, tab, widget) / 2;
    int vpadding = style->pixelMetric(QStyle::PM_TabBarTabVSpace, tab, widget) / 2;

    rect.adjust(hpadding, vpadding, -hpadding, -vpadding);
    // left widget
    if (!tab->leftButtonSize.isEmpty()) {
        rect.setLeft(rect.left() + 8 + (verticalTabs ? tab->leftButtonSize.height() : tab->leftButtonSize.width()));
    }
    // right widget
    if (!tab->rightButtonSize.isEmpty()) {
        rect.setRight(rect.right() - 8 - (verticalTabs ? tab->rightButtonSize.height() : tab->rightButtonSize.width()));
    }
    // icon
    if (!tab->icon.isNull()) {
        QSize iconSize = tab->iconSize;
        if (!iconSize.isValid()) {
            iconSize = QSize(iconExtent, iconExtent);
        }
        QSize tabIconSize = tab->icon.actualSize(iconSize,
                                                 (tab->state & QStyle::State_Enabled) ? QIcon::Normal : QIcon::Disabled,
                                                 (tab->state & QStyle::State_Selected) ? QIcon::On : QIcon::Off);
        // High-dpi icons do not need adjustment; make sure tabIconSize is not larger than iconSize
        tabIconSize = QSize(qMin(tabIconSize.width(), iconSize.width()), qMin(tabIconSize.height(), iconSize.height()));

        *iconRect = QRect(rect.left(), rect.center().y() - tabIconSize.height() / 2,
                          tabIconSize.width(), tabIconSize.height());
        if (!verticalTabs)
            *iconRect = style->visualRect(tab->direction, tab->rect, *iconRect);
        rect.setLeft(rect.left() + tabIconSize.width() + 8);
    }
    if (!verticalTabs)
        rect = style->visualRect(tab->direction, tab->rect, rect);
    *textRect = rect;
}



QColor configMixColor(const QColor &c1, const QColor &c2, qreal bias)
{
    if (bias <= 0.0) {
        return c1;
    }
    if (bias >= 1.0) {
        return c2;
    }
    if (qIsNaN(bias)) {
        return c1;
    }

    qreal r = configMixQreal(c1.redF(),   c2.redF(),   bias);
    qreal g = configMixQreal(c1.greenF(), c2.greenF(), bias);
    qreal b = configMixQreal(c1.blueF(),  c2.blueF(),  bias);
    qreal a = configMixQreal(c1.alphaF(), c2.alphaF(), bias);

    return QColor::fromRgbF(r, g, b, a);
}



void configDrawArrow(const QStyle *style, const QStyleOptionToolButton *toolbutton,
                      const QRect &rect, QPainter *painter, const QWidget *widget)
{
    QStyle::PrimitiveElement pe;
    switch (toolbutton->arrowType) {
    case Qt::LeftArrow:
        pe = QStyle::PE_IndicatorArrowLeft;
        break;
    case Qt::RightArrow:
        pe = QStyle::PE_IndicatorArrowRight;
        break;
    case Qt::UpArrow:
        pe = QStyle::PE_IndicatorArrowUp;
        break;

    case Qt::DownArrow:
        pe = QStyle::PE_IndicatorArrowDown;
        break;
    default:
        return;
    }
    QStyleOption arrowOpt = *toolbutton;
    arrowOpt.rect = rect;
    style->drawPrimitive(pe, &arrowOpt, painter, widget);
}
