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
#include "common/deviceprofile.h"

#include <QtTest>

using namespace Wacom;

/**
 * @file testdeviceprofile.cpp
 *
 * @test UnitTest for the device profiles
 */
class TestDeviceProfile : public QObject
{
    Q_OBJECT

private slots:
    void testConstructor();
    void testSetter();
    void testCopy();
};

QTEST_MAIN(TestDeviceProfile)

void TestDeviceProfile::testConstructor()
{
    DeviceProfile profile1;
    QVERIFY(profile1.getName().isEmpty());

    DeviceProfile profile2(DeviceType::Stylus);
    QCOMPARE(profile2.getDeviceType(), DeviceType::Stylus);
    QCOMPARE(profile2.getName(), DeviceType::Stylus.key());
}

void TestDeviceProfile::testCopy()
{
    DeviceProfile profile1;
    DeviceProfile profile2;

    CommonTestUtils::setValues(profile1);
    profile2 = profile1;
    CommonTestUtils::assertValues(profile2);

    DeviceProfile profile3(profile2);
    CommonTestUtils::assertValues(profile3);
}

void TestDeviceProfile::testSetter()
{
    DeviceProfile profile;

    CommonTestUtils::setValues(profile);
    CommonTestUtils::assertValues(profile);
}

#include "testdeviceprofile.moc"
