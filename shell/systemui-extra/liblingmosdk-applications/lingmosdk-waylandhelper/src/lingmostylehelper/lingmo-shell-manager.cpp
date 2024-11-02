/*
 * liblingmosdk-waylandhelper's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include <KWayland/Client/surface.h>
#include <KWayland/Client/connection_thread.h>
#include <QApplication>
#include <QDebug>
#include "lingmo-shell-manager.h"
#include "../windowmanager/lingmo-shell-client-protocol.h"
#include "../windowmanager/lingmowaylandinterface.h"

static LingmoUIShellManager *global_instance = nullptr;
static LingmoUIWaylandInterface *g_lingmowaylandInterface = nullptr;

LingmoUIShellManager::LingmoUIShellManager()
{
    if(!g_lingmowaylandInterface)
        g_lingmowaylandInterface = new LingmoUIWaylandInterface;
}

LingmoUIShellManager *LingmoUIShellManager::getInstance()
{
    if (!global_instance)
        global_instance = new LingmoUIShellManager;
    return global_instance;
}

bool LingmoUIShellManager::removeHeaderBar(QWindow *windowHandle)
{
    if(!g_lingmowaylandInterface->lingmoProtocolReady())
        return false;
    return g_lingmowaylandInterface->removeHeaderBar(windowHandle);
}

bool LingmoUIShellManager::lingmoshellReady()
{
    return g_lingmowaylandInterface->lingmoProtocolReady();
}
