/*
 * Peony-Qt
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "plasma-shell-manager.h"

#include <QApplication>

#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/registry.h>

#include <KWayland/Client/surface.h>

#include <QX11Info>

static PlasmaShellManager* global_instance = nullptr;

PlasmaShellManager *PlasmaShellManager::getInstance()
{
    if (!global_instance)
        global_instance = new PlasmaShellManager;
    return global_instance;
}

bool PlasmaShellManager::setRole(QWindow *window, KWayland::Client::PlasmaShellSurface::Role role)
{
    if (!supportPlasmaShell())
        return false;

    auto surface = KWayland::Client::Surface::fromWindow(window);
    if (!surface)
        return false;

    auto plasmaShellSurface = m_shell->createSurface(surface, window);
    if (!plasmaShellSurface)
        return false;

    plasmaShellSurface->setRole(role);
    return true;
}

bool PlasmaShellManager::setPos(QWindow *window, const QPoint &pos)
{
    if (!supportPlasmaShell())
        return false;

    auto surface = KWayland::Client::Surface::fromWindow(window);
    if (!surface)
        return false;

    auto plasmaShellSurface = m_shell->createSurface(surface, window);
    if (!plasmaShellSurface)
        return false;

    plasmaShellSurface->setPosition(pos);
    return true;
}

bool PlasmaShellManager::supportPlasmaShell()
{
    return m_shell;
}

KWayland::Client::PlasmaShellSurface *PlasmaShellManager::createSurface(QWindow *window)
{
    if (!supportPlasmaShell())
        return nullptr;

    auto surface = KWayland::Client::Surface::fromWindow(window);
    if (!surface)
        return nullptr;

    auto plasmaShellSurface = m_shell->createSurface(surface, window);
    return plasmaShellSurface;
}

PlasmaShellManager::PlasmaShellManager(QObject *parent) : QObject(parent)
{
    if (!QApplication::platformName().toLower().contains("wayland"))
        return;
    auto connection = KWayland::Client::ConnectionThread::fromApplication(qApp);
    auto registry = new KWayland::Client::Registry(this);
    registry->create(connection->display());

    connect(registry, &KWayland::Client::Registry::plasmaShellAnnounced, this, [=](){
        const auto interface = registry->interface(KWayland::Client::Registry::Interface::PlasmaShell);
        if (interface.name != 0) {
            m_shell = registry->createPlasmaShell(interface.name, interface.version, this);
        }
    });

    registry->setup();
    connection->roundtrip();
}
