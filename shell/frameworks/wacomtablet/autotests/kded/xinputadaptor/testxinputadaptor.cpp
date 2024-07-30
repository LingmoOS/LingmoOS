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

#include "kded/xinputadaptor.h"
#include "kded/xinputproperty.h"

#include "../tabletdependenttest.h"

#include <QtTest>

using namespace Wacom;

/**
 * @file testxinputadaptor.cpp
 *
 * @test UnitTest for ...
 */
class TestXinputAdaptor : public TabletDependentTest
{
    Q_OBJECT

protected:
    void initTestCaseDependent() override;

private slots:
    void testSetProperty();
};

QTEST_MAIN(TestXinputAdaptor)

void TestXinputAdaptor::initTestCaseDependent()
{
    TabletInformation info = getTabletInformation();

    // check prerequisites
    bool isValid = true;

    if (info.getDeviceName(DeviceType::Stylus).isEmpty()) {
        isValid = false;
    }

    if (!isValid) {
        QSKIP("Can not run test! The currently connected device does not have all prerequisites!", SkipAll);
        return;
    }
}

void TestXinputAdaptor::testSetProperty()
{
    TabletInformation info = getTabletInformation();

    QVERIFY(!info.getDeviceName(DeviceType::Stylus).isEmpty());

    XinputAdaptor adaptor(info.getDeviceName(DeviceType::Stylus));

    QVERIFY(!adaptor.getProperty(Property::CursorAccelProfile).isEmpty());
    QVERIFY(!adaptor.getProperty(Property::CursorAccelAdaptiveDeceleration).isEmpty());
    QVERIFY(!adaptor.getProperty(Property::CursorAccelConstantDeceleration).isEmpty());
    QVERIFY(!adaptor.getProperty(Property::CursorAccelVelocityScaling).isEmpty());

    adaptor.setProperty(Property::CursorAccelProfile, QLatin1String("2"));
    adaptor.setProperty(Property::CursorAccelAdaptiveDeceleration, QLatin1String("5.1"));
    adaptor.setProperty(Property::CursorAccelConstantDeceleration, QLatin1String("5.2"));
    adaptor.setProperty(Property::CursorAccelVelocityScaling, QLatin1String("5.3"));

    QCOMPARE(adaptor.getProperty(Property::CursorAccelProfile), QLatin1String("2"));
    QCOMPARE(adaptor.getProperty(Property::CursorAccelAdaptiveDeceleration), QLatin1String("5.1"));
    QCOMPARE(adaptor.getProperty(Property::CursorAccelConstantDeceleration), QLatin1String("5.2"));
    QCOMPARE(adaptor.getProperty(Property::CursorAccelVelocityScaling), QLatin1String("5.3"));
}

#include "testxinputadaptor.moc"
