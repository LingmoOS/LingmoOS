/*
 * Copyright (C) 2025 Lingmo OS Team.
 *
 * Author:     Lingmo OS Team <team@lingmo.org>
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

#include "dockdbusinterface.h"
#include <QDBusServiceWatcher>
#include <QRect>

static DockDBusInterface *DOCKDBUS_SELF = nullptr;

DockDBusInterface *DockDBusInterface::self()
{
    if (!DOCKDBUS_SELF)
        DOCKDBUS_SELF = new DockDBusInterface;

    return DOCKDBUS_SELF;
}

DockDBusInterface::DockDBusInterface(QObject *parent)
    : QObject(parent)
    , m_dockInterface("com.lingmo.Dock",
                    "/Dock",
                    "com.lingmo.Dock", QDBusConnection::sessionBus())
    , m_leftMargin(0)
    , m_rightMargin(0)
    , m_bottomMargin(0)
{
    if (m_dockInterface.isValid()) {
        updateMargins();
        connect(&m_dockInterface, SIGNAL(primaryGeometryChanged()), this, SLOT(updateMargins()));
        connect(&m_dockInterface, SIGNAL(directionChanged()), this, SLOT(updateMargins()));
        connect(&m_dockInterface, SIGNAL(visibilityChanged()), this, SLOT(updateMargins()));
    } else {
        QDBusServiceWatcher *watcher = new QDBusServiceWatcher("com.lingmo.Dock",
                                                               QDBusConnection::sessionBus(),
                                                               QDBusServiceWatcher::WatchForUnregistration,
                                                               this);
        connect(watcher, &QDBusServiceWatcher::serviceUnregistered, this, [=] {
            updateMargins();
            connect(&m_dockInterface, SIGNAL(primaryGeometryChanged()), this, SLOT(updateMargins()));
            connect(&m_dockInterface, SIGNAL(directionChanged()), this, SLOT(updateMargins()));
            connect(&m_dockInterface, SIGNAL(visibilityChanged()), this, SLOT(updateMargins()));
        });
    }
}

int DockDBusInterface::leftMargin() const
{
    return m_leftMargin;
}

int DockDBusInterface::rightMargin() const
{
    return m_rightMargin;
}

int DockDBusInterface::bottomMargin() const
{
    return m_bottomMargin;
}

void DockDBusInterface::updateMargins()
{
    QRect dockGeometry = m_dockInterface.property("primaryGeometry").toRect();
    int dockDirection = m_dockInterface.property("direction").toInt();
    int visibility = m_dockInterface.property("visibility").toInt();

    m_leftMargin = 0;
    m_rightMargin = 0;
    m_bottomMargin = 0;

    // AlwaysHide
    if (visibility == 1) {
        emit marginsChanged();
        return;
    }

    if (dockDirection == 0) {
        m_leftMargin = dockGeometry.width();
    } else if (dockDirection == 1) {
        m_bottomMargin = dockGeometry.height();
    } else if (dockDirection == 2) {
        m_rightMargin = dockGeometry.width();
    }

    emit marginsChanged();
}
