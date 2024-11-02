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
 * Authors: Ling Yang <yangling@kylinos.cn>
 *
 */

#include "label-vfs-register.h"
#include "label-vfs-file.h"
#include <gio/gio.h>
#include <QDebug>

bool Label_is_registed = false;

static GFile* test_vfs_parse_name (GVfs* vfs, const char* parse_name, gpointer user_data)
{
    Q_UNUSED(vfs)
    Q_UNUSED(user_data)
    return vfs_label_file_new_for_uri(parse_name);
}

static GFile* test_vfs_lookup (GVfs* vfs, const char *uri, gpointer user_data)
{
    return test_vfs_parse_name(vfs, uri, user_data);
}

void Peony::LabelVFSInternalPlugin::setEnable(bool enable)
{
    Q_UNUSED(enable)
}

void Peony::LabelVFSInternalPlugin::initVFS()
{
    LabelVFSRegister::registLabelVFS();
}

void *Peony::LabelVFSInternalPlugin::parseUriToVFSFile(const QString &uri)
{
    return vfs_label_file_new_for_uri(uri.toUtf8());
}

void Peony::LabelVFSRegister::registLabelVFS()
{
    if (Label_is_registed) {
        return;
    }

    GVfs* vfs = nullptr;
    const gchar * const *schemes;

    vfs = g_vfs_get_default ();
    schemes = g_vfs_get_supported_uri_schemes (vfs);

    const gchar* const* p = schemes;
    while (*p) {
        qDebug() << *p;
        p++;
    }

#if GLIB_CHECK_VERSION(2, 50, 0)
    gboolean res = false;
    res = g_vfs_register_uri_scheme (vfs, "label", test_vfs_lookup, NULL, NULL, test_vfs_parse_name, NULL, NULL);
    Q_UNUSED(res)
#else
#endif
}

Peony::LabelVFSRegister::LabelVFSRegister()
{

}
