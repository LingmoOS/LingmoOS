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

#include "common/screenspace.h"

#include <QtTest>

using namespace Wacom;

/**
 * @file testtabletarea.cpp
 *
 * @test UnitTest for the tablet area model class
 */
class TestScreenSpace : public QObject
{
    Q_OBJECT

private slots:
    void testDesktop();
    void testOutput();
};

QTEST_MAIN(TestScreenSpace)

void TestScreenSpace::testDesktop()
{
    QString input = QLatin1String("desktop");

    ScreenSpace space(input);

    QVERIFY(space.getType() == ScreenSpace::ScreenSpaceType::Desktop);
    QCOMPARE(space.toString(), input);
    QVERIFY(space.isDesktop());
}

void TestScreenSpace::testOutput()
{
    QString input = QLatin1String("HDMI-1");

    ScreenSpace space(input);

    QVERIFY(space.getType() == ScreenSpace::ScreenSpaceType::Output);
    QCOMPARE(space.toString(), input);
    QVERIFY(!space.isDesktop());
    QVERIFY(space.isMonitor());
}

#include "testscreenspace.moc"
