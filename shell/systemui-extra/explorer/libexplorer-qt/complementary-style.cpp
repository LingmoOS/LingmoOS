/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#include "complementary-style.h"

#include "tool-bar.h"
#include "location-bar.h"

#include <QMenu>
#include <QPushButton>
#include <QStyleOptionComplex>

#include <QDebug>
#include <qdrawutil.h>

using namespace Peony;

static ComplementaryStyle *global_instance = nullptr;

ComplementaryStyle::ComplementaryStyle(QStyle *parent) : QProxyStyle(parent)
{
    m_styled_menu = new QMenu;
    m_styled_button = new QPushButton;
}

ComplementaryStyle::~ComplementaryStyle()
{
    m_styled_menu->deleteLater();
    m_styled_button->deleteLater();
}

ComplementaryStyle *ComplementaryStyle::getStyle()
{
    if (!global_instance)
        global_instance = new ComplementaryStyle;
    return global_instance;
}

void ComplementaryStyle::polish(QWidget *widget)
{
    QProxyStyle::polish(widget);
    if (auto button = qobject_cast<QAbstractButton *>(widget)) {
        //fix push button use as icon caused color issues
        if (! button->property("isIcon").toBool())
        {
            button->setProperty("useIconHighlightEffect", true);
            button->setProperty("iconHighlightEffectMode", 1);
            button->setProperty("fillIconSymbolicColor", true);
        }
        return;
    }
    if (auto menu = qobject_cast<QMenu *>(widget)) {
        menu->setProperty("useIconHighlightEffect", true);
        menu->setProperty("iconHighlightEffectMode", 1);
        menu->setProperty("fillIconSymbolicColor", true);
        return;
    }
}

