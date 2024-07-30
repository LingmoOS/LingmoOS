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

#include "tabletbackendmock.h"

using namespace Wacom;

TabletBackendMock::TabletBackendMock()
{
    m_propertyAdaptor = nullptr;
}

TabletBackendMock::~TabletBackendMock()
{
    if (m_propertyAdaptor) {
        delete m_propertyAdaptor;
    }

    QMap<QString, PropertyAdaptorMock<DeviceProperty> *>::iterator it = m_properties.begin();

    while (it != m_properties.end()) {
        delete it.value();
        it = m_properties.erase(it);
    }
}

void TabletBackendMock::addAdaptor(const DeviceType &deviceType, PropertyAdaptor *adaptor)
{
    if (m_propertyAdaptor) {
        delete m_propertyAdaptor;
    }

    m_propertyAdaptor = adaptor;
    m_propertyAdaptorType = deviceType.key();
}

const TabletInformation &TabletBackendMock::getInformation() const
{
    return m_tabletInformation;
}

const QString TabletBackendMock::getProperty(const DeviceType &type, const Property &property) const
{
    QMap<QString, PropertyAdaptorMock<DeviceProperty> *>::const_iterator it = m_properties.find(type.key());

    if (it == m_properties.end()) {
        return QString();
    }

    return it.value()->getProperty(property);
}

void TabletBackendMock::setProfile(const TabletProfile &profile)
{
    m_tabletProfile = profile;
}

void TabletBackendMock::setProfile(const DeviceType &deviceType, const DeviceProfile &profile)
{
    m_deviceProfile = profile;
    m_deviceProfileType = deviceType.key();
}

void TabletBackendMock::setStatusLED(int led)
{
    // doing nothing right now
    Q_UNUSED(led)
}

void TabletBackendMock::setStatusLEDBrightness(int brightness)
{
    // doing nothing right now
    Q_UNUSED(brightness)
}

bool TabletBackendMock::setProperty(const DeviceType &type, const Property &property, const QString &value)
{
    QMap<QString, PropertyAdaptorMock<DeviceProperty> *>::iterator it = m_properties.find(type.key());

    if (it == m_properties.end()) {
        m_properties.insert(type.key(), new PropertyAdaptorMock<DeviceProperty>());
        it = m_properties.find(type.key());
    }

    return it.value()->setProperty(property, value);
}
