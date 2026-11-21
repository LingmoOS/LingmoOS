// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "base/transform.h"

using namespace Lingmo::Base;

using Type = AffineTransform::Type;

TEST(TransformTest, Types)
{
    EXPECT_EQ(AffineTransform().type(), Type::Identity);
    EXPECT_EQ(AffineTransform::fromTranslate(1, 0).type(), Type::Translate);
    EXPECT_EQ(AffineTransform::fromScale(2, 1).type(), Type::Scale);
    EXPECT_EQ(AffineTransform::fromRotation(90).type(), Type::Rotate);
    EXPECT_EQ(AffineTransform(1, 0.5, 0, 1, 0, 0).type(), Type::Shear);
    // Within the tolerance counts as nothing.
    EXPECT_EQ(AffineTransform(1, 1e-13, 0, 1, 1e-13, 0).type(), Type::Identity);
    EXPECT_TRUE(AffineTransform().isIdentity());
    EXPECT_TRUE(AffineTransform::fromRotation(30).isRotating());
}

TEST(TransformTest, RightAnglesAreExact)
{
    const AffineTransform quarter = AffineTransform::fromRotation(90);
    EXPECT_EQ(quarter, AffineTransform(0, 1, -1, 0, 0, 0));
    // Clockwise on a y-down screen: +x turns into +y.
    EXPECT_EQ(quarter.map(PointF(1, 0)), PointF(0, 1));
    EXPECT_EQ(AffineTransform::fromRotation(180), AffineTransform(-1, 0, 0, -1, 0, 0));
    EXPECT_EQ(AffineTransform::fromRotation(-90), AffineTransform(0, -1, 1, 0, 0, 0));
    EXPECT_EQ(AffineTransform::fromRotation(270), AffineTransform::fromRotation(-90));
}

TEST(TransformTest, StepsApplyBeforeTheTransform)
{
    AffineTransform transform = AffineTransform::fromScale(2, 3);
    transform.translate(10, 20);
    // Translated first, then scaled.
    EXPECT_EQ(transform.map(PointF(0, 0)), PointF(20, 60));
    EXPECT_EQ(transform, AffineTransform::fromTranslate(10, 20) * AffineTransform::fromScale(2, 3));

    AffineTransform turned = AffineTransform::fromTranslate(100, 0);
    turned.rotate(90);
    EXPECT_EQ(turned.map(PointF(1, 0)), PointF(100, 1));
    turned.scale(2, 1);
    EXPECT_EQ(turned.map(PointF(1, 0)), PointF(100, 2));
}

TEST(TransformTest, CompositionOrder)
{
    const AffineTransform a = AffineTransform::fromTranslate(5, 0);
    const AffineTransform b = AffineTransform::fromScale(2, 2);
    // a then b.
    EXPECT_EQ((a * b).map(PointF(1, 1)), PointF(12, 2));
    EXPECT_EQ((b * a).map(PointF(1, 1)), PointF(7, 2));
    const AffineTransform r = AffineTransform::fromRotation(90);
    EXPECT_EQ((a * r).map(PointF(0, 0)), PointF(0, 5));
    AffineTransform c = a;
    c *= r;
    EXPECT_EQ(c, a * r);
}

TEST(TransformTest, Inverse)
{
    bool invertible = false;
    const AffineTransform scale = AffineTransform(2, 0, 0, 4, 6, 8);
    EXPECT_EQ(scale.inverted(&invertible), AffineTransform(0.5, 0, 0, 0.25, -3, -2));
    EXPECT_TRUE(invertible);

    AffineTransform general = AffineTransform::fromTranslate(3, 4);
    general.rotate(30);
    general.scale(2, 0.5);
    const AffineTransform inverse = general.inverted(&invertible);
    EXPECT_TRUE(invertible);
    const PointF back = inverse.map(general.map(PointF(7, -3)));
    EXPECT_NEAR(back.x(), 7, 1e-12);
    EXPECT_NEAR(back.y(), -3, 1e-12);

    EXPECT_EQ(AffineTransform(1, 1, 1, 1, 0, 0).inverted(&invertible), AffineTransform());
    EXPECT_FALSE(invertible);
    EXPECT_EQ(AffineTransform::fromScale(0, 1).inverted(&invertible), AffineTransform());
    EXPECT_FALSE(invertible);
    EXPECT_EQ(AffineTransform::fromTranslate(1, 2).inverted(), AffineTransform::fromTranslate(-1, -2));
}

TEST(TransformTest, MapRects)
{
    const AffineTransform flip = AffineTransform(-1, 0, 0, 1, 10, 0);
    EXPECT_EQ(flip.mapRect(RectF(1, 2, 3, 4)), RectF(6, 2, 3, 4));
    EXPECT_EQ(flip.mapRect(Rect(1, 2, 3, 4)), Rect(6, 2, 3, 4));
    const AffineTransform quarter = AffineTransform(0, 1, -1, 0, 100, 0);
    EXPECT_EQ(quarter.mapRect(RectF(0, 0, 10, 20)), RectF(80, 0, 20, 10));
    EXPECT_EQ(quarter.mapRect(Rect(0, 0, 10, 20)), Rect(80, 0, 20, 10));
    EXPECT_EQ(AffineTransform::fromScale(1.5, 1.5).mapRect(Rect(1, 1, 3, 3)), Rect(2, 2, 5, 5));
    EXPECT_EQ(AffineTransform::fromTranslate(0.5, -0.5).mapRect(Rect(0, 0, 2, 2)), Rect(1, -1, 2, 2));
    EXPECT_EQ(AffineTransform::fromScale(2, 2).map(Point(1, 2)), Point(2, 4));

    const Region region({Rect(0, 0, 2, 2), Rect(4, 0, 2, 2)});
    EXPECT_EQ(AffineTransform::fromScale(1.5, 1.5).mapToEnclosingRegion(region),
              Region({Rect(0, 0, 3, 3), Rect(6, 0, 3, 3)}));
    EXPECT_EQ(quarter.mapToEnclosingRegion(region), Region({Rect(98, 0, 2, 2), Rect(98, 4, 2, 2)}));
}
