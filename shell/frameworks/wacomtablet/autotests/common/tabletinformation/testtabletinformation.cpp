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
#include "common/devicetype.h"
#include "common/tabletinfo.h"
#include "common/tabletinformation.h"

#include <QtTest>

using namespace Wacom;

/**
 * @file testtabletinformation.cpp
 *
 * @test UnitTest for the profile manager
 */
class TestTabletInformation : public QObject
{
    Q_OBJECT

private slots:
    void testCompare();
    void testConstructor();
    void testCopy();
    void testSetter();
};

QTEST_MAIN(TestTabletInformation)

void TestTabletInformation::testCompare()
{
    TabletInformation tabletInfo1;
    TabletInformation tabletInfo2;

    CommonTestUtils::setValues(tabletInfo1);
    CommonTestUtils::assertValues(tabletInfo1);
    QVERIFY(tabletInfo1 != tabletInfo2);

    CommonTestUtils::setValues(tabletInfo2);
    CommonTestUtils::assertValues(tabletInfo2);
    QVERIFY(tabletInfo1 == tabletInfo2);

    // changing the available flag shouldn't change anything
    tabletInfo2.setAvailable(!CommonTestUtils::TABLETINFORMATION_IS_AVAILABLE);
    QVERIFY(tabletInfo1 == tabletInfo2);
}

void TestTabletInformation::testConstructor()
{
    // default constructor
    TabletInformation tabletInfo1;

    QVERIFY(tabletInfo1.getTabletSerial() == 0);

    foreach (const TabletInfo &info, TabletInfo::list()) {
        QVERIFY(tabletInfo1.get(info).isEmpty());
    }

    // serial constructor
    long expectedSerial = 1234;
    TabletInformation tabletInfo2(expectedSerial);

    QVERIFY(tabletInfo2.getTabletSerial() == expectedSerial);

    foreach (const TabletInfo &info, TabletInfo::list()) {
        QString value = tabletInfo2.get(info);

        if (info == TabletInfo::TabletSerial) {
            QVERIFY(value == QString::number(expectedSerial));
        } else if (info == TabletInfo::TabletId) {
            QVERIFY(value == QString::fromLatin1("%1").arg(expectedSerial, 4, 16, QLatin1Char('0')).toUpper());
        } else {
            QVERIFY(value.isEmpty());
        }
    }
}

void TestTabletInformation::testCopy()
{
    TabletInformation tabletInfo1;

    CommonTestUtils::setValues(tabletInfo1);
    CommonTestUtils::assertValues(tabletInfo1);

    // Copy Constructor
    TabletInformation tabletInfo2(tabletInfo1);
    CommonTestUtils::assertValues(tabletInfo2);

    // Copy Operator
    TabletInformation tabletInfo3;
    tabletInfo3 = tabletInfo1;
    CommonTestUtils::assertValues(tabletInfo3);
}

void TestTabletInformation::testSetter()
{
    TabletInformation info;

    CommonTestUtils::setValues(info);
    CommonTestUtils::assertValues(info);

    DeviceType dev1Type = *DeviceType::find(CommonTestUtils::TABLETINFORMATION_DEV1_TYPE);
    DeviceType dev2Type = *DeviceType::find(CommonTestUtils::TABLETINFORMATION_DEV2_TYPE);
    DeviceType dev3Type = *DeviceType::find(CommonTestUtils::TABLETINFORMATION_DEV3_TYPE);

    // getDevice()
    const DeviceInformation *dev1Info = info.getDevice(dev1Type);
    const DeviceInformation *dev2Info = info.getDevice(dev2Type);
    const DeviceInformation *dev3Info = info.getDevice(dev3Type);
    QVERIFY(dev1Info != NULL);
    QVERIFY(dev2Info != NULL);
    QVERIFY(dev3Info != NULL);

    // hasDevice()
    QVERIFY(info.hasDevice(dev1Type));
    QVERIFY(info.hasDevice(dev1Info->getDeviceId()));
    QVERIFY(info.hasDevice(dev2Type));
    QVERIFY(info.hasDevice(dev2Info->getDeviceId()));
    QVERIFY(info.hasDevice(dev3Type));
    QVERIFY(info.hasDevice(dev3Info->getDeviceId()));

    // getDeviceName()
    QCOMPARE(info.getDeviceName(dev1Type), CommonTestUtils::TABLETINFORMATION_DEV1_NAME);
    QCOMPARE(info.getDeviceName(dev2Type), CommonTestUtils::TABLETINFORMATION_DEV2_NAME);
    QCOMPARE(info.getDeviceName(dev3Type), CommonTestUtils::TABLETINFORMATION_DEV3_NAME);

    // getTabletSerial()
    QVERIFY(info.getTabletSerial() == CommonTestUtils::TABLETINFORMATION_TABLET_SERIAL.toLong());
}

#include "testtabletinformation.moc"
