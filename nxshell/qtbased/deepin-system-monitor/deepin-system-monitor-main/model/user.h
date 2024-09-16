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

#ifndef USER_H
#define USER_H

#include <QObject>
#include <QString>


class User : public QObject
{
    Q_OBJECT
public:

    enum UserType {
        StandardUser = 0,
        Administrator,
        Customized
    };

    explicit User(QObject *parent = nullptr);

    ~User();

    const QString name() const { return m_name; }
    void setName(const QString &name);

    const QString fullname() const { return m_fullname; }
    void setFullname(const QString &fullname);

    const QString iconFile() const { return m_iconFile; }
    void setIconFile(const QString &iconFile);

    inline bool isCurrentUser() const { return m_isCurrentUser; }
    void setIsCurrentUser(bool isCurrentUser);

    inline int userType() const { return m_userType; }
    void setUserType(const int userType);

    inline QString userUid() const { return m_uid; }
    void setUserUid(const QString &uid);
    const QString displayName() const;

    inline bool online() const { return m_online; }
    void setOnline(bool online);

private:
    bool m_isCurrentUser;
    bool m_online;
    int m_userType;
    QString m_uid;

    QString m_name;
    QString m_fullname;
    QString m_iconFile;

};



#endif // USER_H
