/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2021, KylinSoft Co., Ltd.
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

#ifndef SIDEBARPLUGINIFACE_H
#define SIDEBARPLUGINIFACE_H

#include "side-bar.h"
#include "plugin-iface.h"

#include <QPluginLoader>
#include <QtPlugin>
#include <QStringList>

#define SideBarPluginInterface_iid "org.lingmo.explor-qt.plugin-iface.SideBarPluginInterface"

namespace Peony {

class SideBarPluginInterface : public PluginInterface
{
public:
    virtual ~SideBarPluginInterface() {}

    /*!
     * \brief keys
     * usually a special string for recognizing.
     */
    virtual const QStringList keys() = 0;

    /*!
     * \brief create
     * \return
     * a SideBar instance with its own implement.
     */
    virtual SideBar *create(QWidget *parent) = 0;
};

}

Q_DECLARE_INTERFACE (Peony::SideBarPluginInterface, SideBarPluginInterface_iid)

#endif // SIDEBARPLUGINIFACE_H
