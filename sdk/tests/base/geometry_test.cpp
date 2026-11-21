// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <sstream>

#include "base/geometry.h"

using namespace Lingmo::Base;

TEST(GeometryTest, Rounding)
{
    EXPECT_EQ(roundToInt(0.5), 1);
    EXPECT_EQ(roundToInt(-0.5), -1);
    EXPECT_EQ(roundToInt(1.49), 1);
    EXPECT_EQ(roundToInt(-2.5), -3);
    EXPECT_EQ(roundToInt(0.49999999999999994), 1);
    EXPECT_EQ(roundToInt(-0.4), 0);
    EXPECT_EQ(roundToInt(1e300), INT_MAX);
    EXPECT_EQ(roundToInt(-1e300), INT_MIN);
    EXPECT_EQ(roundToInt(std::nan("")), 0);
    EXPECT_EQ(floorToInt(-0.1), -1);
    EXPECT_EQ(ceilToInt(0.1), 1);
    EXPECT_TRUE(fuzzyCompare(1.0, 1.0 + 1e-14));
    EXPECT_FALSE(fuzzyCompare(1.0, 1.001));
    EXPECT_TRUE(fuzzyIsNull(1e-13));
}

TEST(GeometryTest, PointArithmetic)
{
    Point a(3, 4);
    EXPECT_EQ(a + Point(1, 1), Point(4, 5));
    EXPECT_EQ(a - Point(1, 1), Point(2, 3));
    EXPECT_EQ(-a, Point(-3, -4));
    EXPECT_EQ(a.manhattanLength(), 7);
    EXPECT_EQ(a.transposed(), Point(4, 3));
    a += Point(1, 2);
    EXPECT_EQ(a, Point(4, 6));
    EXPECT_TRUE(Point().isOrigin());

    PointF f(1.5, -2.5);
    EXPECT_EQ(f.toPoint(), Point(2, -3));
    EXPECT_EQ(f.toFlooredPoint(), Point(1, -3));
    EXPECT_EQ(f.toCeiledPoint(), Point(2, -2));
    EXPECT_EQ(f * 2, PointF(3, -5));
    EXPECT_EQ(2 * f, PointF(3, -5));
    EXPECT_EQ(f / 2, PointF(0.75, -1.25));
    EXPECT_EQ(PointF(Point(1, 2)), PointF(1, 2));
    EXPECT_DOUBLE_EQ(PointF(3, 4).length(), 5);
}

TEST(GeometryTest, Sizes)
{
    EXPECT_TRUE(Size().isEmpty());
    EXPECT_TRUE(Size(5, 0).isEmpty());
    EXPECT_TRUE(Size(-1, 5).isEmpty());
    EXPECT_FALSE(Size(1, 1).isEmpty());
    EXPECT_EQ(Size(4, 2) - Size(5, 1), Size(-1, 1));
    EXPECT_EQ(Size(4, 2).expandedTo(Size(1, 3)), Size(4, 3));
    EXPECT_EQ(Size(4, 2).boundedTo(Size(1, 3)), Size(1, 2));
    EXPECT_EQ(Size(4, 2).transposed(), Size(2, 4));
    EXPECT_EQ(Size(70000, 70000).area(), 4900000000LL);

    EXPECT_EQ(SizeF(1.5, 2.4).toSize(), Size(2, 2));
    EXPECT_EQ(SizeF(1.5, 2.4).toCeiledSize(), Size(2, 3));
    EXPECT_EQ(SizeF(1.5, 2.4).toFlooredSize(), Size(1, 2));
    EXPECT_EQ(SizeF(2, 3) * 1.5, SizeF(3, 4.5));
    EXPECT_TRUE(SizeF(std::nan(""), 1).isEmpty());
}

TEST(GeometryTest, RectEdgesAreExclusive)
{
    const Rect rect(10, 20, 30, 40);
    EXPECT_EQ(rect.right(), 40);
    EXPECT_EQ(rect.bottom(), 60);
    EXPECT_EQ(rect.topRight(), Point(40, 20));
    EXPECT_EQ(rect.bottomRight(), Point(40, 60));
    EXPECT_EQ(rect.bottomLeft(), Point(10, 60));
    EXPECT_EQ(rect.center(), Point(25, 40));
    EXPECT_EQ(Rect(0, 0, 5, 5).center(), Point(2, 2));
    EXPECT_EQ(Rect(0, 0, 5, 5).centerF(), PointF(2.5, 2.5));
    EXPECT_TRUE(rect.contains(Point(10, 20)));
    EXPECT_TRUE(rect.contains(Point(39, 59)));
    EXPECT_FALSE(rect.contains(Point(40, 59)));
    EXPECT_FALSE(rect.contains(Point(39, 60)));
}

