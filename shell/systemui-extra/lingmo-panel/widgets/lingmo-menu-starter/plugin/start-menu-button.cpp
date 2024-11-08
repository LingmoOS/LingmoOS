
/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#include "start-menu-button.h"

#include <QDBusInterface>
#include <QDBusConnection>
#include <QDebug>
#include <QAction>
#include <QDBusReply>
#include <KWindowSystem>
#include <QGSettings>
#include <app-launcher.h>

StartMenuButton::StartMenuButton(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<QList<QAction*>>();
    loadUserAction();
    loadPowerAction();
}

void StartMenuButton::openStartMenu()
{
    QDBusInterface interface("org.lingmo.menu", "/org/lingmo/menu", "org.lingmo.menu", QDBusConnection::sessionBus());
    interface.asyncCall(QStringLiteral("WinKeyResponse"));
}

void StartMenuButton::showDesktop()
{
    KWindowSystem::setShowingDesktop(!KWindowSystem::showingDesktop());
}

void StartMenuButton::execSessionAction(const QString &action)
{
    QDBusInterface sessionManager("org.gnome.SessionManager", "/org/gnome/SessionManager",
                                  "org.gnome.SessionManager", QDBusConnection::sessionBus());
    sessionManager.asyncCall(action);
}

void StartMenuButton::loadUserAction()
{
    bool hasMultipleUsers {false}, canSwitch {false};

    // sessionManager
    QDBusInterface sessionManager("org.gnome.SessionManager", "/org/gnome/SessionManager",
                                  "org.gnome.SessionManager", QDBusConnection::sessionBus());

    QDBusReply<bool> reply = sessionManager.call(QStringLiteral("canLockscreen"));
    if (reply.isValid() && reply.value()) {
        m_userActions << new QAction(QIcon::fromTheme("system-lock-screen-symbolic"), tr("Lock Screen"), this);
        connect(m_userActions.last(), &QAction::triggered, this, [this] {
            LingmoUIQuick::AppLauncher::instance()->runCommand("lingmo-screensaver-command -l");
        });
    }

    reply = sessionManager.call(QStringLiteral("canSwitch"));
    canSwitch = (reply.isValid() && reply.value());

    if (canSwitch) {
        QDBusInterface interface("org.freedesktop.Accounts", "/org/freedesktop/Accounts",
                                 "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
        QDBusReply<QVariant> getReply = interface.call(QStringLiteral("Get"), QStringLiteral("org.freedesktop.Accounts"), QStringLiteral("HasMultipleUsers"));
        hasMultipleUsers = (getReply.isValid() && getReply.value().toBool());

        if (hasMultipleUsers) {
            m_userActions << new QAction(QIcon::fromTheme("user-available-symbolic"), tr("Switch User"), this);
            connect(m_userActions.last(), &QAction::triggered, this, [this] {
                execSessionAction(QStringLiteral("switchUser"));
            });
        }
    }

    reply = sessionManager.call(QStringLiteral("canLogout"));
    if (reply.isValid() && reply.value()) {
        m_userActions << new QAction(QIcon::fromTheme("system-log-out-symbolic"), tr("Log Out"), this);
        connect(m_userActions.last(), &QAction::triggered, this, [this] {
            execSessionAction(QStringLiteral("logout"));
        });
    }
}

void StartMenuButton::loadPowerAction()
{
    QDBusInterface sessionManager("org.gnome.SessionManager", "/org/gnome/SessionManager",
                                  "org.gnome.SessionManager", QDBusConnection::sessionBus());

    QDBusReply<bool> reply = sessionManager.call(QStringLiteral("canSuspend"));
    if (reply.isValid() && reply.value()) {
        m_powerActions << new QAction(QIcon::fromTheme("lingmo-sleep-symbolic"), tr("Suspend"), this);
        connect(m_powerActions.last(), &QAction::triggered, this, [this] {
            execSessionAction(QStringLiteral("suspend"));
        });
    }

    reply = sessionManager.call(QStringLiteral("canHibernate"));
    if (reply.isValid() && reply.value()) {
        m_powerActions << new QAction(QIcon::fromTheme("lingmo-hibernate-symbolic"), tr("Hibernate"), this);
        connect(m_powerActions.last(), &QAction::triggered, this, [this] {
            execSessionAction(QStringLiteral("hibernate"));
        });
    }

    reply = sessionManager.call(QStringLiteral("canReboot"));
    if (reply.isValid() && reply.value()) {
        m_powerActions << new QAction(QIcon::fromTheme("lingmo-system-restart-symbolic"), tr("Reboot"), this);
        connect(m_powerActions.last(), &QAction::triggered, this, [this] {
            execSessionAction(QStringLiteral("reboot"));
        });
    }

    const QByteArray id("org.lingmo.time-shutdown.settings");
    if (QGSettings::isSchemaInstalled(id)) {
        m_powerActions << new QAction(QIcon::fromTheme("lingmo-shutdown-timer-symbolic"), tr("Time Shutdown"), this);
        connect(m_powerActions.last(), &QAction::triggered, this, [this] {
            LingmoUIQuick::AppLauncher::instance()->runCommand("time-shutdown");
        });
    }

    reply = sessionManager.call(QStringLiteral("canPowerOff"));
    if (reply.isValid() && reply.value()) {
        m_powerActions << new QAction(QIcon::fromTheme("system-shutdown-symbolic"), tr("Power Off"), this);
        connect(m_powerActions.last(), &QAction::triggered, this, [this] {
            execSessionAction(QStringLiteral("powerOff"));
        });
    }
}
