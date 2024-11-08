/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#include "menu-dbus-service.h"
#include "menuadaptor.h"

#include <KWindowSystem>

#include <QDir>
#include <QDebug>
#include <QDBusConnection>
#include <QDBusServiceWatcher>
#include <QDBusConnectionInterface>

#define MENU_CORE_DBUS_SERVICE          "org.lingmo.menu"
#define MENU_CORE_DBUS_PATH             "/org/lingmo/menu"
#define MENU_CORE_DBUS_INTERFACE        "org.lingmo.menu"

using namespace LingmoUIMenu;

QString MenuDbusService::displayFromPid(uint pid)
{
    QFile environFile(QStringLiteral("/proc/%1/environ").arg(QString::number(pid)));
    if (environFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QByteArray DISPLAY = KWindowSystem::isPlatformWayland() ? QByteArrayLiteral("WAYLAND_DISPLAY")
                                                                      : QByteArrayLiteral("DISPLAY");
        const auto lines = environFile.readAll().split('\0');
        for (const QByteArray &line : lines) {
            const int equalsIdx = line.indexOf('=');
            if (equalsIdx <= 0) {
                continue;
            }
            const QByteArray key = line.left(equalsIdx);
            if (key == DISPLAY) {
                const QByteArray value = line.mid(equalsIdx + 1);
                return value;
            }
        }
    }

    return {};
}

MenuDbusService::MenuDbusService(const QString &display, QObject *parent) : QObject(parent), m_display(display)
{
    bool isServiceRegistered = QDBusConnection::sessionBus().interface()->isServiceRegistered(MENU_CORE_DBUS_SERVICE);
    qDebug() << "menu service is registered:" << isServiceRegistered << ", display:" << display;

    if (isServiceRegistered) {
        initWatcher();

    } else {
        bool success = registerService();
        if (!success) {
            initWatcher();
        }
        qDebug() << "menu service register:" << success;
    }
}

void MenuDbusService::WinKeyResponse()
{
    uint callerPid = QDBusConnection::sessionBus().interface()->servicePid(message().service()).value();
    QString display = MenuDbusService::displayFromPid(callerPid);

    active(display);
}

QString MenuDbusService::GetSecurityConfigPath()
{
    QString path = QDir::homePath() + "/.config/lingmo-menu-security-config.json";
    return path;
}

void MenuDbusService::ReloadSecurityConfig()
{
    Q_EMIT reloadConfigSignal();
}

void MenuDbusService::active(const QString &display)
{
    if (display.isEmpty() || (display == m_display)) {
        Q_EMIT menuActive();
        return;
    }

    if (m_menuAdaptor) {
        Q_EMIT m_menuAdaptor->activeRequest(display);
    }
}

void MenuDbusService::activeMenu(QString display)
{
    if (display == m_display) {
        Q_EMIT menuActive();
    }
}

bool MenuDbusService::registerService()
{
    m_menuAdaptor = new MenuAdaptor(this);
    QDBusReply<QDBusConnectionInterface::RegisterServiceReply> reply =
        QDBusConnection::sessionBus().interface()->registerService(MENU_CORE_DBUS_SERVICE,
                                                                   QDBusConnectionInterface::ReplaceExistingService,
                                                                   QDBusConnectionInterface::DontAllowReplacement);

    if (reply.value() == QDBusConnectionInterface::ServiceNotRegistered) {
        return false;
    }

    bool res = QDBusConnection::sessionBus().registerObject(MENU_CORE_DBUS_PATH, this);
    if (!res) {
        QDBusConnection::sessionBus().interface()->unregisterService(MENU_CORE_DBUS_SERVICE);
    }

    return res;
}

void MenuDbusService::onServiceOwnerChanged(const QString &service, const QString &oldOwner, const QString &newOwner)
{
    qDebug() << "serviceOwnerChanged:" << service << oldOwner << newOwner;
    if (newOwner.isEmpty()) {
        bool success = registerService();
        if (success) {
            disConnectActiveRequest();
            delete m_watcher;
            m_watcher = nullptr;
        }
        qDebug() << "try to register service:" << success;
        return;
    }

    uint newOwnerPid = QDBusConnection::sessionBus().interface()->servicePid(newOwner);
    qDebug() << "newOwnerPid:" << newOwnerPid << ", myPid:" << QCoreApplication::applicationPid() << ", display:" << m_display;
//        if (newOwnerPid == QCoreApplication::applicationPid()) {
//            qDebug() << "Becoming a new service";
//        }
}

void MenuDbusService::connectToActiveRequest()
{
    QDBusConnection::sessionBus().connect(MENU_CORE_DBUS_SERVICE,
                                          MENU_CORE_DBUS_PATH,
                                          MENU_CORE_DBUS_INTERFACE,
                                          "activeRequest",
                                          this,
                                          SLOT(activeMenu(QString)));
}

void MenuDbusService::disConnectActiveRequest()
{
    QDBusConnection::sessionBus().disconnect(MENU_CORE_DBUS_SERVICE,
                                             MENU_CORE_DBUS_PATH,
                                             MENU_CORE_DBUS_INTERFACE,
                                             "activeRequest",
                                             this,
                                             SLOT(activeMenu(QString)));
}

void MenuDbusService::initWatcher()
{
    m_watcher = new QDBusServiceWatcher(MENU_CORE_DBUS_SERVICE,QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForOwnerChange);
    connect(m_watcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &MenuDbusService::onServiceOwnerChanged);
    connectToActiveRequest();
}
