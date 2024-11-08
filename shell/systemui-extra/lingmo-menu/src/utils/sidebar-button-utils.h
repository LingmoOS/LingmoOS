/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 */
#ifndef SIDEBARBUTTONUTILS_H
#define SIDEBARBUTTONUTILS_H

#include <QObject>
#include <QDBusInterface>

namespace LingmoUIMenu {

typedef struct UserInfo
{
    bool current;
    bool isLogged;
    bool autologin;
    bool noPwdLogin;

    int accountType;
    int passwdType;
    qint64 uid;

    QString objPath;
    QString username;
    QString realName;
    QString iconFile;
    QString passwd;
} UserInformation;

class SidebarButtonUtils : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString username READ getUsername NOTIFY userInfoChanged)
    Q_PROPERTY(QString realName READ getRealName NOTIFY userInfoChanged)
    Q_PROPERTY(QString iconFile READ getIconFile NOTIFY userInfoChanged)
public:
    explicit SidebarButtonUtils(QObject *parent = nullptr);
    static void defineModule(const char *uri, int versionMajor, int versionMinor);

    Q_INVOKABLE QString getUsername();
    Q_INVOKABLE QString getRealName();
    Q_INVOKABLE QString getIconFile();
    Q_INVOKABLE void openUserCenter();
    Q_INVOKABLE void openPeonyComputer();
    Q_INVOKABLE void openControlCenter();

private:
    void registeredAccountsDbus();
    void initUserInfo();
    QStringList getUserObjectPath();
    UserInformation GetUserInformation(const QString &objectPath);

private Q_SLOTS:
    void currentAccountUpdateSlot(const QString &property, const QMap<QString, QVariant> &propertyMap,
                                  const QStringList &propertyList);

Q_SIGNALS:
    void userInfoChanged();

private:
    QDBusInterface *m_systemUserIFace = nullptr;
    QDBusInterface *m_currentUserIFace = nullptr;

    UserInformation m_currentUserInfo;
    QString m_administrator;
    QString m_standardUser;
};

} // LingmoUIMenu

#endif // SIDEBARBUTTONUTILS_H