TEST(GeometryTest, RectClampsNegativeAndOverflowingSizes)
{
    EXPECT_EQ(Rect(5, 5, -3, 4).width(), 0);
    EXPECT_TRUE(Rect(5, 5, -3, 4).isEmpty());
    const Rect huge(10, 0, INT_MAX, INT_MAX);
    EXPECT_EQ(huge.right(), INT_MAX);
    EXPECT_EQ(huge.width(), INT_MAX - 10);
    const Rect negative(-10, -10, INT_MAX, INT_MAX);
    EXPECT_EQ(negative.width(), INT_MAX);
    EXPECT_EQ(Rect::fromEdges(INT_MIN, 0, INT_MAX, 1).width(), INT_MAX);
    EXPECT_EQ(Rect::fromEdges(10, 10, 5, 20), Rect(10, 10, 0, 10));
}

TEST(GeometryTest, RectSettersAndMoves)
{
    Rect rect(10, 10, 20, 20);
    rect.setLeft(5);
    EXPECT_EQ(rect, Rect(5, 10, 25, 20));
    rect.setRight(40);
    EXPECT_EQ(rect, Rect(5, 10, 35, 20));
    rect.setTop(0);
    EXPECT_EQ(rect, Rect(5, 0, 35, 30));
    rect.setBottom(10);
    EXPECT_EQ(rect, Rect(5, 0, 35, 10));
    rect.moveRight(100);
    EXPECT_EQ(rect, Rect(65, 0, 35, 10));
    rect.moveBottom(50);
    EXPECT_EQ(rect, Rect(65, 40, 35, 10));
    rect.moveCenter(Point(0, 0));
    EXPECT_EQ(rect, Rect(-17, -5, 35, 10));
    rect.moveTo(1, 2);
    EXPECT_EQ(rect, Rect(1, 2, 35, 10));
    EXPECT_EQ(rect.translated(Point(1, 1)), Rect(2, 3, 35, 10));
    EXPECT_EQ(rect.adjusted(1, 1, -1, -1), Rect(2, 3, 33, 8));
    EXPECT_EQ(rect.marginsAdded(Insets(1, 2, 3, 4)), Rect(0, 0, 39, 16));
    EXPECT_EQ(rect.marginsRemoved(Insets::uniform(1)), Rect(2, 3, 33, 8));
    EXPECT_EQ(rect.transposed(), Rect(1, 2, 10, 35));
    EXPECT_EQ(Rect(Size(3, 4)), Rect(0, 0, 3, 4));
    EXPECT_EQ(Rect::boundingRect(Point(5, 1), Point(1, 5)), Rect(1, 1, 4, 4));
}

TEST(GeometryTest, RectSetOperations)
{
    const Rect a(0, 0, 10, 10);
    const Rect b(5, 5, 10, 10);
    EXPECT_TRUE(a.intersects(b));
    EXPECT_EQ(a.intersected(b), Rect(5, 5, 5, 5));
    EXPECT_EQ(a & b, Rect(5, 5, 5, 5));
    EXPECT_EQ(a.united(b), Rect(0, 0, 15, 15));
    EXPECT_FALSE(a.intersects(Rect(10, 0, 5, 5)));
    EXPECT_EQ(a.intersected(Rect(10, 0, 5, 5)), Rect());
    // An empty operand does not stretch the union.
    EXPECT_EQ(a.united(Rect(100, 100, 0, 5)), a);
    EXPECT_EQ(Rect().united(b), b);
    EXPECT_FALSE(a.intersects(Rect(5, 5, 0, 0)));
    EXPECT_TRUE(a.contains(Rect(0, 0, 10, 10)));
    EXPECT_FALSE(a.contains(Rect(0, 0, 11, 10)));
    EXPECT_TRUE(a.contains(Rect(10, 10, 0, 0)));
}

