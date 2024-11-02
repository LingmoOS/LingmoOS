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

#include "icon-view-style.h"

#include <QStyleOptionRubberBand>
#include <QPainter>

#include <QApplication>

#include <QDebug>

using namespace Peony;
using namespace Peony::DirectoryView;

static IconViewStyle *global_instance = nullptr;

IconViewStyle::IconViewStyle(QStyle *style) : QProxyStyle(style)
{
    //qDebug()<<"icon view style";
}

IconViewStyle *IconViewStyle::getStyle()
{
    if (!global_instance)
        global_instance = new IconViewStyle;
    return global_instance;
}

void IconViewStyle::release()
{
    if (global_instance)
        global_instance->deleteLater();
}

void IconViewStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (element == PE_Frame) {
        return;
    }
    return qApp->style()->drawPrimitive(element, option, painter, widget);
}

void IconViewStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (element == CE_RubberBand) {
        if (qApp->devicePixelRatio() != 1.0) {
            const QStyleOptionRubberBand *tmp = qstyleoption_cast<const QStyleOptionRubberBand *>(option);
            QStyleOptionRubberBand opt = *tmp;
            if (opt.rect.width() >= 2 && opt.rect.height() >= 2) {
                opt.rect.adjust(1, 1, -1, -1);
            } else {
                opt.rect = QRect();
            }
            return qApp->style()->drawControl(element, &opt, painter, widget);
        }
    }
    qApp->style()->drawControl(element, option, painter, widget);
}

void IconViewStyle::drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const
{
    //qDebug()<<"drawItemPixmap";
    qApp->style()->drawItemPixmap(painter, rect, alignment, pixmap);
}

void IconViewStyle::drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled, const QString &text, QPalette::ColorRole textRole) const
{
    //qDebug()<<"drawItemText";
    qApp->style()->drawItemText(painter, rect, flags, pal, enabled, text, textRole);
}
