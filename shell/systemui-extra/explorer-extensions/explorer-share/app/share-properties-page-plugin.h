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

#ifndef SHAREMENUPLUGIN_H
#define SHAREMENUPLUGIN_H

#include <QObject>

#include <properties-window-tab-page-plugin-iface.h>

#include "usershare-manager.h"

namespace Peony {

class SharePropertiesPagePlugin : public QObject, public PropertiesWindowTabPagePluginIface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PropertiesWindowTabPagePluginIface_iid FILE "common.json")
    Q_INTERFACES(Peony::PropertiesWindowTabPagePluginIface)
public:
    explicit SharePropertiesPagePlugin(QObject *parent = nullptr);
    static SharePropertiesPagePlugin *getInstance();

    PluginInterface::PluginType pluginType() override {return PluginInterface::PropertiesWindowPlugin;}
    const QString name() override {return tr("Peony Qt Share Extension");}
    const QString description() override {return tr("Allow user share folders");}
    const QIcon icon() override {return QIcon::fromTheme("emblem-shared");}
    void setEnable(bool enable) override {m_enable = enable;}
    bool isEnable() override {return m_enable;}

    int tabOrder() override {return 99;}
    bool supportUris(const QStringList &uris) override;
    PropertiesWindowTabIface * createTabPage(const QStringList &uris) override;

    void closeFactory() {deleteLater();}

private:
    bool            m_enable;
};

}

#endif // SHAREMENUPLUGIN_H
