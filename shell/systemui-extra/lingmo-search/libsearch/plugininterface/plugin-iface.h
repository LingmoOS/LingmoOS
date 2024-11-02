/*
 *
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
 *
 */
#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

namespace LingmoUISearch {

class PluginInterface
{
public:
    enum class PluginType
    {
        MenuPlugin,
        PreviewPagePlugin,
        SearchPlugin, // for lingmo-search itself.
        SearchTaskPlugin, // for search service.
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
#endif // PLUGININTERFACE_H
