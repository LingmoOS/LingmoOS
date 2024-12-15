// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "utils.h"

#include <unistd.h>
#include <pwd.h>

namespace Utils {

QString workingUser()
{
    auto pwd = getpwuid(getuid());
    return pwd ? pwd->pw_name : "";
}

QString mountPointOf(GMount *mount)
{
    GFile_autoptr root = g_mount_get_root(mount);
    if (root) {
        g_autofree char *cMpt = g_file_get_path(root);
        return QString(cMpt);
    }
    return "";
}

/*!
 * @~english \brief mountInfoOf
 * @~english \param mount
 * @~english \return the pair of uri and mountpoint.
 * @~english      uri is like 'smb://1.2.3.4',
 * @~english      while mountpoint is like '/run/user/1000/gvfs/xxx...'
 */
QPair<QString, QString> mountInfoOf(GMount *mount)
{
    GFile_autoptr root = g_mount_get_root(mount);
    if (root) {
        g_autofree char *cMpt = g_file_get_path(root);
        g_autofree char *cUri = g_file_get_uri(root);
        return { { cUri }, { cMpt } };
    }
    return {};
}

}   // namespace Utils
