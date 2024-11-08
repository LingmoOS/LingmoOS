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

#include "linux-pwd-helper.h"

#include <unistd.h>
#include <QtNetwork/QHostInfo>

LinuxPWDHelper::LinuxPWDHelper()
{

}

const QList<PWDItem> LinuxPWDHelper::getAllUserInfos()
{
    setpwent();
    QList<PWDItem> l;
    struct passwd *user;
    while((user = getpwent())!=nullptr) {
        l<<PWDItem(user);
    }
    endpwent();

    return l;
}

PWDItem::PWDItem(passwd *user)
{
    m_user_name = user->pw_name;
    m_full_name = user->pw_gecos;
    m_home_dir = user->pw_dir;
    m_shell_dir = user->pw_shell;
    m_uid = user->pw_uid;
    m_gid = user->pw_gid;
}

const PWDItem LinuxPWDHelper::getCurrentUser()
{
    uid_t uid = geteuid();
    struct passwd *pw = getpwuid(uid);
    return PWDItem(pw);
}

const QString LinuxPWDHelper::localHost()
{
    return QHostInfo::localHostName();
}
