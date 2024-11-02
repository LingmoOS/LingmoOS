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

#ifndef COMPLEMENTARYSTYLE_H
#define COMPLEMENTARYSTYLE_H

#include <QProxyStyle>
#include "explor-core_global.h"

class QMenu;
class QPushButton;

namespace Peony {

/*!
 * \brief The ComplementaryStyle class
 * \details
 * This class provide a fixed style for painting qt's control
 * with system theme which qt5-gtk2-platformtheme provided.
 *
 * If just use the qpa plugin, there were be some incorrect styled
 * controls. Such as QToolBarButton's indicator.
 *
 * \note
 * If you are not using gtk-theme as default system theme, you should not use this
 * proxy style for painting.
 *
 * \todo
 * add border radius support.
 */
class ComplementaryStyle : public QProxyStyle
{
    Q_OBJECT
public:
    static PEONYCORESHARED_EXPORT ComplementaryStyle *getStyle();

    void polish(QWidget *widget);

    int styleHint(QStyle::StyleHint hint,
                  const QStyleOption *option = nullptr,
                  const QWidget *widget = nullptr,
                  QStyleHintReturn *returnData = nullptr) const;

    void drawPrimitive(QStyle::PrimitiveElement element,
                       const QStyleOption *option,
                       QPainter *painter,
                       const QWidget *widget = nullptr) const;

    void drawComplexControl(QStyle::ComplexControl cc,
                            const QStyleOptionComplex *opt,
                            QPainter *p,
                            const QWidget *widget = nullptr) const;

private:
    explicit ComplementaryStyle(QStyle *parent = nullptr);
    ~ComplementaryStyle();

    QMenu *m_styled_menu;
    QPushButton *m_styled_button;
};

}

#endif // COMPLEMENTARYSTYLE_H
