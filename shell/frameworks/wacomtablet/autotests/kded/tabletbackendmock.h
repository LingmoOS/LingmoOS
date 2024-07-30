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

#ifndef TABLETBACKENDMOCK_H
#define TABLETBACKENDMOCK_H

#include "tabletbackendinterface.h"

#include "deviceproperty.h"
#include "propertyadaptormock.h"

#include <QMap>
#include <QString>

namespace Wacom
{
class TabletBackendMock : public TabletBackendInterface
{
public:
    TabletBackendMock();

    ~TabletBackendMock() override;

    void addAdaptor(const DeviceType &deviceType, PropertyAdaptor *adaptor) override;

    const TabletInformation &getInformation() const override;

    const QString getProperty(const DeviceType &type, const Property &property) const override;

    void setProfile(const TabletProfile &profile) override;

    void setProfile(const DeviceType &deviceType, const DeviceProfile &profile) override;

    void setStatusLED(int led) override;

    void setStatusLEDBrightness(int brightness) override;

    bool setProperty(const DeviceType &type, const Property &property, const QString &value) override;

    QString m_propertyAdaptorType; //!< The device type of the property adaptor.
    PropertyAdaptor *m_propertyAdaptor; //!< The property adaptor which was set by addAdaptor()

    TabletInformation m_tabletInformation; //!< The information returned by getInformation()

    TabletProfile m_tabletProfile; //!< The last tablet profile which was set.
    DeviceProfile m_deviceProfile; //!< The last device profile which was set.
    QString m_deviceProfileType; //!< The last device profile type which was set.

    QMap<QString, PropertyAdaptorMock<DeviceProperty> *> m_properties; //!< Properties which were set.

}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
