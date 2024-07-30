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

#include "tabletdependenttest.h"
#include "kded/x11tabletfinder.h"

using namespace Wacom;

TabletDependentTest::TabletDependentTest(QObject *parent)
    : QObject(parent)
{
    m_isTabletAvailable = false;
}

const TabletInformation &TabletDependentTest::getTabletInformation() const
{
    return m_tabletInformation;
}

bool TabletDependentTest::isTabletAvailable() const
{
    return m_isTabletAvailable;
}

void TabletDependentTest::findTablet()
{
    X11TabletFinder tabletFinder;
    m_isTabletAvailable = false;

    if (tabletFinder.scanDevices()) {
        m_tabletInformation = tabletFinder.getTablets().at(0);
        m_isTabletAvailable = true;

        printTabletInformation(m_tabletInformation);
    }
}

void TabletDependentTest::printTabletInformation(const TabletInformation &info) const
{
    qDebug() << QString::fromLatin1(
                    "\n\n Tablet Information:"
                    "\n  + Stylus Name   : %1"
                    "\n  + Eraser Name   : %2"
                    "\n  + Touch Name    : %3"
                    "\n  + Pad Name      : %4"
                    "\n  + Cursor Name   : %5"
                    "\n  + Company ID    : %6"
                    "\n  + Company Name  : %7"
                    "\n  + Tablet ID     : %8"
                    "\n  + Tablet Serial : %9"
                    "\n  + Tablet Name   : %10"
                    "\n  + Tablet Model  : %11"
                    "\n")
                    .arg(info.getDeviceName(DeviceType::Stylus))
                    .arg(info.getDeviceName(DeviceType::Eraser))
                    .arg(info.getDeviceName(DeviceType::Touch))
                    .arg(info.getDeviceName(DeviceType::Pad))
                    .arg(info.getDeviceName(DeviceType::Cursor))
                    .arg(info.get(TabletInfo::CompanyId))
                    .arg(info.get(TabletInfo::CompanyName))
                    .arg(info.get(TabletInfo::TabletId))
                    .arg(info.get(TabletInfo::TabletSerial))
                    .arg(info.get(TabletInfo::TabletName))
                    .arg(info.get(TabletInfo::TabletModel));

    foreach (const DeviceType &deviceType, DeviceType::list()) {
        const DeviceInformation *deviceInfo = info.getDevice(deviceType);

        if (deviceInfo == nullptr) {
            continue;
        }

        qDebug() << QString::fromLatin1(
                        "\n\n Device '%1'"
                        "\n  + Device Id   : %2"
                        "\n  + Product Id  : %3"
                        "\n  + Vendor Id   : %4"
                        "\n  + Device Node : %5"
                        "\n")
                        .arg(deviceInfo->getName())
                        .arg(deviceInfo->getDeviceId())
                        .arg(deviceInfo->getProductId())
                        .arg(deviceInfo->getVendorId())
                        .arg(deviceInfo->getDeviceNode());
    }
}

void TabletDependentTest::initTestCase()
{
    findTablet();

    if (!isTabletAvailable()) {
        QSKIP("No tablet found! Can not run this tablet dependent test case!", SkipAll);
        return;
    }

    // printTabletInformation(getTabletInformation());

    initTestCaseDependent();
}

#include "moc_tabletdependenttest.cpp"
