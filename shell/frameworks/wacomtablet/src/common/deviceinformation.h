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

#ifndef DEVICEINFORMATION_H
#define DEVICEINFORMATION_H

#include "devicetype.h"

#include <QString>

namespace Wacom
{

class DeviceInformationPrivate;

/**
 * Device information structure which stores all information about a tablet
 * component (stylus, eraser, pad, ...).
 */
class DeviceInformation
{
public:
    DeviceInformation(const DeviceType &deviceType, const QString &deviceName);
    DeviceInformation(const DeviceInformation &that);
    virtual ~DeviceInformation();

    DeviceInformation &operator=(const DeviceInformation &that);

    bool operator!=(const DeviceInformation &that) const;

    bool operator==(const DeviceInformation &that) const;

    long getDeviceId() const;

    const QString &getDeviceNode() const;

    const QString &getName() const;

    long getProductId() const;

    long getTabletSerial() const;

    const DeviceType &getType() const;

    long getVendorId() const;

    /*
     * THESE SETTERS MIGHT BE REMOVED AS WE PROBABLY DO NOT NEED THEM.
     * THERE IS MOST LIKELY NO REASON TO MANIPULATE A DEVICE STRUCTURE
     * ONCE IT IS CREATED.
     *
     * TODO CHECK THIS!
     */

    void setDeviceId(long deviceId);

    void setDeviceNode(const QString &deviceNode);

    void setProductId(long productId);

    void setTabletSerial(long tabletId);

    void setVendorId(long vendorId);

private:
    Q_DECLARE_PRIVATE(DeviceInformation)
    DeviceInformationPrivate *const d_ptr;

}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
