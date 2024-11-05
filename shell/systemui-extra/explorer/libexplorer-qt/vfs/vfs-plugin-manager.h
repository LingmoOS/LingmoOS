/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#ifndef VFSPLUGINMANAGER_H
#define VFSPLUGINMANAGER_H

#include "vfs-plugin-iface.h"

#include <gio/gio.h>

#include <QObject>

namespace Peony {

class PEONYCORESHARED_EXPORT VFSPluginManager : public QObject
{
    Q_OBJECT
public:
    static VFSPluginManager *getInstance();

    void registerPlugin(VFSPluginIface *plugin);

    void unregisterPlugin(VFSPluginIface *plugin);

    QList<VFSPluginIface *> registeredPlugins();

    /*!
     * \brief supportExtraSchemes
     * \return the registered plugins schemes, like "search://".
     */
    const QStringList supportExtraSchemes();

    /*!
     * \brief newVFSFile
     * construct a vfs file implemented by plugin.
     * \param uri
     * \return
     * \see supportExtraSchemes
     */
    GFile *newVFSFile(const QString &uri);

Q_SIGNALS:
    void updateVFSPlugin(VFSPluginIface *newPlugin, bool enable);

private:
    explicit VFSPluginManager(QObject *parent = nullptr);

    QList<VFSPluginIface *> m_plugins;
    QStringList m_support_schemes;
};

}

#endif // VFSPLUGINMANAGER_H
