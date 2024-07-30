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

#ifndef X11TABLETFINDER_H
#define X11TABLETFINDER_H

#include <QList>
#include <QString>

#include "tabletinformation.h"
#include "x11inputvisitor.h"

namespace Wacom
{

class X11Input;

// Forward Declarations
class DeviceInformation;
class X11TabletFinderPrivate;

/**
 * A tablet finder which uses the X11 window system to find connected tablets.
 */
class X11TabletFinder : public X11InputVisitor
{
public:
    X11TabletFinder();
    ~X11TabletFinder() override;

    /**
     * Returns the devices which were found from a previous run of scanDevices().
     *
     * @return A list of devices found.
     */
    const QList<TabletInformation> &getTablets() const;

    /**
     * Scans for available devices.
     *
     * @return True if devices were found, else false.
     */
    bool scanDevices();

    /**
     * @see X11InputVisitor::visit(X11InputDevice&)
     */
    bool visit(X11InputDevice &device) override;

private:
    /**
     * Adds the given device information to the internal tablet map.
     * If no tablet exists with the serial number of the device, a new one is created.
     */
    void addDeviceInformation(Wacom::DeviceInformation &deviceInformation);

    /**
     * Gather all information about the given x11 device and write the data
     * into the device information structure.
     *
     * @param device The device to gather information about.
     * @param deviceInformation The device information structure which will contain all data.
     */
    void gatherDeviceInformation(X11InputDevice &device, DeviceInformation &deviceInformation) const;

    /**
     * Gets the device node of the given device.
     *
     * @return The device node or an empty string if not set.
     */
    const QString getDeviceNode(Wacom::X11InputDevice &device) const;

    /**
     * Determines the device type base on the given toolTyple.
     *
     * @return A pointer to the device type if it could be determined, else NULL.
     */
    const DeviceType *getDeviceType(const QString &toolType) const;

    /**
     * Gets the product and vendor id if set. These are usually the company and
     * product id of the USB device. However the information is optional and might
     * not be set.
     *
     * @param device The device to get the information from.
     * @param vendorId A reference to a long value which will contain the vendor id.
     * @param productId A reference to a long value which will contain the product id.
     *
     * @return True if it least one value was set, else false.
     */
    bool getProductId(X11InputDevice &device, long &vendorId, long &productId) const;

    /**
     * Gets the tablet id of this device.
     *
     * @return The tablet id or 0 on error.
     */
    long getTabletSerial(X11InputDevice &device) const;

    /**
     * Gets the tool type name from the wacom tool type property.
     *
     * @return The tool type name on success, an empty string on error.
     */
    const QString getToolType(X11InputDevice &device) const;

private:
    Q_DECLARE_PRIVATE(X11TabletFinder)
    X11TabletFinderPrivate *const d_ptr;

}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
