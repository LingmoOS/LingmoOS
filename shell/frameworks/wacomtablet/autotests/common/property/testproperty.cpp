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

#include "common/property.h"

#include <QtTest>

using namespace Wacom;

/**
 * @file testproperty.cpp
 *
 * @test UnitTest for the properties
 */
class TestProperty : public QObject
{
    Q_OBJECT

private slots:
    void test();
};

QTEST_MAIN(TestProperty)

void TestProperty::test()
{
    /*
     * Just some very basic tests to test the functors and proper class initialization.
     * Most code should have been covered already by TestEnum.
     */
    Property test = Property::AbsWheelUp;
    QVERIFY(test == test);
    QVERIFY(test == Property::AbsWheelUp);
    QVERIFY(test != Property::AbsWheelDown);
    QVERIFY(test.key() == Property::AbsWheelUp.key());

    const Property *nonexistent = Property::find(QLatin1String("NON_EXISTANT"));
    QVERIFY(nonexistent == NULL);

    const Property *existent = Property::find(QLatin1String("touch"));
    QVERIFY(existent != NULL);
    QVERIFY(*existent == Property::Touch);

    QVERIFY(Property::size() == Property::keys().size());
    QVERIFY(Property::size() == Property::list().size());
}

#include "testproperty.moc"
