/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "usmproxystyle.h"

#include <QPainter>
#include <QStyleOption>
#include <QStyle>
#include <QDebug>

USMProxyStyle::USMProxyStyle(QStyle *style)
    : QProxyStyle(style)
{
}

USMProxyStyle::USMProxyStyle(const QString &key)
    : QProxyStyle(key)
{
    themeStyle = key;
}

void USMProxyStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch(element) {
    case CE_PushButton:
        if (const GroupButtonStyleOption *btn = qstyleoption_cast<const GroupButtonStyleOption *>(option)) {
            if (btn->position < GroupButtonStyleOption::Begin || 
                btn->position > GroupButtonStyleOption::End)
                break;
            proxy()->drawControl(CE_PushButtonBevel, btn, painter, widget);
            GroupButtonStyleOption subopt = *btn;
            subopt.rect = subElementRect(SE_PushButtonContents, btn, widget);
            proxy()->drawControl(CE_PushButtonLabel, &subopt, painter, widget);
        }
        return;
    case CE_PushButtonBevel:
        if (const GroupButtonStyleOption *btn = qstyleoption_cast<const GroupButtonStyleOption *>(option)) {
            if (btn->position < GroupButtonStyleOption::Begin || 
                btn->position > GroupButtonStyleOption::End)
                break;
            QRect br = btn->rect;
            int dbi = proxy()->pixelMetric(PM_ButtonDefaultIndicator, btn, widget);
            if (btn->features & QStyleOptionButton::AutoDefaultButton)
                br.setCoords(br.left() + dbi, br.top() + dbi, br.right() - dbi, br.bottom() - dbi);

            GroupButtonStyleOption tmpBtn = *btn;
            tmpBtn.rect = br;
            proxy()->drawPrimitive(PE_PanelButtonCommand, &tmpBtn, painter, widget);
        }
        return;
    default:
        break;
    }
    return QProxyStyle::drawControl(element, option, painter, widget);
}

void USMProxyStyle::drawItemPixmap(QPainter *painter, const QRect &rectangle, int alignment, const QPixmap &pixmap) const
{
    return QProxyStyle::drawItemPixmap(painter, rectangle, alignment, pixmap);
}

void USMProxyStyle::drawItemText(QPainter *painter, const QRect &rectangle, int alignment, const QPalette &palette, bool enabled, const QString &text, QPalette::ColorRole textRole) const
{
    return QProxyStyle::drawItemText(painter, rectangle, alignment, palette, enabled, text, textRole);
}

void USMProxyStyle::drawRoundedRect(QPainter *pa, const QRect &rect, qreal xRadius, qreal yRadius, Corners corners, Qt::SizeMode mode) const
{
    QRectF r = rect.normalized();

    if (r.isNull())
        return;

    if (mode == Qt::AbsoluteSize) {
        qreal w = r.width() / 2;
        qreal h = r.height() / 2;

        if (w == 0) {
            xRadius = 0;
        } else {
            xRadius = 100 * qMin(xRadius, w) / w;
        }
        if (h == 0) {
            yRadius = 0;
        } else {
            yRadius = 100 * qMin(yRadius, h) / h;
        }
    } else {
        if (xRadius > 100)                          // fix ranges
            xRadius = 100;

        if (yRadius > 100)
            yRadius = 100;
    }

    if (xRadius <= 0 || yRadius <= 0) {             // add normal rectangle
        pa->drawRect(r);
        return;
    }

    QPainterPath path;
    qreal x = r.x();
    qreal y = r.y();
    qreal w = r.width();
    qreal h = r.height();
    qreal rxx2 = w * xRadius / 100;
    qreal ryy2 = h * yRadius / 100;

    path.arcMoveTo(x, y, rxx2, ryy2, 180);

    if (corners & TopLeftCorner) {
        path.arcTo(x, y, rxx2, ryy2, 180, -90);
    } else {
        path.lineTo(r.topLeft());
    }

    if (corners & TopRightCorner) {
        path.arcTo(x + w - rxx2, y, rxx2, ryy2, 90, -90);
    } else {
        path.lineTo(r.topRight());
    }

    if (corners & BottomRightCorner) {
        path.arcTo(x + w - rxx2, y + h - ryy2, rxx2, ryy2, 0, -90);
    } else {
        path.lineTo(r.bottomRight());
    }

    if (corners & BottomLeftCorner) {
        path.arcTo(x, y + h - ryy2, rxx2, ryy2, 270, -90);
    } else {
        path.lineTo(r.bottomLeft());
    }

    path.closeSubpath();
    pa->drawPath(path);
}

