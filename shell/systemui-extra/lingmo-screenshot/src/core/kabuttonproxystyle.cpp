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

#include "kabuttonproxystyle.h"

#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>
#include <QStyle>
#include <QDebug>

KAButtonProxyStyle::KAButtonProxyStyle(QStyle *style)
    : QProxyStyle(style)
{
}

KAButtonProxyStyle::KAButtonProxyStyle(const QString &key)
    : QProxyStyle(key)
{
}

void KAButtonProxyStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch(element) {
    case CE_PushButton:
        if (const KAButtonStyleOption *btn = qstyleoption_cast<const KAButtonStyleOption *>(option)) {
            if (KAButtonStyleOption::BT_GROUP == btn->katype) {
                if (btn->position < KAButtonStyleOption::Begin ||
                    btn->position > KAButtonStyleOption::End)
                    break;
                proxy()->drawControl(CE_PushButtonBevel, btn, painter, widget);
                KAButtonStyleOption subopt = *btn;
                subopt.rect = subElementRect(SE_PushButtonContents, btn, widget);
                proxy()->drawControl(CE_PushButtonLabel, &subopt, painter, widget);
                return ;
            } else if (KAButtonStyleOption::BT_TAB == btn->katype) {
                proxy()->drawControl(CE_PushButtonBevel, btn, painter, widget);
                KAButtonStyleOption subopt = *btn;
                subopt.rect = subElementRect(SE_PushButtonContents, btn, widget);
                proxy()->drawControl(CE_PushButtonLabel, &subopt, painter, widget);
                return ;
            }
        }
        break;
    case CE_PushButtonBevel:
        if (const KAButtonStyleOption *btn = qstyleoption_cast<const KAButtonStyleOption *>(option)) {
            if (KAButtonStyleOption::BT_GROUP == btn->katype) {
                if (btn->position < KAButtonStyleOption::Begin ||
                    btn->position > KAButtonStyleOption::End)
                    break;
                QRect br = btn->rect;
                int dbi = proxy()->pixelMetric(PM_ButtonDefaultIndicator, btn, widget);
                if (btn->features & QStyleOptionButton::AutoDefaultButton)
                    br.setCoords(br.left() + dbi, br.top() + dbi, br.right() - dbi, br.bottom() - dbi);

                KAButtonStyleOption tmpBtn = *btn;
                tmpBtn.rect = br;
                proxy()->drawPrimitive(PE_PanelButtonCommand, &tmpBtn, painter, widget);
                return ;
            } else if (KAButtonStyleOption::BT_TAB == btn->katype) {
                QRect br = btn->rect;
                int dbi = proxy()->pixelMetric(PM_ButtonDefaultIndicator, btn, widget);
                if (btn->features & QStyleOptionButton::AutoDefaultButton)
                    br.setCoords(br.left() + dbi, br.top() + dbi, br.right() - dbi, br.bottom() - dbi);

                KAButtonStyleOption tmpBtn = *btn;
                tmpBtn.rect = br;
                proxy()->drawPrimitive(PE_PanelButtonCommand, &tmpBtn, painter, widget);
                return ;
            }
        }
        break;
    case CE_PushButtonLabel:
        {
            if (const KAButtonStyleOption *button = qstyleoption_cast<const KAButtonStyleOption *>(option)) {
                if (KAButtonStyleOption::BT_TAB != button->katype)
                    break;
                const bool enable = button->state & State_Enabled;
                const bool text = !button->text.isNull();
                const bool icon = !button->icon.isNull();

                bool isWindowButton = false;
                bool isWindowColoseButton = false;
                bool isImportant = false;
                bool useButtonPalette = false;
                if (widget) {
                    if (widget->property("isWindowButton").isValid()) {
                        if (widget->property("isWindowButton").toInt() == 0x01)
                            isWindowButton = true;
                        if (widget->property("isWindowButton").toInt() == 0x02)
                            isWindowColoseButton = true;
                    }
                    if (widget->property("isImportant").isValid())
                        isImportant = widget->property("isImportant").toBool();

                    if (widget->property("useButtonPalette").isValid())
                        useButtonPalette = widget->property("useButtonPalette").toBool();
                }

                QRect drawRect = button->rect;
                int spacing = 8;
                QStyleOption sub = *option;
                if (isImportant && !(button->features & QStyleOptionButton::Flat))
                    sub.state = option->state | State_On;
                else if (isWindowButton || useButtonPalette)
                    sub.state = enable ? State_Enabled : State_None;
                else
                    sub.state = option->state;

                if (button->features & QStyleOptionButton::HasMenu) {
                    QRect arrowRect;
                    int indicator = proxy()->pixelMetric(PM_MenuButtonIndicator, option, widget);
                    arrowRect.setRect(drawRect.right() - indicator, drawRect.top() + (drawRect.height() - indicator) / 2, indicator, indicator);
                    arrowRect = visualRect(option->direction, option->rect, arrowRect);
                    if (!text && !icon)
                        spacing = 0;
                    drawRect.setWidth(drawRect.width() - indicator - spacing);
                    drawRect = visualRect(button->direction, button->rect, drawRect);
                    sub.rect = arrowRect;
                    proxy()->drawPrimitive(PE_IndicatorArrowDown, &sub, painter, widget);
                }

                int tf = Qt::AlignCenter;
                if (proxy()->styleHint(SH_UnderlineShortcut, button, widget))
                    tf |= Qt::TextShowMnemonic;
                QPixmap pixmap;
                if (icon) {
                    QIcon::Mode mode = button->state & State_Enabled ? QIcon::Normal : QIcon::Disabled;
                    if (mode == QIcon::Normal && button->state & State_HasFocus)
                        mode = QIcon::Active;
                    QIcon::State state = QIcon::Off;
                    if (button->state & State_On)
                        state = QIcon::On;
                    pixmap = button->icon.pixmap(button->iconSize, mode, state);
                }

                QFontMetrics fm = button->fontMetrics;
                int textWidth = fm.boundingRect(option->rect, tf, button->text).width() + 2;
                int iconWidth = icon ? button->iconSize.width() : 0;
                QRect iconRect, textRect;
                if (icon && text) {
                    int width = textWidth + spacing + iconWidth;
                    if (width > drawRect.width()) {
                        width = drawRect.width();
                        textWidth = width - spacing - iconWidth;
                    }
                    textRect.setRect(drawRect.x(), drawRect.y(), width, drawRect.height());
                    textRect.moveCenter(drawRect.center());
                    iconRect.setRect(textRect.left(), textRect.top(), iconWidth, textRect.height());
                    textRect.setRect(iconRect.right() + spacing + 1, textRect.y(), textWidth, textRect.height());
                    iconRect = visualRect(option->direction, drawRect, iconRect);
                    textRect = visualRect(option->direction, drawRect, textRect);
                } else if (icon) {
                    iconRect = drawRect;
                } else if (text) {
                    textRect = drawRect;
                }

                if (iconRect.isValid()) {
                    QStyle::drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);
                }

                if (textRect.isValid()) {
                    if (enable) {
                        if (isWindowButton || useButtonPalette) {
                            proxy()->drawItemText(painter, textRect, tf, button->palette, true, button->text, QPalette::ButtonText);
                        } else {
                            if (isImportant) {
                                if (button->features & QStyleOptionButton::Flat) {
                                    proxy()->drawItemText(painter, textRect, tf, button->palette, true, button->text, QPalette::ButtonText);
                                } else {
                                    proxy()->drawItemText(painter, textRect, tf, button->palette, true, button->text, QPalette::HighlightedText);
                                }
                                if (button->state & (State_MouseOver | State_Sunken | State_On)) {
                                    proxy()->drawItemText(painter, textRect, tf, button->palette, true, button->text, QPalette::HighlightedText);
                                }
                            } else {
                                if (button->state & (State_Sunken | State_On)) {
                                    proxy()->drawItemText(painter, textRect, tf, button->palette, true, button->text, QPalette::Highlight);
                                } else if (button->state & (State_MouseOver)) {
                                    proxy()->drawItemText(painter, textRect, tf, button->palette, true, button->text, QPalette::Highlight);
                                } else {
                                    proxy()->drawItemText(painter, textRect, tf, button->palette, true, button->text, QPalette::ButtonText);
                                }
                            }
                        }
                    } else {
                        proxy()->drawItemText(painter, textRect, tf, button->palette, false, button->text, QPalette::ButtonText);
                    }
                }
                return;
            }
            break;
        }
    default:
        break;
    }
    return QProxyStyle::drawControl(element, option, painter, widget);
}

