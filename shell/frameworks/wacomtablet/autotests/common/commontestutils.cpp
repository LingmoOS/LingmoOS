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

#include "commontestutils.h"

#include <QtTest>

#include "common/deviceproperty.h"

using namespace Wacom;

const long CommonTestUtils::DEVICEINFORMATION_DEVICE_ID = 42;
const QString CommonTestUtils::DEVICEINFORMATION_DEVICE_NODE = QLatin1String("/dev/input/event1");
const long CommonTestUtils::DEVICEINFORMATION_PRODUCT_ID = 1234;
const long CommonTestUtils::DEVICEINFORMATION_TABLET_SERIAL = 123456;
const long CommonTestUtils::DEVICEINFORMATION_VENDOR_ID = 4321;

const QString CommonTestUtils::TABLETINFORMATION_COMPANY_ID = QLatin1String("1234");
const QString CommonTestUtils::TABLETINFORMATION_COMPANY_NAME = QLatin1String("Wacom Ltd.");
const QString CommonTestUtils::TABLETINFORMATION_TABLET_MODEL = QLatin1String("Bamboo Create");
const QString CommonTestUtils::TABLETINFORMATION_TABLET_NAME = QLatin1String("Bamboo Touch");
const QString CommonTestUtils::TABLETINFORMATION_TABLET_SERIAL = QLatin1String("6666");
const QString CommonTestUtils::TABLETINFORMATION_TABLET_ID = QString::fromLatin1("%1").arg(6666, 4, 16, QLatin1Char('0')).toUpper();
const bool CommonTestUtils::TABLETINFORMATION_HAS_BUTTONS = true;
const bool CommonTestUtils::TABLETINFORMATION_HAS_TOUCHRING = true;
const bool CommonTestUtils::TABLETINFORMATION_HAS_TOUCHSTRIPLEFT = true;
const bool CommonTestUtils::TABLETINFORMATION_HAS_TOUCHSTRIPRIGHT = true;
const bool CommonTestUtils::TABLETINFORMATION_HAS_WHEEL = true;
const bool CommonTestUtils::TABLETINFORMATION_IS_AVAILABLE = true;
const QString CommonTestUtils::TABLETINFORMATION_NUM_PAD_BUTTONS = QLatin1String("4");

const QString CommonTestUtils::TABLETINFORMATION_DEV1_NAME = QLatin1String("Device Stylus");
const QString CommonTestUtils::TABLETINFORMATION_DEV1_TYPE = QLatin1String("stylus");

const QString CommonTestUtils::TABLETINFORMATION_DEV2_NAME = QLatin1String("Device Eraser");
const QString CommonTestUtils::TABLETINFORMATION_DEV2_TYPE = QLatin1String("eraser");

const QString CommonTestUtils::TABLETINFORMATION_DEV3_NAME = QLatin1String("Device Pad");
const QString CommonTestUtils::TABLETINFORMATION_DEV3_TYPE = QLatin1String("pad");

void CommonTestUtils::assertValues(const DeviceInformation &info, const DeviceType &expectedType, const QString &expectedName)
{
    QVERIFY(info.getDeviceId() == DEVICEINFORMATION_DEVICE_ID);
    QCOMPARE(info.getDeviceNode(), DEVICEINFORMATION_DEVICE_NODE);
    QCOMPARE(info.getName(), expectedName);
    QVERIFY(info.getProductId() == DEVICEINFORMATION_PRODUCT_ID);
    QVERIFY(info.getTabletSerial() == DEVICEINFORMATION_TABLET_SERIAL);
    QVERIFY(info.getType() == expectedType);
    QVERIFY(info.getVendorId() == DEVICEINFORMATION_VENDOR_ID);
}

void CommonTestUtils::assertValues(DeviceProfile &profile, const char *name)
{
    foreach (const DeviceProperty &property, DeviceProperty::list()) {
        QCOMPARE(profile.getProperty(property.id()), property.id().key());
    }

    if (name != nullptr) {
        QCOMPARE(profile.getName(), QLatin1String(name));
    } else {
        QCOMPARE(profile.getName(), DeviceType::Pad.key());
    }
}

