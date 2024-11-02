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

#ifndef LINGMO_QUICK_ITEMS_MARGIN_H
#define LINGMO_QUICK_ITEMS_MARGIN_H

#include <QObject>

namespace LingmoUIQuick {

class Margin : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int left READ left WRITE setLeft NOTIFY leftChanged)
    Q_PROPERTY(int top READ top WRITE setTop NOTIFY topChanged)
    Q_PROPERTY(int right READ right WRITE setRight NOTIFY rightChanged)
    Q_PROPERTY(int bottom READ bottom WRITE setBottom NOTIFY bottomChanged)
public:
    explicit Margin(QObject *parent = nullptr);
    Margin(int l, int t, int r, int b, QObject *parent = nullptr);

    int left() const;
    void setLeft(int l);

    int top() const;
    void setTop(int t);

    int right() const;
    void setRight(int r);

    int bottom() const;
    void setBottom(int b);

Q_SIGNALS:
    void leftChanged();
    void topChanged();
    void rightChanged();
    void bottomChanged();

private:
    int m_left {0};
    int m_top {0};
    int m_right {0};
    int m_bottom {0};
};

} // LingmoUIQuick

#endif //LINGMO_QUICK_ITEMS_MARGIN_H
