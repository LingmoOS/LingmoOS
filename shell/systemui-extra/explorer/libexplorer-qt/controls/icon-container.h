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

#ifndef ICONCONTAINER_H
#define ICONCONTAINER_H

#include <QPushButton>
#include <QProxyStyle>

namespace Peony {

class IconContainer : public QPushButton
{
    Q_OBJECT
public:
    explicit IconContainer(QWidget *parent = nullptr);
    ~IconContainer();

protected:
    void mouseMoveEvent(QMouseEvent *e) {}
    void mousePressEvent(QMouseEvent *e) {}
    void paintEvent(QPaintEvent *e);

private:
    QStyle *m_style;
};

class IconContainerStyle : public QProxyStyle
{
    friend class IconContainer;
    explicit IconContainerStyle();

    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const override;
};

#endif // ICONCONTAINER_H

}
