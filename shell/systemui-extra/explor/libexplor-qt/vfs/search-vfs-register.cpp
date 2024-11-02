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

#include "search-vfs-register.h"
#include "explor-search-vfs-file.h"
#include "explor-search-vfs-file-enumerator.h"
#include "search-vfs-manager.h"

#include <gio/gio.h>
#include <QDebug>

using namespace Peony;

bool is_registed = false;

static GFile *
test_vfs_parse_name (GVfs       *vfs,
                     const char *parse_name,
                     gpointer    user_data)
{
    Q_UNUSED(vfs)
    Q_UNUSED(user_data)
    QString tmp = parse_name;
    if (tmp.contains("real-uri:")) {
        QString realUri = tmp.split("real-uri:").last();
        return g_file_new_for_uri(realUri.toUtf8().constData());
    }
    return explor_search_vfs_file_new_for_uri(parse_name);
}

static GFile *
test_vfs_lookup (GVfs       *vfs,
                 const char *uri,
                 gpointer    user_data)
{
    return test_vfs_parse_name(vfs, uri, user_data);
}

void SearchVFSRegister::registSearchVFS()
{
    if (is_registed)
        return;

    //init manager
    Peony::SearchVFSManager::getInstance();

    GVfs *vfs;
    const gchar * const *schemes;

    vfs = g_vfs_get_default ();
    schemes = g_vfs_get_supported_uri_schemes(vfs);
    const gchar * const *p;
    p = schemes;
    while (*p) {
        qDebug() << "registSearchVFS:" << *p;
        p++;
    }

#if GLIB_CHECK_VERSION(2, 50, 0)
    gboolean res;
    res = g_vfs_register_uri_scheme (vfs, "search",
                                     test_vfs_lookup, NULL, NULL,
                                     test_vfs_parse_name, NULL, NULL);
    Q_UNUSED(res)
#else
    //FIXME: how to implement search operation in old glib?
#endif
}

SearchVFSRegister::SearchVFSRegister()
{

}

void SearchVFSInternalPlugin::setEnable(bool enable)
{
    Q_UNUSED(enable)
}

void SearchVFSInternalPlugin::initVFS()
{
    SearchVFSRegister::registSearchVFS();
}

void *SearchVFSInternalPlugin::parseUriToVFSFile(const QString &uri)
{
    return explor_search_vfs_file_new_for_uri(uri.toUtf8().constData());
}