TEST(GeometryTest, RectFConversions)
{
    const RectF rect(0.4, 0.6, 10.2, 10.3);
    EXPECT_EQ(rect.toEnclosingRect(), Rect(0, 0, 11, 11));
    EXPECT_EQ(rect.toEnclosedRect(), Rect(1, 1, 9, 9));
    EXPECT_EQ(rect.toRoundedRect(), Rect(0, 1, 11, 10));
    // Origin rounded; size rounded with half the origin's error added.
    EXPECT_EQ(rect.toNearestRect(), Rect(0, 1, 10, 10));
    EXPECT_EQ(RectF(0.5, 0.5, 1.5, 1.5).toNearestRect(), Rect(1, 1, 1, 1));
    EXPECT_EQ(RectF(-0.5, -0.5, 1, 1).toEnclosingRect(), Rect(-1, -1, 2, 2));
    EXPECT_EQ(RectF(Rect(1, 2, 3, 4)), RectF(1, 2, 3, 4));
    EXPECT_EQ(scaleToEnclosingRect(Rect(1, 1, 3, 3), 1.5), Rect(1, 1, 5, 5));
    EXPECT_EQ(scaleToEnclosedRect(Rect(1, 1, 3, 3), 1.5), Rect(2, 2, 4, 4));
    EXPECT_EQ(scaleToRoundedRect(Rect(1, 1, 3, 3), 1.5), Rect(2, 2, 4, 4));
}

TEST(GeometryTest, RectFOperations)
{
    RectF rect(0, 0, 10, 10);
    EXPECT_TRUE(rect.contains(PointF(9.99, 0)));
    EXPECT_FALSE(rect.contains(PointF(10, 0)));
    EXPECT_EQ(rect.intersected(RectF(5, 5, 10, 10)), RectF(5, 5, 5, 5));
    EXPECT_EQ(rect.united(RectF(5, 5, 10, 10)), RectF(0, 0, 15, 15));
    EXPECT_EQ(rect.scaled(1.5), RectF(0, 0, 15, 15));
    EXPECT_EQ(RectF(1, 1, 2, 2).scaled(2, 3), RectF(2, 3, 4, 6));
    EXPECT_EQ(RectF(1, 1, -2, 2).width(), 0);
    EXPECT_EQ(RectF::boundingRect(PointF(3, 0), PointF(1, 2)), RectF(1, 0, 2, 2));
    EXPECT_EQ(rect.marginsRemoved(InsetsF::uniform(0.5)), RectF(0.5, 0.5, 9, 9));
    EXPECT_EQ(rect.center(), PointF(5, 5));
    rect.moveCenter(PointF(0, 0));
    EXPECT_EQ(rect, RectF(-5, -5, 10, 10));
}

TEST(GeometryTest, Printing)
{
    std::ostringstream stream;
    stream << Rect(1, 2, 3, 4) << ' ' << PointF(0.5, 1) << ' ' << Size(3, 4);
    EXPECT_EQ(stream.str(), "1,2 3x4 0.5,1 3x4");
    EXPECT_EQ(Insets(1, 2, 3, 4).toString(), "l=1 t=2 r=3 b=4");
}

TEST(GeometryTest, FuzzyEquality)
{
    EXPECT_TRUE(fuzzyCompare(PointF(1, 2), PointF(1 + 1e-14, 2)));
    EXPECT_FALSE(fuzzyCompare(PointF(1, 2), PointF(1.0001, 2)));
    // Against 0 the difference itself must be negligible.
    EXPECT_TRUE(fuzzyCompare(PointF(0, 0), PointF(1e-13, -1e-13)));
    EXPECT_FALSE(fuzzyCompare(PointF(0, 0), PointF(1e-11, 0)));
    EXPECT_TRUE(fuzzyCompare(SizeF(0.1 + 0.2, 4), SizeF(0.3, 4)));
    EXPECT_FALSE(fuzzyCompare(SizeF(3, 4), SizeF(3, 4.5)));
    // Origin and size: a large origin does not hide a small size error.
    EXPECT_TRUE(fuzzyCompare(RectF(1e6, 0, 10, 10), RectF(1e6 + 1e-7, 0, 10, 10)));
    EXPECT_FALSE(fuzzyCompare(RectF(1e6, 0, 10, 10), RectF(1e6, 0, 10 + 1e-7, 10)));
    EXPECT_TRUE(fuzzyCompareCorners(RectF(1e6, 0, 10, 10), RectF(1e6, 0, 10 + 1e-7, 10)));
}