int ComplementaryStyle::styleHint(QStyle::StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
{
    switch (hint) {
    case QStyle::SH_Menu_Scrollable: {
        return 1;
    }
    default:
        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
}

void ComplementaryStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (widget && widget->inherits("QMenu")) {
        return QProxyStyle::drawPrimitive(element, option, painter, widget);
    }

    switch (element) {
    case QStyle::PE_FrameFocusRect:
    case QStyle::PE_IndicatorToolBarSeparator: {
        //do not draw toolbar separator and focus rect
        return;
    }
    case PE_IndicatorArrowDown: {
        ToolBar *toolBar = qobject_cast<ToolBar*>(widget->parentWidget());
        LocationBar *locationBar = qobject_cast<LocationBar*>(widget->parentWidget());
        if (toolBar) {
            auto rect = option->rect;
            rect.setY((rect.top()+rect.bottom())/2 - 8);
            rect.setWidth(16);
            rect.setHeight(16);
            if (option->state & State_Sunken) {
                rect.adjust(1, 1, 1, 1);
            }
            QIcon arrowDown = QIcon::fromTheme("lingmo-down-symbolic", QIcon(":/icons/lingmo-down-symbolic"));
            arrowDown.paint(painter, rect.adjusted(-3, 0, -3, 0));
            return;
        } else if (locationBar) {
            auto rect = option->rect;
            rect.setY((rect.top()+rect.bottom())/2 - 8);
            rect.setX((rect.left()+rect.right())/2 - 8);
            rect.setWidth(16);
            rect.setHeight(16);
            if (option->state & State_Sunken) {
                QIcon arrowDown = QIcon::fromTheme("lingmo-down-symbolic", QIcon(":/icons/lingmo-down-symbolic"));
                arrowDown.paint(painter, rect.adjusted(1, 1, 1, 1));
            } else {
                QIcon arrowDown = QIcon::fromTheme("lingmo-end-symbolic", QIcon(":/icons/lingmo-end-symbolic"));
                arrowDown.paint(painter, rect.adjusted(1, 1, 1, 1));
            }
            return;
        }
        break;
    }
    default:
        break;
    }
    return QProxyStyle::drawPrimitive(element, option, painter, widget);
}

void ComplementaryStyle::drawComplexControl(QStyle::ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p, const QWidget *widget) const
{
    switch (cc) {
    case QStyle::CC_ToolButton: {
        if (const QStyleOptionToolButton *toolbutton
                = qstyleoption_cast<const QStyleOptionToolButton *>(opt)) {

            //Peony::ToolBar has different style with other toolbar.
            ToolBar *parentWidget = qobject_cast<ToolBar*>(widget->parentWidget());

            QRect button, menuarea;
            button = proxy()->subControlRect(cc, toolbutton, SC_ToolButton, widget);
            menuarea = proxy()->subControlRect(cc, toolbutton, SC_ToolButtonMenu, widget);

            State bflags = toolbutton->state & ~State_Sunken;

            if (bflags & State_AutoRaise) {
                if (!(bflags & State_MouseOver) || !(bflags & State_Enabled)) {
                    bflags &= ~State_Raised;
                }
            }
            State mflags = bflags;
            if (toolbutton->state & State_Sunken) {
                if (toolbutton->activeSubControls & SC_ToolButton)
                    bflags |= State_Sunken;
                mflags |= State_Sunken;
            }

            QStyleOption tool = *toolbutton;
            if (toolbutton->subControls & SC_ToolButton) {
                if (bflags & (State_Sunken | State_On | State_Raised)) {
                    tool.rect = button;
                    tool.state = bflags;
                    if (parentWidget) {
                        if (toolbutton->features.testFlag(QStyleOptionToolButton::Menu)) {
                            tool.rect = QRect(button.topLeft(), QSize(button.width() + menuarea.width(), button.height()));
                        }
                        proxy()->drawPrimitive(PE_PanelButtonTool, &tool, p, widget);
                    } else {
                        proxy()->drawPrimitive(PE_PanelButtonTool, &tool, p, widget);
                    }
                } else {
                    if (parentWidget) {
                        tool.rect = button;
                        tool.state = bflags;
                        if (toolbutton->features.testFlag(QStyleOptionToolButton::HasMenu)) {
                            tool.rect = QRect(button.topLeft(), QSize(button.width() + menuarea.width(), button.height()));
                        }

                        qDrawShadePanel(p,
                                        tool.rect.adjusted(1, 1, -1, -1),
                                        m_styled_menu->palette().button().color(),
                                        false,
                                        0,
                                        &m_styled_menu->palette().brush(QPalette::Normal, QPalette::Button));
                    }
                }
            }

            if (toolbutton->state & State_HasFocus) {
                QStyleOptionFocusRect fr;
                fr.QStyleOption::operator=(*toolbutton);
                fr.rect.adjust(3, 3, -3, -3);
                if (toolbutton->features & QStyleOptionToolButton::MenuButtonPopup)
                    fr.rect.adjust(0, 0, -proxy()->pixelMetric(QStyle::PM_MenuButtonIndicator,
                                   toolbutton, widget), 0);
                proxy()->drawPrimitive(PE_FrameFocusRect, &fr, p, widget);
            }
            QStyleOptionToolButton label = *toolbutton;
            label.state = bflags;
            int fw = proxy()->pixelMetric(PM_DefaultFrameWidth, opt, widget);
            label.rect = button.adjusted(fw, fw, -fw, -fw);
            label.palette.setColor(QPalette::Window, m_styled_menu->palette().window().color());

            proxy()->drawControl(CE_ToolButtonLabel, &label, p, widget);

            if (toolbutton->subControls & SC_ToolButtonMenu) {
                tool.rect = menuarea;//.adjusted(1, 1, -1, -1);
                tool.state = mflags;
                tool.palette.setColor(QPalette::Button, m_styled_button->palette().highlight().color());
                //FIXME: paint indicator button background correctly.
                if (mflags & (State_Sunken | State_On | State_Raised)) {
                    if (parentWidget) {
                        //do nothing
                    } else {
                        proxy()->drawPrimitive(PE_PanelButtonCommand, &tool, p, widget);
                    }

                }
                drawPrimitive(PE_IndicatorArrowDown, &tool, p, widget);
            } else if (toolbutton->features & QStyleOptionToolButton::HasMenu) {
                int mbi = proxy()->pixelMetric(PM_MenuButtonIndicator, toolbutton, widget);
                QRect ir = toolbutton->rect;
                QStyleOptionToolButton newBtn = *toolbutton;
                newBtn.rect = QRect(ir.right() + 5 - mbi, ir.y() + ir.height() - mbi + 4, mbi - 6, mbi - 6);
                newBtn.rect = visualRect(toolbutton->direction, button, newBtn.rect);
                drawPrimitive(PE_IndicatorArrowDown, &newBtn, p, widget);
            }
        }
        break;
    }
    default:
        return QProxyStyle::drawComplexControl(cc, opt, p, widget);
    }
}
