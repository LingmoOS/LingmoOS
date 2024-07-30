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

#include "../commontestutils.h"
#include "common/deviceinformation.h"

#include <QtTest>

using namespace Wacom;

/**
 * @file testdeviceinformation.cpp
 *
 * @test UnitTest for the device information
 */
class TestDeviceInformation : public QObject
{
    Q_OBJECT

private slots:
    void testCompare();
    void testConstructor();
    void testCopy();
    void testSetter();
};

QTEST_MAIN(TestDeviceInformation)

void TestDeviceInformation::testCompare()
{
    DeviceType expectedType = DeviceType::Pad;
    QString expectedName = QLatin1String("Test Device");

    DeviceType notExpectedType = DeviceType::Touch;
    QString notExpectedName = QLatin1String("A device name we would not expect.");

    // same type and name
    DeviceInformation expectedDevice(expectedType, expectedName);
    DeviceInformation actualDevice(expectedType, expectedName);
    QVERIFY(actualDevice == expectedDevice);
    QVERIFY(!(actualDevice != expectedDevice));

    // same type, different name
    actualDevice = DeviceInformation(expectedType, notExpectedName);
    QVERIFY(!(actualDevice == expectedDevice));
    QVERIFY(actualDevice != expectedDevice);

    // same name, different type
    actualDevice = DeviceInformation(notExpectedType, expectedName);
    QVERIFY(!(actualDevice == expectedDevice));
    QVERIFY(actualDevice != expectedDevice);

    // same values
    actualDevice = expectedDevice;
    CommonTestUtils::setValues(actualDevice);
    CommonTestUtils::setValues(expectedDevice);
    QVERIFY(actualDevice == expectedDevice);
    QVERIFY(!(actualDevice != expectedDevice));

    // different device id
    actualDevice = expectedDevice;
    actualDevice.setDeviceId(9999);
    QVERIFY(!(actualDevice == expectedDevice));
    QVERIFY(actualDevice != expectedDevice);

    // different device node
    actualDevice = expectedDevice;
    actualDevice.setDeviceNode(QLatin1String("/test/path/to/device"));
    QVERIFY(!(actualDevice == expectedDevice));
    QVERIFY(actualDevice != expectedDevice);

    // different product id
    actualDevice = expectedDevice;
    actualDevice.setProductId(9999);
    QVERIFY(!(actualDevice == expectedDevice));
    QVERIFY(actualDevice != expectedDevice);

    // different tablet serial
    actualDevice = expectedDevice;
    actualDevice.setTabletSerial(9999);
    QVERIFY(!(actualDevice == expectedDevice));
    QVERIFY(actualDevice != expectedDevice);

    // different vendor id
    actualDevice = expectedDevice;
    actualDevice.setVendorId(9999);
    QVERIFY(!(actualDevice == expectedDevice));
    QVERIFY(actualDevice != expectedDevice);
}

void TestDeviceInformation::testConstructor()
{
    DeviceType expectedType = DeviceType::Pad;
    QString expectedName = QLatin1String("Test Device");

    DeviceInformation deviceInfo(expectedType, expectedName);

    QVERIFY(deviceInfo.getType() == expectedType);
    QCOMPARE(deviceInfo.getName(), expectedName);

    QVERIFY(deviceInfo.getDeviceId() == 0);
    QCOMPARE(deviceInfo.getDeviceNode(), QString());
    QVERIFY(deviceInfo.getProductId() == 0);
    QVERIFY(deviceInfo.getTabletSerial() == 0);
    QVERIFY(deviceInfo.getVendorId() == 0);
}

void TestDeviceInformation::testCopy()
{
    DeviceType expectedType = DeviceType::Pad;
    QString expectedName = QLatin1String("Test Device");

    DeviceType notExpectedType = DeviceType::Touch;
    QString notExpectedName = QLatin1String("A device name we would not expect.");

    DeviceInformation expectedInfo(expectedType, expectedName);
    CommonTestUtils::setValues(expectedInfo);

    // test copy constructor
    DeviceInformation actual1Info(expectedInfo);
    CommonTestUtils::assertValues(actual1Info, expectedType, expectedName);

    // test copy operator
    DeviceInformation actual2Info(notExpectedType, notExpectedName);
    actual2Info = expectedInfo;
    CommonTestUtils::assertValues(actual2Info, expectedType, expectedName);
}

void TestDeviceInformation::testSetter()
{
    DeviceType expectedType = DeviceType::Stylus;
    QString expectedName = QLatin1String("Test Device");

    DeviceInformation deviceInfo(expectedType, expectedName);

    CommonTestUtils::setValues(deviceInfo);
    CommonTestUtils::assertValues(deviceInfo, expectedType, expectedName);
}

#include "testdeviceinformation.moc"
