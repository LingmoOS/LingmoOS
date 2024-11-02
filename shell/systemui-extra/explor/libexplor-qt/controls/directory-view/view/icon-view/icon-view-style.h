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

#ifndef ICONVIEWSTYLE_H
#define ICONVIEWSTYLE_H

#include <QProxyStyle>
#include <explor-core_global.h>

namespace Peony {

namespace DirectoryView {

/*!
 * \brief The IconViewStyle class
 * \details
 * This class is aim to provide a custom style for IconView's items.
 * In Qt5, item view draw its items from styled delegate by default,
 * even though we can implement a item's painting in delegate's paint().
 * I think it is better to follow Qt's desgin.
 * I mainly use this class control the internal layout of a item. It will
 * combine with delegate's paint() method.
 */
class PEONYCORESHARED_EXPORT IconViewStyle : public QProxyStyle
{
    Q_OBJECT
public:
    static IconViewStyle *getStyle();
    void release();

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
    explicit IconViewStyle(QStyle *style = nullptr);
    ~IconViewStyle() override {}
};

}

}

#endif // ICONVIEWSTYLE_H
