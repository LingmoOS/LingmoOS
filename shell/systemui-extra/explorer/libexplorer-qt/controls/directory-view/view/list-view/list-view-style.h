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

#ifndef LISTVIEWSTYLE_H
#define LISTVIEWSTYLE_H

#include <QProxyStyle>
#include <QStyleOptionViewItem>

namespace Peony {

namespace DirectoryView {

class ListViewStyle : public QProxyStyle
{
    Q_OBJECT
public:
    static ListViewStyle *getStyle();

    void drawPrimitive(PrimitiveElement element,
                       const QStyleOption *option,
                       QPainter *painter,
                       const QWidget *widget = nullptr) const override;
//    void drawControl(ControlElement ce, const QStyleOption *option, QPainter *painter,
//                     const QWidget *widget) const override;
//    //绘制列表视图文本
//    void viewItemDrawText(QPainter *p, const QStyleOptionViewItem *option, const QRect &rect) const;

private:
    explicit ListViewStyle(QObject *parent = nullptr);
};

}

}

#endif // LISTVIEWSTYLE_H
