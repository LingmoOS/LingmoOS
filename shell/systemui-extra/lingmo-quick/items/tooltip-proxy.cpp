/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#include "tooltip-proxy.h"

#include <QCursor>
#include <QToolTip>

namespace LingmoUIQuick {

ToolTip::ToolTip(QObject *parent) : QObject(parent)
{

}

QString ToolTip::text() const
{
    return m_text;
}

void ToolTip::setText(const QString &text)
{
    if (text == m_text) {
        return;
    }

    m_text = text;
    Q_EMIT textChanged();
}

void ToolTip::show(int x, int y)
{
    show(QPoint(x, y));
}

void ToolTip::show(QPointF point)
{
    show(QPoint(point.x(), point.y()));
}

void ToolTip::show(QPoint point)
{
    if (point.isNull()) {
        QToolTip::showText(QCursor::pos(), m_text);

    } else {
        QToolTip::showText(point, m_text);
    }
}

void ToolTip::hide()
{
    QToolTip::hideText();
}

// ====== ToolTipAttached ====== //
ToolTip *ToolTipAttached::qmlAttachedProperties(QObject *object)
{
    return new ToolTip(object);
}

} // LingmoUIQuick
