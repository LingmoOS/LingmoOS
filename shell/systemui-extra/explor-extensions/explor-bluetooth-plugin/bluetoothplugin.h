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
 * Authors: tang guang <tangguang@kylinos.cn>
 *
 */

#ifndef BLUETOOTHPLUGIN_H
#define BLUETOOTHPLUGIN_H

#include <file-info.h>
#include <file-utils.h>

#include "explor-bluetooth-plugin_global.h"
#include <menu-plugin-iface.h>

#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>
#include <QString>
#include <QStringList>
#include <QAction>
#include <QTranslator>
#include <QLocale>
#include <QApplication>
#include <QDebug>

namespace Peony {

class PEONYQTENGRAMPAMENUPLUGINSHARED_EXPORT BluetoothPlugin: public QObject, public MenuPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID MenuPluginInterface_iid FILE "common.json")
    Q_INTERFACES(Peony::MenuPluginInterface)
public:
    explicit BluetoothPlugin(QObject *parent = nullptr);

    PluginInterface::PluginType pluginType() override {return PluginInterface::MenuPlugin;}
    const QString name() override {return tr("Peony-Qt bluetooth Extension");}
    const QString description() override {return tr("bluetooth Menu Extension");}
    const QIcon icon() override {return QIcon::fromTheme("blueman-tray");}
    void setEnable(bool enable) override {m_enable = enable;}
    bool isEnable() override {return m_enable;}

    QString testPlugin() override {return "test compress";}
    QList<QAction *> menuActions(Types types, const QString &uri, const QStringList &selectionUris) override;

private:
    bool m_enable;
};

}
#endif // BLUETOOTHPLUGIN_H
