/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2019, Tianjin LINGMO Information Technology Co., Ltd.
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

#ifndef PROPERTIESWINDOWTABPAGEEXAMPLEFACTORY_H
#define PROPERTIESWINDOWTABPAGEEXAMPLEFACTORY_H

#include "properties-window-tab-page-plugin-example_global.h"

#include "properties-window-tab-page-plugin-iface.h"
#include <QObject>

class PROPERTIESWINDOWTABPAGEPLUGINEXAMPLESHARED_EXPORT PropertiesWindowTabPageExampleFactory :
    public QObject, public Peony::PropertiesWindowTabPagePluginIface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PropertiesWindowTabPagePluginIface_iid)
    Q_INTERFACES(Peony::PropertiesWindowTabPagePluginIface)
public:
    explicit PropertiesWindowTabPageExampleFactory(QObject *parent = nullptr);
    ~PropertiesWindowTabPageExampleFactory() override;

    PluginInterface::PluginType pluginType() override {
        return PluginInterface::PropertiesWindowPlugin;
    }
    const QString name() override {
        return "Properties Page Plugin Example";
    }
    const QString description() override {
        return "This plugin is a factory plugin providing the add-on properties page for explor-qt's properties window";
    }
    const QIcon icon() override {
        return QIcon::fromTheme("view-paged-symbolic");
    }
    void setEnable(bool enable) override {
        m_enable = enable;
    }
    bool isEnable() override {
        return m_enable;
    }

    int tabOrder() override {
        return 0;
    }
    bool supportUris(const QStringList &uris) override {
        return true;
    }
    QWidget *createTabPage(const QStringList &uris) override;

    void closeFactory() override {
        deleteLater();
    }

private:
    bool m_enable = true;
};

#endif // PROPERTIESWINDOWTABPAGEEXAMPLEFACTORY_H
