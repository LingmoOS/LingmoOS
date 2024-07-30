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

#ifndef DBUSTABLETINTERFACE_H
#define DBUSTABLETINTERFACE_H

#include "devicetype.h"
#include "property.h"
#include "tabletinfo.h"
#include "wacominterface.h"

// D-Bus interface metatypes
// Q_DECLARE_METATYPE(Wacom::TabletInformation)

namespace Wacom
{

/**
 * A singleton class to access the D-Bus tablet interface.
 */
class DBusTabletInterface : public ::OrgKdeWacomInterface
{
public:
    /**
     * Returns a reference to the only instance of this class.
     */
    static DBusTabletInterface &instance();

    /**
     * Resets the D-Bus interface connection.
     * This will invalidate all pointers to the current instance!
     * It will also destroy all signal-slot connections.
     */
    static void resetInterface();

    /**
     * Registers d-bus metatypes.
     *
     * This method should not be called directly. It is used by this
     * interface and the d-bus service class to register the meta
     * types. This is just to have one single location where all
     * metatypes are managed.
     */
    static void registerMetaTypes();

protected:
    /**
     * Protected default constructor.
     * Use \a instance to get an instance of this class.
     */
    DBusTabletInterface();

private:
    /**
     * Singleton, no copying allowed
     */
    DBusTabletInterface(const DBusTabletInterface &) = delete;
    DBusTabletInterface &operator=(const DBusTabletInterface &) = delete;

    static DBusTabletInterface *m_instance;

}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
