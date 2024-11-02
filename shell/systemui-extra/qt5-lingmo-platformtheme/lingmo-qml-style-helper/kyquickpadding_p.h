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
 * Authors: Yan Wang <wangyan@kylinos.cn>
 *
 */

#ifndef KYQUICKPADDING_P_H
#define KYQUICKPADDING_P_H

#include <qobject.h>

class KyQuickPadding : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int left READ left WRITE setLeft NOTIFY leftChanged)
    Q_PROPERTY(int top READ top WRITE setTop NOTIFY topChanged)
    Q_PROPERTY(int right READ right WRITE setRight NOTIFY rightChanged)
    Q_PROPERTY(int bottom READ bottom WRITE setBottom NOTIFY bottomChanged)

    int m_left;
    int m_top;
    int m_right;
    int m_bottom;

public:
    KyQuickPadding(QObject *parent = nullptr) :
        QObject(parent),
        m_left(0),
        m_top(0),
        m_right(0),
        m_bottom(0) {}

    int left() const { return m_left; }
    int top() const { return m_top; }
    int right() const { return m_right; }
    int bottom() const { return m_bottom; }

public Q_SLOTS:
    void setLeft(int arg) { if (m_left != arg) {m_left = arg; emit leftChanged();}}
    void setTop(int arg) { if (m_top != arg) {m_top = arg; emit topChanged();}}
    void setRight(int arg) { if (m_right != arg) {m_right = arg; emit rightChanged();}}
    void setBottom(int arg) {if (m_bottom != arg) {m_bottom = arg; emit bottomChanged();}}

Q_SIGNALS:
    void leftChanged();
    void topChanged();
    void rightChanged();
    void bottomChanged();
};


#endif // KYQUICKPADDING_P_H