void CommonTestUtils::assertValues(const TabletInformation &info)
{
    QCOMPARE(info.get(TabletInfo::CompanyId), TABLETINFORMATION_COMPANY_ID);
    QCOMPARE(info.get(TabletInfo::CompanyName), TABLETINFORMATION_COMPANY_NAME);
    QCOMPARE(info.get(TabletInfo::TabletId), TABLETINFORMATION_TABLET_ID);
    QCOMPARE(info.get(TabletInfo::TabletModel), TABLETINFORMATION_TABLET_MODEL);
    QCOMPARE(info.get(TabletInfo::TabletName), TABLETINFORMATION_TABLET_NAME);
    QCOMPARE(info.get(TabletInfo::TabletSerial), TABLETINFORMATION_TABLET_SERIAL);

    QVERIFY(info.isAvailable() == TABLETINFORMATION_IS_AVAILABLE);
    QVERIFY(info.hasButtons() == TABLETINFORMATION_HAS_BUTTONS);

    QStringList deviceList = info.getDeviceList();
    QVERIFY(deviceList.size() == 3);
    QVERIFY(deviceList.contains(TABLETINFORMATION_DEV1_NAME));
    QVERIFY(deviceList.contains(TABLETINFORMATION_DEV2_NAME));
    QVERIFY(deviceList.contains(TABLETINFORMATION_DEV3_NAME));
}

void CommonTestUtils::setValues(DeviceInformation &info)
{
    info.setDeviceId(DEVICEINFORMATION_DEVICE_ID);
    info.setDeviceNode(DEVICEINFORMATION_DEVICE_NODE);
    info.setProductId(DEVICEINFORMATION_PRODUCT_ID);
    info.setTabletSerial(DEVICEINFORMATION_TABLET_SERIAL);
    info.setVendorId(DEVICEINFORMATION_VENDOR_ID);
}

void CommonTestUtils::setValues(DeviceProfile &profile)
{
    foreach (const DeviceProperty &property, DeviceProperty::list()) {
        profile.setProperty(property.id(), property.id().key());
    }

    if (profile.getDeviceType() == DeviceType::Unknown) {
        profile.setDeviceType(DeviceType::Pad);
    }
}

void CommonTestUtils::setValues(TabletInformation &info)
{
    info.set(TabletInfo::CompanyId, TABLETINFORMATION_COMPANY_ID);
    info.set(TabletInfo::CompanyName, TABLETINFORMATION_COMPANY_NAME);
    info.set(TabletInfo::TabletModel, TABLETINFORMATION_TABLET_MODEL);
    info.set(TabletInfo::TabletName, TABLETINFORMATION_TABLET_NAME);
    info.set(TabletInfo::TabletSerial, TABLETINFORMATION_TABLET_SERIAL);

    info.set(TabletInfo::HasLeftTouchStrip, TABLETINFORMATION_HAS_TOUCHSTRIPLEFT);
    info.set(TabletInfo::HasRightTouchStrip, TABLETINFORMATION_HAS_TOUCHSTRIPRIGHT);
    info.set(TabletInfo::HasTouchRing, TABLETINFORMATION_HAS_TOUCHRING);
    info.set(TabletInfo::HasWheel, TABLETINFORMATION_HAS_WHEEL);

    info.setAvailable(TABLETINFORMATION_IS_AVAILABLE);

    DeviceInformation dev1Info(*DeviceType::find(TABLETINFORMATION_DEV1_TYPE), TABLETINFORMATION_DEV1_NAME);
    DeviceInformation dev2Info(*DeviceType::find(TABLETINFORMATION_DEV2_TYPE), TABLETINFORMATION_DEV2_NAME);
    DeviceInformation dev3Info(*DeviceType::find(TABLETINFORMATION_DEV3_TYPE), TABLETINFORMATION_DEV3_NAME);

    info.setDevice(dev1Info);
    info.setDevice(dev2Info);
    info.setDevice(dev3Info);
}
