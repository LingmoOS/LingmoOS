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

#ifndef MATETERMINALMENUPLUGIN_H
#define MATETERMINALMENUPLUGIN_H

#include "explor-qt-menu-plugin-mate-terminal_global.h"

#include <QObject>
#include <QtPlugin>

#include "menu-plugin-iface.h"

namespace Peony {

class PEONYQTMENUPLUGINMATETERMINALSHARED_EXPORT MateTerminalMenuPlugin : public QObject, public MenuPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID MenuPluginInterface_iid)
    Q_INTERFACES(Peony::MenuPluginInterface)
public:
    explicit MateTerminalMenuPlugin(QObject *parent = nullptr);

    PluginInterface::PluginType pluginType() override {
        return PluginInterface::MenuPlugin;
    }
    const QString name() override {
        return tr("Peony-Qt Mate Terminal Menu Extension");
    }
    const QString description() override {
        return tr("Open Terminal with menu.");
    }
    const QIcon icon() override {
        return QIcon::fromTheme("utilities-terminal-symbolic");
    }
    void setEnable(bool enable) override {
        m_enable = enable;
    }
    bool isEnable() override {
        return m_enable;
    }

    QString testPlugin() override {
        return "test";
    }
    QList<QAction *> menuActions(Types types, const QString &uri, const QStringList &selectionUris) override;

private:
    bool m_enable;
};

}

#endif // MATETERMINALMENUPLUGIN_H
