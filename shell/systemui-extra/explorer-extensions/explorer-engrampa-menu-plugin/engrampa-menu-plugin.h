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

#ifndef ENGRAMPAMENUPLUGIN_H
#define ENGRAMPAMENUPLUGIN_H

#include "explorer-engrampa-menu-plugin_global.h"
#include <menu-plugin-iface.h>

namespace Peony {

class PEONYQTENGRAMPAMENUPLUGINSHARED_EXPORT EngrampaMenuPlugin: public QObject, public MenuPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID MenuPluginInterface_iid FILE "common.json")
    Q_INTERFACES(Peony::MenuPluginInterface)
public:
    explicit EngrampaMenuPlugin(QObject *parent = nullptr);

    PluginInterface::PluginType pluginType() override {return PluginInterface::MenuPlugin;}
    const QString name() override {return tr("Peony-Qt engrampa Extension");}
    const QString description() override {return tr("engrampa Menu Extension");}
    const QIcon icon() override {return QIcon::fromTheme("application-zip");}
    void setEnable(bool enable) override {m_enable = enable;}
    bool isEnable() override {return m_enable;}

    QString testPlugin() override {return "test compress";}
    QList<QAction *> menuActions(Types types, const QString &uri, const QStringList &selectionUris) override;

    bool is_compressed_file(QString file_name);

private:
    bool m_enable;
    QStringList m_file_type_list = {"7z","ar","bz","bz2","cbz","deb", "ear","exe","gz","jar","tar","tar.7z",
                                    "tar.bz2","tar.gz","tar.lzma","rar","tar.xz","war","zip","lzma","xz","tgz",
                                    "odp", "ods", "otp", "ott", "ots", "odt", "iso"};
};

}

#endif // ENGRAMPAMENUPLUGIN_H
