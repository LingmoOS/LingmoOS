/*
 * Peony-Qt
 *
 * Copyright (C) 2023, KylinSoft Information Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef VFSPLUGINIFACE_H
#define VFSPLUGINIFACE_H

#include "plugin-iface.h"
#include "explor-core_global.h"

#include "custom-error-handler.h"

#define VFSPluginInterface_iid "org.lingmo.explor-qt.plugin-iface.VFSPluginInterface"

namespace Peony {
class PEONYCORESHARED_EXPORT VFSPluginIface : public PluginInterface {
public:
    virtual ~VFSPluginIface() {}

    /*!
     * \brief uriScheme
     * \return
     * \details
     * indicate the uri scheme you want to register.
     */
    virtual QString uriScheme() = 0;

    /*!
     * \brief holdInSideBar
     * \return
     * \details
     * indicate if it should display as an parent item in sidebar.
     */
    virtual bool holdInSideBar() = 0;

    /*!
     * \brief initVFS
     * \details
     * explor will init your custom app vfs with this interface at early stage.
     * you should call g_vfs_register_uri_scheme() in your own implement to register
     * your vfs into explor.
     * \note
     * you should implement your vfs registion with your uri scheme.
     */
    virtual void initVFS() = 0;

    /*!
     * \brief parseUriToVFSFile
     * \param uri
     * \return a GFile Handler represent crosponding uri.
     * \details
     * backup function for while register costom uri scheme is not supported.
     * \note
     * in old glib version, there is no g_vfs_register_uri_scheme(), and in new
     * glib, register a vfs into system is asynchronous, those means it might not be
     * registered and can not use GVfsFileLookupFunc to get file handler.
     */
    virtual void* parseUriToVFSFile(const QString &uri) = 0;

    /*!
     * \brief customErrorHandler
     * \return
     * custom error handler for this vfs. normally it using with FileEnumerator::prepare(),
     * and FileEnumerator::handleError().
     */
    virtual CustomErrorHandler *customErrorHandler() = 0;
};
}

Q_DECLARE_INTERFACE(Peony::VFSPluginIface, VFSPluginInterface_iid)

#endif // VFSPLUGINIFACE_H
