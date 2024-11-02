/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
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

#include "list-view-style.h"

#include <QStyleOption>
#include <QPainterPath>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QTextLayout>

#include <QApplication>

using namespace Peony;
using namespace Peony::DirectoryView;

static ListViewStyle *global_instance = nullptr;

ListViewStyle::ListViewStyle(QObject *parent) : QProxyStyle()
{

}

ListViewStyle *ListViewStyle::getStyle()
{
    if (!global_instance)
        global_instance = new ListViewStyle;
    return global_instance;
}

void ListViewStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch (element) {
    case PE_Frame: {
        painter->save();
        bool isActive = option->state & State_Active;
        bool isEnable = option->state & State_Enabled;
        auto baseColor = option->palette.color(isEnable? (isActive? QPalette::Active: QPalette::Inactive): QPalette::Disabled, QPalette::Window);
        painter->fillRect(widget->rect(), baseColor);
        painter->restore();
        return;
    }
    case PE_FrameWindow: {
        return;
    }
    case PE_IndicatorBranch: {
        const QStyleOptionViewItem *tmp = qstyleoption_cast<const QStyleOptionViewItem *>(option);
        QStyleOptionViewItem opt = *tmp;
        if (!opt.state.testFlag(QStyle::State_Selected)) {
            if (opt.state & QStyle::State_Sunken) {
                opt.palette.setColor(QPalette::Highlight, opt.palette.button().color());
            }
            if (opt.state & QStyle::State_MouseOver) {
                opt.palette.setColor(QPalette::Highlight, opt.palette.mid().color());
            }
        }
        return qApp->style()->drawPrimitive(element, &opt, painter, widget);
    }
    case QStyle::PE_IndicatorItemViewItemDrop: {
        if (option->rect.height() <= 1) {
            // 不显示中间线条，规避#85608，另外偶现错位的问题暂时没有定位，有可能是因为重写了updateGeometries导致的
            return;
        }
        painter->save();
        //fix bug#147348
        painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform, true);/* 反锯齿 */
        /* 按设计要求，边框颜色为调色板highlight值，圆角为6px */
        QColor color = option->palette.color(QPalette::Highlight);
        painter->setPen(color);
        painter->drawRoundedRect(option->rect, 6, 6);
        painter->restore();
        return;
    }
    default:
        return qApp->style()->drawPrimitive(element, option, painter, widget);
    }
}

