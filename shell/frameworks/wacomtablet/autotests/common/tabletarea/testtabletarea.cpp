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

#include "common/tabletarea.h"

#include <QtTest>

using namespace Wacom;

/**
 * @file testtabletarea.cpp
 *
 * @test UnitTest for the tablet area model class
 */
class TestTabletArea : public QObject
{
    Q_OBJECT

private slots:

    void testAssignment();

    void testFromString();

    void testToString();

private:
    void assertArea(const TabletArea &area, int x, int y, int width, int height, bool assertEmpty = false);
};

QTEST_MAIN(TestTabletArea)

void TestTabletArea::testAssignment()
{
    TabletArea area1;
    QVERIFY(area1.isEmpty());

    TabletArea area2(QRect(0, 0, 800, 600));
    assertArea(area2, 0, 0, 800, 600);

    TabletArea area3(area2);
    assertArea(area3, 0, 0, 800, 600);

    QVERIFY(area1 != area2);
    QVERIFY(area2 == area3);

    TabletArea area4;
    QVERIFY(area2 != area4);

    area4 = area2;
    QVERIFY(area4 == area2);

    area4 = area1;
    QVERIFY(area4 == area1);

    TabletArea area5(QRect(-1, -1, -1, -1));
    assertArea(area5, 0, 0, 0, 0, true);

    TabletArea area6(QRect(0, 0, -20, -20));
    assertArea(area6, 0, 0, 0, 0, true);

    TabletArea area7(QRect(-5, -5, 20, 20));
    assertArea(area7, -5, -5, 20, 20);
}

void TestTabletArea::testFromString()
{
    TabletArea area1(QLatin1String("test"));
    assertArea(area1, 0, 0, 0, 0, true);

    TabletArea area2(QLatin1String("4 values 2 test"));
    assertArea(area2, 0, 0, 0, 0, true);

    TabletArea area3(QString(), QRect(0, 0, 800, 600));
    assertArea(area3, 0, 0, 800, 600);

    TabletArea area4(QLatin1String("4 3 2"), QRect(10, 10, 500, 400));
    assertArea(area4, 10, 10, 500, 400);

    TabletArea area5(QLatin1String("10 10 410 510"), QRect(0, 0, 100, 100));
    assertArea(area5, 10, 10, 400, 500);

    TabletArea area6(QLatin1String("0 0 0 0"), QRect(0, 0, 100, 200));
    assertArea(area6, 0, 0, 100, 200);

    TabletArea area7(QLatin1String("-1 -1 -1 -1"), QRect(0, 0, 100, 200));
    assertArea(area7, 0, 0, 100, 200);

    TabletArea area8(QLatin1String("-1 -1 -2 -2"), QRect(0, 0, 100, 200));
    assertArea(area8, 0, 0, 100, 200);

    TabletArea area9(QLatin1String("-1 -1 -2 -2"));
    assertArea(area9, 0, 0, 0, 0, true);

    TabletArea area10;

    area10.fromString(QString(), QRect(0, 0, 800, 600));
    assertArea(area10, 0, 0, 800, 600);

    area10.fromString(QLatin1String("4 3 2"), QRect(10, 10, 500, 400));
    assertArea(area10, 10, 10, 500, 400);

    area10.fromString(QLatin1String("10 10 410 510"), QRect(0, 0, 100, 100));
    assertArea(area10, 10, 10, 400, 500);

    area10.fromString(QLatin1String("0 0 0 0"), QRect(0, 0, 100, 200));
    assertArea(area10, 0, 0, 100, 200);

    area10.fromString(QLatin1String("-1 -1 -1 -1"), QRect(0, 0, 100, 200));
    assertArea(area10, 0, 0, 100, 200);

    area10.fromString(QLatin1String("-1 -1 -2 -2"), QRect(0, 0, 100, 200));
    assertArea(area8, 0, 0, 100, 200);

    area10.fromString(QLatin1String("-1 -1 -2 -2"));
    assertArea(area10, 0, 0, 0, 0, true);

    area10.fromString(QLatin1String("4 values 2 test"));
    assertArea(area10, 0, 0, 0, 0, true);
}

void TestTabletArea::testToString()
{
    TabletArea area1(QRect(0, 0, 100, 200));
    QString area1String = area1.toString();
    QCOMPARE(area1String, QLatin1String("0 0 100 200"));

    TabletArea area2(QRect(10, 20, 500, 400));
    QString area2String = area2.toString();
    QCOMPARE(area2String, QLatin1String("10 20 510 420"));

    TabletArea area3(QRect(-10, -20, 500, 400));
    QString area3String = area3.toString();
    QCOMPARE(area3String, QLatin1String("-10 -20 490 380"));
}

void TestTabletArea::assertArea(const TabletArea &area, int x, int y, int width, int height, bool assertEmpty)
{
    if (assertEmpty) {
        QVERIFY(area.isEmpty());
    } else {
        QVERIFY(!area.isEmpty());
    }

    QCOMPARE(area.x(), x);
    QCOMPARE(area.y(), y);
    QCOMPARE(area.width(), width);
    QCOMPARE(area.height(), height);
}

#include "testtabletarea.moc"
