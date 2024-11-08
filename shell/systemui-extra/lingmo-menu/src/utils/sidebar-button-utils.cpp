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
#include "sidebar-button-utils.h"
#include "app-manager.h"

#include <QProcess>
#include <QDBusReply>
#include <QQmlEngine>

#define LINGMO_ACCOUNT_INFORMATION_NAME         "org.freedesktop.Accounts"
#define LINGMO_ACCOUNT_INFORMATION_PATH         "/org/freedesktop/Accounts"
#define LINGMO_ACCOUNT_INFORMATION_INTERFACE    "org.freedesktop.Accounts"
#define DEFAULT_USER_ICON_FILE        ":/res/icon/default-community-image.png"

using namespace LingmoUIMenu;

SidebarButtonUtils::SidebarButtonUtils(QObject *parent) : QObject(parent)
{
    registeredAccountsDbus();
    initUserInfo();
}

void SidebarButtonUtils::defineModule(const char *uri, int versionMajor, int versionMinor)
{
    qmlRegisterType<SidebarButtonUtils>("org.lingmo.menu.utils", versionMajor, versionMinor, "SidebarButtonUtils");
}

QString SidebarButtonUtils::getUsername()
{
    return m_currentUserInfo.username;
}

QString SidebarButtonUtils::getRealName()
{
    return m_currentUserInfo.realName;
}

QString SidebarButtonUtils::getIconFile()
{
    if (m_currentUserInfo.iconFile.isEmpty() || m_currentUserInfo.iconFile.endsWith("/.face")) {
        return {DEFAULT_USER_ICON_FILE};
    }

    return m_currentUserInfo.iconFile;
}

void SidebarButtonUtils::openUserCenter()
{
    QStringList args;
    args.append("-m");
    args.append("Userinfo");
    LingmoUIMenu::AppManager::instance()->launchAppWithArguments("/usr/share/applications/lingmo-control-center.desktop", args, "lingmo-control-center");
}

void SidebarButtonUtils::openPeonyComputer()
{
    LingmoUIMenu::AppManager::instance()->launchAppWithArguments("/usr/share/applications/explorer-computer.desktop", QStringList(), "/usr/bin/explorer computer:///");
}

void SidebarButtonUtils::openControlCenter()
{
    QStringList args;
    args.append("-m");
    args.append("Boot");
    LingmoUIMenu::AppManager::instance()->launchAppWithArguments("/usr/share/applications/lingmo-control-center.desktop", args, "lingmo-control-center");
}

void SidebarButtonUtils::registeredAccountsDbus()
{
    m_systemUserIFace = new QDBusInterface(LINGMO_ACCOUNT_INFORMATION_NAME,
                                           LINGMO_ACCOUNT_INFORMATION_PATH,
                                           LINGMO_ACCOUNT_INFORMATION_INTERFACE,
                                           QDBusConnection::systemBus(), this);
}

void SidebarButtonUtils::initUserInfo()
{
    QStringList objectPaths = getUserObjectPath();
    UserInformation user;

    for (const QString& objectPath : objectPaths){
        user = GetUserInformation(objectPath);
        if(user.current && user.isLogged) {
            m_currentUserInfo = user;
            m_currentUserIFace = new QDBusInterface(LINGMO_ACCOUNT_INFORMATION_NAME,
                                                    objectPath,
                                                    "org.freedesktop.DBus.Properties",
                                                    QDBusConnection::systemBus(), this);

            m_currentUserIFace->connection().connect(LINGMO_ACCOUNT_INFORMATION_NAME, objectPath,
                                                     "org.freedesktop.DBus.Properties", "PropertiesChanged",
                                                     this, SLOT(currentAccountUpdateSlot(QString, QMap<QString, QVariant>, QStringList)));

            break;
        }
    }
}

QStringList SidebarButtonUtils::getUserObjectPath()
{
    QStringList users;

    QDBusReply<QList<QDBusObjectPath> > reply = m_systemUserIFace->call("ListCachedUsers");
    if (reply.isValid()) {
        for (const QDBusObjectPath& op : reply.value()) {
            users << op.path();
        }
    }

    return users;
}

UserInformation SidebarButtonUtils::GetUserInformation(const QString &objectPath)
{
    UserInformation user;

    user.current = false;
    user.isLogged = false;
    user.autologin = false;

    QDBusInterface userInterface("org.freedesktop.Accounts",
                                 objectPath,
                                 "org.freedesktop.DBus.Properties",
                                 QDBusConnection::systemBus());

    QDBusReply<QMap<QString, QVariant> > reply = userInterface.call("GetAll", "org.freedesktop.Accounts.User");
    if (reply.isValid()) {
        QMap<QString, QVariant> propertyMap;
        propertyMap = reply.value();
        user.username = propertyMap.find("UserName").value().toString();
        user.realName = propertyMap.find("RealName").value().toString();
        user.accountType = propertyMap.find("AccountType").value().toInt();
        user.iconFile = propertyMap.find("IconFile").value().toString();
        user.passwdType = propertyMap.find("PasswordMode").value().toInt();
        user.uid = propertyMap.find("Uid").value().toInt();
        user.autologin = false;
        user.objPath = objectPath;

        if (user.username == QString(qgetenv("USER"))) {
            user.current = true;
            user.isLogged = true;
        }
    } else {
        qDebug() << "GetUserInformation: reply failed";
    }

    return user;
}

void SidebarButtonUtils::currentAccountUpdateSlot(const QString &property, const QMap<QString, QVariant> &propertyMap, const QStringList &propertyList)
{
    Q_UNUSED(property)
    Q_UNUSED(propertyList)
    QStringList keys = propertyMap.keys();

    if (keys.contains("IconFile")) {
        m_currentUserInfo.iconFile = propertyMap.value("IconFile").toString();
    }

    if (keys.contains("UserName")) {
        m_currentUserInfo.username = propertyMap.value("UserName").toString();
    }

    if (keys.contains("RealName")) {
        m_currentUserInfo.realName = propertyMap.value("RealName").toString();
    }

    if (keys.contains("AccountType")) {
        m_currentUserInfo.accountType = propertyMap.value("AccountType").toInt();
    }

    Q_EMIT userInfoChanged();
}
