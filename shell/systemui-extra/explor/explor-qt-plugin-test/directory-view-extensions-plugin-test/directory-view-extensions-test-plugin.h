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

#ifndef DIRECTORYVIEWEXTENSIONSTESTPLUGIN_H
#define DIRECTORYVIEWEXTENSIONSTESTPLUGIN_H

#include "directory-view-extensions-plugin-test_global.h"
#include "directory-view-plugin-iface2.h"

namespace Peony {

class DIRECTORYVIEWEXTENSIONSPLUGINTESTSHARED_EXPORT DirectoryViewExtensionsTestPlugin : public QObject, public DirectoryViewPluginIface2
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DirectoryViewPluginIface2_iid)
    Q_INTERFACES(Peony::DirectoryViewPluginIface2)
public:
    explicit DirectoryViewExtensionsTestPlugin(QObject *parent = nullptr);

    //plugin implement
    const QString name() override {
        return QObject::tr("Directory View Plugin");
    }
    PluginType pluginType() override {
        return PluginType::DirectoryViewPlugin2;
    }
    const QString description() override {
        return QObject::tr("Show the folder children as icons.");
    }
    const QIcon icon() override {
        return QIcon::fromTheme("folder");
    }
    void setEnable(bool enable) override {
        Q_UNUSED(enable)
    }
    bool isEnable() override {
        return true;
    }

    //directory view plugin implemeny
    QString viewIdentity() override {
        return QObject::tr("Directory View Plugin");
    }
    QIcon viewIcon() override {
        return QIcon::fromTheme("folder");
    }
    bool supportUri(const QString &uri) override {
        return !uri.isEmpty();
    }

    //void fillDirectoryView(DirectoryViewWidget *view) override;

    DirectoryViewWidget *create() override;

    int zoom_level_hint() override {
        return 0;
    }
    int priority(const QString &) override {
        return -1;
    }
};

}

#endif // DIRECTORYVIEWEXTENSIONSTESTPLUGIN_H
