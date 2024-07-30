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

#include "tabletbackendfactory.h"

#include "tabletbackend.h"
#include "tabletdatabase.h"
#include "xinputadaptor.h"
#include "xsetwacomadaptor.h"

using namespace Wacom;

TabletBackendInterface *TabletBackendFactory::m_tabletBackendMock = nullptr;
bool TabletBackendFactory::m_isUnitTest = false;

TabletBackendFactory::TabletBackendFactory()
{
}

TabletBackendInterface *TabletBackendFactory::createBackend(const TabletInformation &info)
{
    static TabletBackendFactory factory;

    // return mock object if it exists or if unit testing is enabled
    if (m_tabletBackendMock || m_isUnitTest) {
        // reset tablet backend mock as it will get deleted by the caller
        // therefore we can not return it more than once
        TabletBackendInterface *mock = m_tabletBackendMock;
        m_tabletBackendMock = nullptr;
        return mock;
    }

    // create tablet backend
    return factory.createInstance(info);
}

void TabletBackendFactory::setTabletBackendMock(TabletBackendInterface *mock)
{
    m_tabletBackendMock = mock;
}

void TabletBackendFactory::setUnitTest(bool isUnitTest)
{
    m_isUnitTest = isUnitTest;
}

TabletBackendInterface *TabletBackendFactory::createInstance(const TabletInformation &info)
{
    QString deviceName;
    TabletBackend *backend = new TabletBackend(info);

    foreach (const DeviceType &type, DeviceType::list()) {
        if (!info.hasDevice(type)) {
            continue;
        }

        deviceName = info.getDeviceName(type);

        if (type == DeviceType::Pad) {
            backend->addAdaptor(type, new XsetwacomAdaptor(deviceName, info.getButtonMap()));

        } else if (type == DeviceType::Stylus || type == DeviceType::Eraser || type == DeviceType::Touch) {
            backend->addAdaptor(type, new XsetwacomAdaptor(deviceName));
            backend->addAdaptor(type, new XinputAdaptor(deviceName));

        } else {
            backend->addAdaptor(type, new XsetwacomAdaptor(deviceName));
        }
    }

    return backend;
}
