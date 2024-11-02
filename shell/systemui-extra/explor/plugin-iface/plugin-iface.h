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

#ifndef PLUGINIFACE_H
#define PLUGINIFACE_H

#include <QString>
#include <QIcon>

//#define PluginInterface_iid "org.lingmo.explor-qt.PluginInterface"

namespace Peony {

class PluginInterface
{
public:
    enum PluginType
    {
        Invalid,
        MenuPlugin,
        PreviewPagePlugin,
        DirectoryViewPlugin,
        DirectoryViewPlugin2,
        ToolBarPlugin,
        PropertiesWindowPlugin,
        ColumnProviderPlugin,
        StylePlugin,
        VFSPlugin,
        EmblemPlugin,
        SideBarPlugin,
        VFSINFOPlugin,
        Other
    };

    virtual ~PluginInterface() {}

    virtual PluginType pluginType() = 0;

    virtual const QString name() = 0;
    virtual const QString description() = 0;
    virtual const QIcon icon() = 0;
    virtual void setEnable(bool enable) = 0;
    virtual bool isEnable() = 0;
};

}

#endif // PLUGINIFACE_H
