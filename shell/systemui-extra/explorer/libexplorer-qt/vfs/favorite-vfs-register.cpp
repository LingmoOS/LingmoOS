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
 * Authors: Ding Jing <dingjing@kylinos.cn>
 *
 */

#include "favorite-vfs-file.h"
#include "favorite-vfs-register.h"
#include "favorite-vfs-file-enumerator.h"

#include <gio/gio.h>
#include <QDebug>

using namespace Peony;

bool favorite_is_registed = false;

static GFile* test_vfs_parse_name (GVfs* vfs, const char* parseName, gpointer udata)
{
    return vfs_favorite_file_new_for_uri(parseName);
}

static GFile * test_vfs_lookup (GVfs* vfs, const char* uri, gpointer udata)
{
    return test_vfs_parse_name(vfs, uri, udata);
}

void Peony::FavoriteVFSRegister::registFavoriteVFS()
{
    if (favorite_is_registed) {
        return;
    }

    GVfs* vfs = nullptr;
    gboolean res = false;
    const gchar * const *schemes;

    vfs = g_vfs_get_default ();
    schemes = g_vfs_get_supported_uri_schemes (vfs);

    const gchar* const* p = schemes;
    while (*p) {
        qDebug() <<"registFavoriteVFS:" << *p;
        p++;
    }

#if GLIB_CHECK_VERSION(2, 50, 0)
    res = g_vfs_register_uri_scheme (vfs, "favorite", test_vfs_lookup, NULL, NULL, test_vfs_parse_name, NULL, NULL);
#else
#endif
}

Peony::FavoriteVFSRegister::FavoriteVFSRegister()
{

}

void FavoriteVFSInternalPlugin::setEnable(bool enable)
{
    Q_UNUSED(enable)
}

void FavoriteVFSInternalPlugin::initVFS()
{
    FavoriteVFSRegister::registFavoriteVFS();
}

void* FavoriteVFSInternalPlugin::parseUriToVFSFile(const QString &uri)
{
    return vfs_favorite_file_new_for_uri(uri.toUtf8());
}
