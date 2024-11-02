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

#include "margin.h"

namespace LingmoUIQuick {

Margin::Margin(QObject *parent) : QObject(parent)
{

}

Margin::Margin(int l, int t, int r, int b, QObject *parent) : QObject(parent), m_left(l), m_top(t), m_right(r), m_bottom(b)
{

}

int Margin::left() const
{
    return m_left;
}

void Margin::setLeft(int l)
{
    if (l == m_left) {
        return;
    }

    m_left = l;
    Q_EMIT leftChanged();
}

int Margin::top() const
{
    return m_top;
}

void Margin::setTop(int t)
{
    if (t == m_top) {
        return;
    }

    m_top = t;
    Q_EMIT topChanged();
}

int Margin::right() const
{
    return m_right;
}

void Margin::setRight(int r)
{
    if (r == m_right) {
        return;
    }

    m_right = r;
    Q_EMIT rightChanged();
}

int Margin::bottom() const
{
    return m_bottom;
}

void Margin::setBottom(int b)
{
    if (b == m_bottom) {
        return;
    }

    m_bottom = b;
    Q_EMIT bottomChanged();
}

} // LingmoUIQuick