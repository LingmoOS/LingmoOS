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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef INTERNALSTYLE_H
#define INTERNALSTYLE_H

#include <QProxyStyle>

/*!
 * \brief The InternalStyle class
 * \details
 * This class is a interface type class. It is desgined as an extension of LINGMO theme
 * frameworks. Applications which use internal style means that there will be no effect
 * when system theme switched, for example, from fusion to lingmo-white.
 * But an internal style usually should response the palette settings changed for
 * keeping the unity as a desktop environment theme's extensions.
 *
 * The typical example which implement the internal style is MPSStyle.
 */
class InternalStyle : public QProxyStyle
{
    Q_OBJECT
public:
    explicit InternalStyle(QStyle *parentStyle = nullptr);
    explicit InternalStyle(const QString parentStyleName);

signals:
    void useSystemStylePolicyChanged(bool use);

public slots:
    virtual void setUseSystemStyle(bool use);
};

#endif // INTERNALSTYLE_H
