/*
 * This file is part of the KDE wacomtablet project. For copyright
 * information and license terms see the AUTHORS and COPYING files
 * in the top-level directory of this distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dbustabletinterface.h"
#include "stringutils.h"

#include <QMetaType>
#include <QMutex>
#include <QString>

using namespace Wacom;

// instantiate static class members
DBusTabletInterface *DBusTabletInterface::m_instance = nullptr;

DBusTabletInterface::DBusTabletInterface()
    : OrgKdeWacomInterface(QLatin1String("org.kde.Wacom"), QLatin1String("/Tablet"), QDBusConnection::sessionBus())
{
    DBusTabletInterface::registerMetaTypes();
}

DBusTabletInterface &DBusTabletInterface::instance()
{
    if (!m_instance) {
        static QMutex mutex;
        mutex.lock();

        if (!m_instance) {
            resetInterface();
        }

        mutex.unlock();
    }

    return *m_instance;
}

void DBusTabletInterface::resetInterface()
{
    static QMutex mutex;
    mutex.lock();

    if (m_instance) {
        delete m_instance;
        m_instance = nullptr;
    }

    m_instance = new DBusTabletInterface();

    mutex.unlock();
}

void DBusTabletInterface::registerMetaTypes()
{
    // nothing to register for now
    // we keep this method so we have a central location to manage meta-types from

    // qDBusRegisterMetaType<Wacom::TabletInformation>();
}
