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

#include "waylandhelper.h"
#include <KWindowSystem/kwindowsystem.h>
#include <QDebug>

using namespace kdk;

static const char* server_flag = "XDG_SESSION_TYPE";
static const char* client_flag = "QT_QPA_PLATFORM";

static WaylandHelper* g_instance = nullptr;
bool WaylandHelper::isWaylandServer()
{
    if (qgetenv(server_flag) == QString("wayland"))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool WaylandHelper::isWaylandClient()
{
    if (qgetenv(client_flag) == QString("wayland"))
    {
        return true;
    }
    else
    {
        return false;
    }
}

WaylandHelper *WaylandHelper::self()
{
    if (!g_instance)
        g_instance = new WaylandHelper();
    return g_instance;
}

WaylandHelper::WaylandHelper(QObject *parent)
    :QObject(parent)
{
}

