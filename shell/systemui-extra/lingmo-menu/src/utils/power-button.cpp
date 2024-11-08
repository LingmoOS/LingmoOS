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

#include <QProcess>
#include <QQmlEngine>
#include <QMenu>
#include <QDBusReply>

#include "power-button.h"
#include "app-manager.h"

#define LINGMO_POWER_NORMAL_PATH_MAJOR "qrc:///res/icon/pad_mainpower.svg"

using namespace LingmoUIMenu;

PowerButton::PowerButton(QObject *parent) : QObject(nullptr)
{

}

void PowerButton::defineModule(const char *uri, int versionMajor, int versionMinor)
{
    qmlRegisterType<PowerButton>("org.lingmo.menu.utils", versionMajor, versionMinor, "PowerButton");
}

PowerButton::~PowerButton()
= default;

QString PowerButton::getIcon()
{
    return "exit-symbolic";
}

QString PowerButton::getToolTip()
{
    return {tr("power")};
}

void PowerButton::clicked(bool leftButtonClicked, int mouseX, int mouseY, bool isFullScreen)
{
    if (leftButtonClicked) {
        QProcess::startDetached("lingmo-session-tools", {});
    } else {
        openMenu(mouseX, mouseY, isFullScreen);
    }
}

void PowerButton::openMenu(int menuX, int menuY, bool isFullScreen)
{
    if (m_contextMenu) {
        m_contextMenu.data()->close();
        return;
    }

    auto powerMenu = new QMenu;
    QDBusReply<bool> reply;
    QDBusInterface qDBusInterface("org.gnome.SessionManager", "/org/gnome/SessionManager",
                                  "org.gnome.SessionManager", QDBusConnection::sessionBus());

    if (canSwitch() && hasMultipleUsers()) {
        QAction *action = new QAction(tr("Switch user"), powerMenu);
        connect(action, &QAction::triggered, powerMenu, [] {
            AppManager::instance()->launchBinaryApp("lingmo-session-tools", "--switchuser");
        });
        powerMenu->addAction(action);
    }

    reply = qDBusInterface.call("canHibernate");
    if (reply.isValid() && reply.value()) {
        QAction *action = new QAction(tr("Hibernate"), powerMenu);
        action->setToolTip(tr("<p>Turn off the computer, but the applications "
                              "will remain open. When you turn on the computer again, "
                              "you can return to the state you were in before</p>"));
        connect(action, &QAction::triggered, powerMenu, [] {
            AppManager::instance()->launchBinaryApp("lingmo-session-tools", "--hibernate");
        });
        powerMenu->addAction(action);
    }

    reply = qDBusInterface.call("canSuspend");
    if (reply.isValid() && reply.value()) {
        QAction *action = new QAction(tr("Suspend"), powerMenu);
        action->setToolTip(tr("<p>The computer remains on but consumes less power, "
                              "and the applications will remain open. You can quickly wake "
                              "up the computer and return to the state you left</p>"));
        connect(action, &QAction::triggered, powerMenu, [] {
            AppManager::instance()->launchBinaryApp("lingmo-session-tools", "--suspend");
        });
        powerMenu->addAction(action);
    }

    QAction *lockAction = new QAction(tr("Lock Screen"), powerMenu);
    connect(lockAction, &QAction::triggered, powerMenu, [] {
        AppManager::instance()->launchBinaryApp("lingmo-screensaver-command", "-l");
    });
    powerMenu->addAction(lockAction);

    reply = qDBusInterface.call("canLogout");
    if (reply.isValid() && reply.value()) {
        QAction *action = new QAction(tr("Log Out"), powerMenu);
        action->setToolTip(tr("<p>The current user logs out of the system, ending "
                              "their session and returning to the login screen</p>"));
        connect(action, &QAction::triggered, powerMenu, [] {
            AppManager::instance()->launchBinaryApp("lingmo-session-tools", "--logout");
        });
        powerMenu->addAction(action);
    }

    reply = qDBusInterface.call("canReboot");
    if (reply.isValid() && reply.value()) {
        QAction *action = new QAction(tr("Reboot"), powerMenu);
        action->setToolTip(tr("<p>Close all applications, turn off the computer, and then turn it back on</p>"));
        connect(action, &QAction::triggered, powerMenu, [] {
            AppManager::instance()->launchBinaryApp("lingmo-session-tools", "--reboot");
        });
        powerMenu->addAction(action);
    }

    reply = qDBusInterface.call("canPowerOff");
    if (reply.isValid() && reply.value()) {
        QAction *action = new QAction(tr("Power Off"), powerMenu);
        action->setToolTip(tr("<p>Close all applications, and then turn off the computer</p>"));
        connect(action, &QAction::triggered, powerMenu, [] {
            AppManager::instance()->launchBinaryApp("lingmo-session-tools", "--shutdown");
        });
        powerMenu->addAction(action);
    }

    m_contextMenu = powerMenu;
    powerMenu->setToolTipsVisible(true);
    powerMenu->setAttribute(Qt::WA_DeleteOnClose);

    if (isFullScreen) {
        m_contextMenu.data()->popup(QPoint(menuX - powerMenu->sizeHint().width(), menuY - powerMenu->sizeHint().height()));
    } else {
        m_contextMenu.data()->popup(QPoint(menuX, menuY - powerMenu->sizeHint().height()));
    }
}

bool PowerButton::hasMultipleUsers()
{
    QDBusInterface interface("org.freedesktop.Accounts", "/org/freedesktop/Accounts",
                             "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

    if (!interface.isValid()) {
        return false;
    }

    QDBusReply<QVariant> reply = interface.call("Get", "org.freedesktop.Accounts", "HasMultipleUsers");
    return reply.value().toBool();
}

bool PowerButton::canSwitch()
{
    QDBusInterface interface("org.freedesktop.DisplayManager", "/org/freedesktop/DisplayManager/Seat0",
                             "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

    if (!interface.isValid()) {
        return false;
    }

    QDBusReply<QVariant> reply = interface.call("Get", "org.freedesktop.DisplayManager.Seat", "CanSwitch");
    return reply.value().toBool();
}