void KAButtonProxyStyle::drawItemPixmap(QPainter *painter, const QRect &rectangle, int alignment, const QPixmap &pixmap) const
{
    return QProxyStyle::drawItemPixmap(painter, rectangle, alignment, pixmap);
}

void KAButtonProxyStyle::drawItemText(QPainter *painter, const QRect &rectangle, int alignment, const QPalette &palette, bool enabled, const QString &text, QPalette::ColorRole textRole) const
{
    return QProxyStyle::drawItemText(painter, rectangle, alignment, palette, enabled, text, textRole);
}

void KAButtonProxyStyle::drawRoundedRect(QPainter *pa, const QRect &rect, qreal xRadius, qreal yRadius, Corners corners, Qt::SizeMode mode) const
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
void KAButtonProxyStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch(element) {
    case PE_PanelButtonCommand:
    {
        if (const KAButtonStyleOption *btn = qstyleoption_cast<const KAButtonStyleOption *>(option)) {
            if (KAButtonStyleOption::BT_GROUP == btn->katype) {
                if (btn->position < KAButtonStyleOption::Begin ||
                    btn->position > KAButtonStyleOption::End)
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
                    case KAButtonStyleOption::Begin:
                        drawRoundedRect(painter, rect, 6.0, 6.0, (Corners)(TopLeftCorner | BottomLeftCorner));
                        break;
                    case KAButtonStyleOption::Middle:
                        painter->drawRect(rect);
                        break;
                    case KAButtonStyleOption::End:
                        drawRoundedRect(painter, rect, 6.0, 6.0, (Corners)(TopRightCorner | BottomRightCorner));
                        break;
                    case KAButtonStyleOption::OnlyOne:
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
                    case KAButtonStyleOption::Begin:
                        drawRoundedRect(painter, rect, 6.0, 6.0, (Corners)(TopLeftCorner | BottomLeftCorner));
                        break;
                    case KAButtonStyleOption::Middle:
                        painter->drawRect(rect);
                        break;
                    case KAButtonStyleOption::End:
                        drawRoundedRect(painter, rect, 6.0, 6.0, (Corners)(TopRightCorner | BottomRightCorner));
                        break;
                    case KAButtonStyleOption::OnlyOne:
                    default:
                        painter->drawRoundedRect(rect, 6.0, 6.0);
                        break;
                    }
                    painter->restore();
                }

                painter->save();
                painter->setRenderHint(QPainter::Antialiasing,true);
                painter->setPen(Qt::NoPen);
                if (btn->state & (QStyle::State_Sunken | QStyle::State_On)) {
                    painter->setBrush(btn->palette.color(QPalette::Highlight));
                } else if (btn->state & QStyle::State_MouseOver) {
                    auto color = btn->palette.color(QPalette::Highlight).lighter(125);
                    painter->setBrush(color);
                }
                switch(btn->position) {
                case KAButtonStyleOption::Begin:
                    drawRoundedRect(painter, rect, 6.0, 6.0, (Corners)(TopLeftCorner | BottomLeftCorner));
                    break;
                case KAButtonStyleOption::Middle:
                    painter->drawRect(rect);
                    break;
                case KAButtonStyleOption::End:
                    drawRoundedRect(painter, rect, 6.0, 6.0, (Corners)(TopRightCorner | BottomRightCorner));
                    break;
                case KAButtonStyleOption::OnlyOne:
                default:
                    painter->drawRoundedRect(rect, 6.0, 6.0);
                    break;
                }
                painter->restore();
                return;
            } else if (KAButtonStyleOption::BT_TAB == btn->katype) {
                QRect rect = btn->rect;
                rect.setY(rect.y()+rect.height()-2);
                rect.setHeight(2);
                if (!(btn->state & QStyle::State_Enabled)) {
                    if (btn->features & QStyleOptionButton::Flat)
                        return;
                    painter->save();
                    painter->setPen(Qt::NoPen);
                    painter->setBrush(btn->palette.color(QPalette::Disabled, QPalette::Button));
                    painter->setRenderHint(QPainter::Antialiasing, true);
                    painter->drawRoundedRect(rect, 1.0, 1.0);
                    painter->restore();
                    return;
                }

                painter->save();
                painter->setRenderHint(QPainter::Antialiasing,true);
                painter->setPen(Qt::NoPen);
                if (btn->state & (QStyle::State_Sunken | QStyle::State_On)) {
                    painter->setBrush(btn->palette.color(QPalette::Highlight));
                } else if (btn->state & QStyle::State_MouseOver) {
                    rect.setHeight(0);
                    auto color = btn->palette.color(QPalette::Highlight).lighter(125);
                    painter->setBrush(color);
                }
                painter->drawRoundedRect(rect, 1.0, 1.0);
                painter->restore();
                return;
            }
        }
    }
        break;
    default:
        break;
    }
    return QProxyStyle::drawPrimitive(element, option, painter, widget);
}

