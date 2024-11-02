/*
 * Peony-Qt's Library
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
 */

#ifndef COMPUTERVIEWSTYLE_H
#define COMPUTERVIEWSTYLE_H
#include <QProxyStyle>
#include <explor-core_global.h>

class ComputerViewStyle : public QProxyStyle
{
public:
    static ComputerViewStyle *getStyle();

    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const override;

    void drawControl(QStyle::ControlElement element,
                     const QStyleOption *option,
                     QPainter *painter,
                     const QWidget *widget = nullptr) const override;

    void drawItemPixmap(QPainter *painter,
                        const QRect &rect,
                        int alignment,
                        const QPixmap &pixmap) const override;

    void drawItemText(QPainter *painter,
                      const QRect &rect,
                      int flags,
                      const QPalette &pal,
                      bool enabled,
                      const QString &text,
                      QPalette::ColorRole textRole = QPalette::NoRole) const override;

private:
    explicit ComputerViewStyle(QStyle *style = nullptr);
    ~ComputerViewStyle() override {}
};

#endif // COMPUTERVIEWSTYLE_H
