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

#include "wmregister.h"
#include <kwindowsystem.h>
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/registry.h>
#include <QGuiApplication>
#include "../waylandhelper.h"
#include "kdewaylandinterface.h"
#include "lingmowaylandinterface.h"
#include "xcbinterface.h"

using namespace kdk;
using namespace KWayland::Client;
kdk::WmRegister::WmRegister(QObject *parent)
    :QObject(parent)
{
    QString platform = QGuiApplication::platformName();
    if(platform.startsWith(QLatin1String("wayland"),Qt::CaseInsensitive))
    {
        #ifdef USE_LINGMO_PROTOCOL
            m_winInterface = new LingmoUIWaylandInterface(this);
            LingmoUIWaylandInterface* interface = dynamic_cast<LingmoUIWaylandInterface*>(m_winInterface);
            if(!interface->lingmoProtocolReady())
            {
                delete m_winInterface;
                m_winInterface = new KdeWaylandInterface(this);
            }
        #else
            m_winInterface = new KdeWaylandInterface(this);
        #endif
    }
    else
        m_winInterface = new XcbInterface(this);
}

WmRegister::~WmRegister()
{
}

AbstractInterface *WmRegister::winInterface()
{
    return m_winInterface;
}
