/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
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

#ifndef PEONYCOMPUTERVIEWPLUGIN_H
#define PEONYCOMPUTERVIEWPLUGIN_H

#include "explor-extension-computer-view_global.h"
#include <explor-qt/directory-view-plugin-iface2.h>

namespace Peony {

class PEONYQTEXTENSIONCOMPUTERVIEW_EXPORT PeonyComputerViewPlugin : public QObject, public Peony::DirectoryViewPluginIface2
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DirectoryViewPluginIface2_iid FILE "common.json")
    Q_INTERFACES(Peony::DirectoryViewPluginIface2)
public:
    explicit PeonyComputerViewPlugin(QObject *parent = nullptr);

    //common
    PluginType pluginType() {return PluginType::DirectoryViewPlugin2;}

    const QString name() {return QObject::tr("Computer View");}
    const QString description() {return QObject::tr("Show drives, network and personal directories");}
    const QIcon icon() {return QIcon::fromTheme("computer");}
    void setEnable(bool enable) {}
    bool isEnable() {return true;}

    //view
    QString viewIdentity() {return "Computer View";}
    QString viewName() {return name();}
    QIcon viewIcon() {return icon();}
    bool supportUri(const QString &uri) {return uri == "computer:///";}

    int zoom_level_hint() {return -1;}
    int minimumSupportedZoomLevel() {return -1;}
    int maximumSupportedZoomLevel() {return -1;}

    int priority(const QString &directoryUri);
    bool supportZoom() {return false;}

    DirectoryViewWidget *create();
};

}

#endif // PEONYCOMPUTERVIEWPLUGIN_H
