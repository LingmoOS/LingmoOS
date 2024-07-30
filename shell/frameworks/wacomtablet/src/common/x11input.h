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

#ifndef X11INPUT_H
#define X11INPUT_H

#include "x11inputdevice.h"
#include "x11inputvisitor.h"

#include <QString>

namespace Wacom
{
/**
 * Basic XInput interface. It uses the visitor pattern to allow easy
 * access to all XInput devices.
 */
class X11Input
{
public:
    /*
     * Properties we are interested in.
     *
     * TODO: this should be moved into a X11InputProperty Enum.
     */
    static const QString PROPERTY_DEVICE_PRODUCT_ID;
    static const QString PROPERTY_DEVICE_NODE;
    static const QString PROPERTY_TRANSFORM_MATRIX;
    static const QString PROPERTY_WACOM_SERIAL_IDS;
    static const QString PROPERTY_WACOM_TABLET_AREA;
    static const QString PROPERTY_WACOM_TOOL_TYPE;

    /**
     * Find a device by Name.
     *
     * @param deviceName The XInput device name of the device to get.
     * @param device     The device which will be opened by this method if a device was found.
     *
     * @return True if device was found, else false.
     */
    static bool findDevice(const QString &deviceName, X11InputDevice &device);

    /**
     * Iterates over all X11 input devices and passes each device to the
     * visitor object. The visitor can then decide either to continue
     * iteration or abort it.
     */
    static void scanDevices(X11InputVisitor &visitor);

}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