//绘制简单的颜色圆角等
void USMProxyStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch(element) {
    case PE_PanelButtonCommand:
    {
        if (const GroupButtonStyleOption *btn = qstyleoption_cast<const GroupButtonStyleOption *>(option)) {
            if (btn->position < GroupButtonStyleOption::Begin || 
                btn->position > GroupButtonStyleOption::End)
                break;
            QRect rect = btn->rect;
            if (!(btn->state & QStyle::State_Enabled)) {
                if (btn->features & QStyleOptionButton::Flat)
                    return;
                painter->save();
                painter->setPen(Qt::NoPen);
                painter->setBrush(btn->palette.color(QPalette::Disabled, QPalette::Button));
                painter->setRenderHint(QPainter::Antialiasing, true);
                switch(btn->position) {
                case GroupButtonStyleOption::Begin:
                    drawRoundedRect(painter, rect, 6.0, 6.0, (Corners)(TopLeftCorner | BottomLeftCorner));
                    break;
                case GroupButtonStyleOption::Middle:
                    painter->drawRect(rect);
                    break;
                case GroupButtonStyleOption::End:
                    drawRoundedRect(painter, rect, 6.0, 6.0, (Corners)(TopRightCorner | BottomRightCorner));
                    break;
                case GroupButtonStyleOption::OnlyOne:
                default:
                    painter->drawRoundedRect(rect, 6.0, 6.0);
                    break;
                }
                painter->restore();
                return;
            }

            if (!(btn->state & QStyle::State_AutoRaise) && !(btn->features & QStyleOptionButton::Flat)) {
                painter->save();
                painter->setPen(Qt::NoPen);
                painter->setBrush(btn->palette.color(QPalette::Button));
                painter->setRenderHint(QPainter::Antialiasing, true);
                switch(btn->position) {
                case GroupButtonStyleOption::Begin:
                    drawRoundedRect(painter, rect, 6.0, 6.0, (Corners)(TopLeftCorner | BottomLeftCorner));
                    break;
                case GroupButtonStyleOption::Middle:
                    painter->drawRect(rect);
                    break;
                case GroupButtonStyleOption::End:
                    drawRoundedRect(painter, rect, 6.0, 6.0, (Corners)(TopRightCorner | BottomRightCorner));
                    break;
                case GroupButtonStyleOption::OnlyOne:
                default:
                    painter->drawRoundedRect(rect, 6.0, 6.0);
                    break;
                }
                painter->restore();
            }

            painter->save();
            painter->setRenderHint(QPainter::Antialiasing,true);
            painter->setPen(Qt::NoPen);
            //选中的优先级最高
            if (btn->state & (QStyle::State_On)){
                painter->setBrush(btn->palette.color(QPalette::Highlight));
            } else if (btn->state & (QStyle::State_Sunken)) {
                QColor color = button_Click(option);
                painter->setBrush(color);
            } else if (btn->state & QStyle::State_MouseOver) {
                QColor color = button_Hover(option);
                painter->setBrush(color);
            }
            switch(btn->position) {
            case GroupButtonStyleOption::Begin:
                drawRoundedRect(painter, rect, 6.0, 6.0, (Corners)(TopLeftCorner | BottomLeftCorner));
                break;
            case GroupButtonStyleOption::Middle:
                painter->drawRect(rect);
                break;
            case GroupButtonStyleOption::End:
                drawRoundedRect(painter, rect, 6.0, 6.0, (Corners)(TopRightCorner | BottomRightCorner));
                break;
            case GroupButtonStyleOption::OnlyOne:
            default:
                painter->drawRoundedRect(rect, 6.0, 6.0);
                break;
            }
            painter->restore();
        }
    }
        return;
    default:
        break;
    }
    return QProxyStyle::drawPrimitive(element, option, painter, widget);
}


QColor USMProxyStyle::button_Hover(const QStyleOption *option) const
{
    QColor button = option->palette.color(QPalette::Active, QPalette::Button);
    QColor mix    = option->palette.color(QPalette::Active, QPalette::BrightText);

    if (themeStyle.contains("dark") || themeStyle.contains("black")) {
        return mixColor(button, mix, 0.2);
    }

    return mixColor(button, mix, 0.05);
}

QColor USMProxyStyle::button_Click(const QStyleOption *option) const
{
    QColor button = option->palette.color(QPalette::Active, QPalette::Button);
    QColor mix    = option->palette.color(QPalette::Active, QPalette::BrightText);

    if (themeStyle.contains("dark") || themeStyle.contains("black")) {
        return mixColor(button, mix, 0.05);
    }
    return mixColor(button, mix, 0.2);

}

QColor USMProxyStyle::mixColor(const QColor &c1, const QColor &c2, qreal bias) const
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

    qreal r = mixQreal(c1.redF(),   c2.redF(),   bias);
    qreal g = mixQreal(c1.greenF(), c2.greenF(), bias);
    qreal b = mixQreal(c1.blueF(),  c2.blueF(),  bias);
    qreal a = mixQreal(c1.alphaF(), c2.alphaF(), bias);

    return QColor::fromRgbF(r, g, b, a);
}