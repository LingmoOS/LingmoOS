/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 */

#ifndef VFSINFOPLUGINMANAGER_H
#define VFSINFOPLUGINMANAGER_H

#include "vfs-info-plugin-iface.h"

#include <gio/gio.h>

#include <QObject>
#include <QMap>

namespace Peony {
class PEONYCORESHARED_EXPORT VFSInfoPluginManager : public QObject
{
    Q_OBJECT
public:
    static VFSInfoPluginManager *getInstance();

    void registerPlugin(VFSInfoPluginIface *plugin);

    VFSInfoPluginIface* userSchemeGetPlugin(QString &scheme);

    const QStringList getAllPluginKeys();

private:
    explicit VFSInfoPluginManager(QObject *parent = nullptr);

    QMap<QString, VFSInfoPluginIface *> m_pluginsMap;
    QStringList m_support_path;
};

}


#endif // VFSINFOPLUGINMANAGER_H
