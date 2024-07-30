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

#include "common/deviceproperty.h"

#include <QtTest>

using namespace Wacom;

/**
 * @file testdeviceproperty.cpp
 *
 * @test UnitTest for the device properties
 */
class TestDeviceProperty : public QObject
{
    Q_OBJECT

private slots:
    void test();
};

QTEST_MAIN(TestDeviceProperty)

void TestDeviceProperty::test()
{
    // very basic test to make sure instantiation works
    // the real unit tests can be found in TestPropertySet
    QCOMPARE(DeviceProperty::Button1.key(), QLatin1String("Button1"));
}

#include "testdeviceproperty.moc"