// change control Qsize
QSize KAButtonProxyStyle::sizeFromContents(ContentsType ct, const QStyleOption *option,
                                     const QSize &size, const QWidget *widget) const
{
    QSize  newSize = size;
    switch (ct) {
    case CT_PushButton:
        {
            if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
                if (!widget->inherits("KATabButton"))
                    break;
                const bool icon = !button->icon.isNull();
                const bool text = !button->text.isNull();
                int w = size.width();
                int h = size.height();
                int Margin_Height = 2;
                int ToolButton_MarginWidth = 2;
                int Button_MarginWidth = proxy()->pixelMetric(PM_ButtonMargin, option, widget);
                if (text && !icon && !(button->features & QStyleOptionButton::HasMenu)) {
                    w += Button_MarginWidth * 2;
                } else {
                    w += ToolButton_MarginWidth * 2;
                }
                h += Margin_Height * 2;

                int spacing = 0;
                if (text && icon)
                    spacing += 4;
                if (!text && icon)
                    spacing -= 4;
                if (button->features & QStyleOptionButton::HasMenu) {
                    if (icon || text)
                        spacing += 8;
                }
                w += spacing;
                if (button->features & (QStyleOptionButton::AutoDefaultButton | QStyleOptionButton::DefaultButton)) {
                    int dbw = proxy()->pixelMetric(PM_ButtonDefaultIndicator, option, widget) * 2;
                    w += dbw;
                    h += dbw;
                }

                newSize.setWidth(w);
                newSize.setHeight(h);
                return newSize;
            }
            break;
        }
    default:
        break;
    }
    return QProxyStyle::sizeFromContents(ct, option, size, widget);
}
