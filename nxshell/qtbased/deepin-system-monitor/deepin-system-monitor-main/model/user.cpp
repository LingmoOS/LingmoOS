/*
 * Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "user.h"

User::User(QObject *parent)
    : QObject(parent)
    , m_isCurrentUser(false)
    , m_online(false)
    , m_userType(0)
    , m_name("")
    , m_fullname("")
    , m_iconFile("")
{

}

User::~User()
{

}

void User::setName(const QString &name)
{
    if (name != m_name) {
        m_name = name;
    }
}

void User::setFullname(const QString &fullname)
{
    if (fullname != m_fullname) {
        m_fullname = fullname;
    }
}

const QString User::displayName() const
{
    return m_fullname.isEmpty() ? m_name : m_fullname;
}

void User::setIsCurrentUser(bool isCurrentUser)
{
    if (isCurrentUser == m_isCurrentUser)
        return;

    m_isCurrentUser = isCurrentUser;
}

void User::setUserType(const int userType)
{
    if (m_userType == userType) {
        return;
    }
    m_userType = userType;
}


void User::setIconFile(const QString &iconFile)
{
    if (m_iconFile == iconFile) {
        return;
    }
    m_iconFile = iconFile;
}

void User::setUserUid(const QString &uid)
{
    if (m_uid == uid) {
        return;
    }
    m_uid = uid;
}

void User::setOnline(bool online)
{
    if (m_online != online) {
        m_online = online;
    }
}
