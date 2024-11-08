/*
 * Peony-Qt's Extension
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

#ifndef MENUPLUGIN_TEST1
#define MENUPLUGIN_TEST1

#include <QObject>
#include <QtPlugin>
#include "menu-plugin-iface.h"

namespace Peony {

class MenuPluginTest1 : public QObject, public MenuPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID MenuPluginInterface_iid FILE "menuplugintest1.json")
    //Q_PLUGIN_METADATA(IID MenuInterface_iid)
    Q_INTERFACES(Peony::MenuPluginInterface)

public:
    PluginInterface::PluginType pluginType() override {
        return PluginInterface::MenuPlugin;
    }
    const QString name() override {
        return "testMenuPlugin1";
    }
    const QString description() override {
        return "This is a menu type test plugin";
    }
    const QIcon icon() override {
        return QIcon::fromTheme("search");
    }
    void setEnable(bool enable) override {
        m_enable = enable;
    }
    bool isEnable() override {
        return m_enable;
    }

    QString testPlugin() override;
    QList<QAction *> menuActions(Types types, const QString &uri, const QStringList &selectionUris) override;

private:
    bool m_enable;
};

}

#endif
