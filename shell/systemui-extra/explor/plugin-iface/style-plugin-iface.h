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

#ifndef STYLEPLUGINIFACE_H
#define STYLEPLUGINIFACE_H

#include <QPluginLoader>
#include <QtPlugin>
#include <QString>

#include "plugin-iface.h"

#define StylePluginIface_iid "org.lingmo.explor-qt.plugin-iface.StylePluginInterface"

class QProxyStyle;

namespace Peony {

class StylePluginIface : public PluginInterface
{
public:
    ~StylePluginIface() {}

    virtual int defaultPriority() = 0;

    virtual QProxyStyle *getStyle() = 0;
};

}

Q_DECLARE_INTERFACE(Peony::StylePluginIface, StylePluginIface_iid)

#endif // STYLEPLUGINIFACE_H
