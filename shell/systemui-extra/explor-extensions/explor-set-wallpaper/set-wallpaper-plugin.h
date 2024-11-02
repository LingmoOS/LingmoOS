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
 * Authors: Meihong <hemeihong@kylinos.cn>
 *
 */

#ifndef SETWALLPAPERPLUGIN_H
#define SETWALLPAPERPLUGIN_H

#include "explor-set-wallpaper_global.h"
#include <menu-plugin-iface.h>

#include <QGSettings>

namespace Peony {

class PEONYQTSETWALLPAPERSHARED_EXPORT SetWallPaperPlugin: public QObject, public MenuPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID MenuPluginInterface_iid FILE "common.json")
    Q_INTERFACES(Peony::MenuPluginInterface)
public:
    explicit SetWallPaperPlugin(QObject *parent = nullptr);

    PluginInterface::PluginType pluginType() override {return PluginInterface::MenuPlugin;}
    const QString name() override {return tr("Peony-Qt set wallpaper Extension");}
    const QString description() override {return tr("set wallpaper Extension");}
    const QIcon icon() override {return QIcon::fromTheme("preferences-desktop-wallpaper-symbolic");}
    void setEnable(bool enable) override {m_enable = enable;}
    bool isEnable() override {return m_enable;}

    QString testPlugin() override {return "test set wallpaper";}
    QList<QAction *> menuActions(Types types, const QString &uri, const QStringList &selectionUris) override;

    bool is_picture_file(QString file_name);

private:
    bool m_enable;
    QStringList m_picture_type_list = {"png", "jpg", "jpeg", "bmp", "dib", "jfif", "jpe", "gif", "tif", "tiff", "wdp"};

    QGSettings *m_bg_settings = nullptr;
};

}

#endif // SETWALLPAPERPLUGIN_H
