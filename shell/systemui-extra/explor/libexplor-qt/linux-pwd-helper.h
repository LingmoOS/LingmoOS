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

#ifndef LINUXPWDHELPER_H
#define LINUXPWDHELPER_H

#include <QObject>

#include <pwd.h>
#include <sys/types.h>

#include "explor-core_global.h"

/*!
 * \brief The PWDItem class
 * is the abstraction of passwd struct in linux.
 */
class PEONYCORESHARED_EXPORT PWDItem
{
public:
    explicit PWDItem(passwd *user);
    ~PWDItem() {}

    const QString userName() {
        return m_user_name;
    }
    int userId() {
        return m_uid;
    }
    int groupId() {
        return m_gid;
    }

    const QString fullName() const {
        return m_full_name;
    }
    const QString homeDir() const {
        return m_home_dir;
    }
    const QString shellDir() const {
        return m_shell_dir;
    }

private:
    QString m_user_name;
    QString m_full_name;
    QString m_home_dir;
    QString m_shell_dir;

    int m_uid = -1;
    int m_gid = -1;
};

class PEONYCORESHARED_EXPORT LinuxPWDHelper
{
public:
    static const QList<PWDItem> getAllUserInfos();
    static const PWDItem getCurrentUser();
    static const QString localHost();

private:
    LinuxPWDHelper();
};

#endif // LINUXPWDHELPER_H
