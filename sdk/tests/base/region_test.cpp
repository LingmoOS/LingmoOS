// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <bitset>
#include <random>
#include <vector>

#include "base/region.h"

using namespace Lingmo::Base;

static constexpr int kGrid = 24;

using Bitmap = std::bitset<kGrid * kGrid>;

static Bitmap rasterize(const Region& region)
{
    Bitmap bitmap;
    for (const Rect& rect : region) {
        for (int y = rect.y(); y < rect.bottom(); ++y) {
            for (int x = rect.x(); x < rect.right(); ++x) {
                EXPECT_TRUE(x >= 0 && x < kGrid && y >= 0 && y < kGrid);
                EXPECT_FALSE(bitmap[size_t(y * kGrid + x)]) << "rectangles overlap";
                bitmap.set(size_t(y * kGrid + x));
            }
        }
    }
    return bitmap;
}

static Bitmap rasterize(const Rect& rect)
{
    Bitmap bitmap;
    for (int y = rect.y(); y < rect.bottom(); ++y) {
        for (int x = rect.x(); x < rect.right(); ++x)
            bitmap.set(size_t(y * kGrid + x));
    }
    return bitmap;
}

// The banding invariants that make the form canonical.
static void expectCanonical(const Region& region)
{
    const std::span<const Rect> rects = region.rects();
    Rect bounds;
    for (size_t i = 0; i < rects.size(); ++i) {
        const Rect& rect = rects[i];
        ASSERT_FALSE(rect.isEmpty());
        bounds = bounds.united(rect);
        if (i == 0)
            continue;
        const Rect& previous = rects[i - 1];
        if (previous.y() == rect.y()) {
            EXPECT_EQ(previous.bottom(), rect.bottom());
            EXPECT_LT(previous.right(), rect.x()) << "spans in a band touch";
        } else {
            EXPECT_LE(previous.bottom(), rect.y()) << "bands overlap";
        }
    }
    EXPECT_EQ(region.boundingRect(), bounds);

    // Adjacent bands never share the same spans.
    std::vector<std::pair<size_t, size_t>> bands;
    for (size_t i = 0; i < rects.size();) {
        size_t end = i + 1;
        while (end < rects.size() && rects[end].y() == rects[i].y())
            ++end;
        bands.emplace_back(i, end);
        i = end;
    }
    for (size_t b = 1; b < bands.size(); ++b) {
        const auto [ps, pe] = bands[b - 1];
        const auto [cs, ce] = bands[b];
        if (rects[ps].bottom() != rects[cs].y() || pe - ps != ce - cs)
            continue;
        bool same = true;
        for (size_t k = 0; k < pe - ps; ++k)
            same = same && rects[ps + k].x() == rects[cs + k].x() && rects[ps + k].right() == rects[cs + k].right();
        EXPECT_FALSE(same) << "uncoalesced bands in " << region;
    }
}

TEST(RegionTest, EmptyAndSingle)
{
    Region empty;
    EXPECT_TRUE(empty.isEmpty());
    EXPECT_EQ(empty.rectCount(), 0u);
    EXPECT_EQ(empty.boundingRect(), Rect());
    EXPECT_TRUE(Region(Rect(5, 5, 0, 3)).isEmpty());

    const Region one(Rect(1, 2, 3, 4));
    EXPECT_EQ(one.rectCount(), 1u);
    EXPECT_EQ(one.boundingRect(), Rect(1, 2, 3, 4));
    EXPECT_TRUE(one.contains(Point(1, 2)));
    EXPECT_FALSE(one.contains(Point(4, 2)));
    EXPECT_TRUE(one.contains(Rect(1, 2, 3, 4)));
    EXPECT_FALSE(one.contains(Rect(1, 2, 3, 5)));
    EXPECT_FALSE(one.contains(Rect()));
}

TEST(RegionTest, UnionCoalesces)
{
    Region region(Rect(0, 0, 10, 5));
    region += Rect(0, 5, 10, 5);
    EXPECT_EQ(region.rectCount(), 1u);
    EXPECT_EQ(region, Region(Rect(0, 0, 10, 10)));
    region |= Rect(10, 0, 5, 10);
    EXPECT_EQ(region, Region(Rect(0, 0, 15, 10)));
    expectCanonical(region);
}

TEST(RegionTest, SubtractMakesHole)
{
    const Region region = Region(Rect(0, 0, 10, 10)) - Rect(3, 3, 4, 4);
    EXPECT_EQ(region.rectCount(), 4u);
    EXPECT_FALSE(region.contains(Point(3, 3)));
    EXPECT_TRUE(region.contains(Point(2, 3)));
    EXPECT_TRUE(region.contains(Point(7, 6)));
    EXPECT_FALSE(region.contains(Rect(0, 0, 10, 10)));
    EXPECT_TRUE(region.contains(Rect(0, 0, 10, 3)));
    EXPECT_TRUE(region.intersects(Rect(2, 2, 2, 2)));
    EXPECT_FALSE(region.intersects(Rect(3, 3, 4, 4)));
    EXPECT_EQ(region.boundingRect(), Rect(0, 0, 10, 10));
    expectCanonical(region);
    EXPECT_EQ(region + Rect(3, 3, 4, 4), Region(Rect(0, 0, 10, 10)));
}

TEST(RegionTest, XorAndIntersect)
{
    const Region a(Rect(0, 0, 10, 10));
    const Region b(Rect(5, 0, 10, 10));
    EXPECT_EQ(a ^ b, Region({Rect(0, 0, 5, 10), Rect(10, 0, 5, 10)}));
    EXPECT_EQ(a & b, Region(Rect(5, 0, 5, 10)));
    EXPECT_TRUE((a & Region(Rect(20, 20, 5, 5))).isEmpty());
    EXPECT_TRUE(a.intersects(b));
    EXPECT_FALSE(a.intersects(Region(Rect(10, 0, 5, 5))));
}

TEST(RegionTest, TranslateAndIterate)
{
    Region region({Rect(0, 0, 2, 2), Rect(4, 0, 2, 2)});
    region.translate(10, 20);
    std::vector<Rect> rects(region.begin(), region.end());
    EXPECT_EQ(rects, (std::vector<Rect>{Rect(10, 20, 2, 2), Rect(14, 20, 2, 2)}));
    EXPECT_EQ(region.boundingRect(), Rect(10, 20, 6, 2));
    EXPECT_EQ(region.translated(Point(-10, -20)).boundingRect(), Rect(0, 0, 6, 2));
    region.clear();
    EXPECT_TRUE(region.isEmpty());
}

TEST(RegionTest, ExtremeCoordinates)
{
    const Rect all(-0x3FFFFFFF, -0x3FFFFFFF, 0x7FFFFFFE, 0x7FFFFFFE);
    Region region(all);
    region -= Rect(0, 0, 10, 10);
    EXPECT_EQ(region.rectCount(), 4u);
    EXPECT_EQ(region.boundingRect(), all);
    region += Rect(0, 0, 10, 10);
    EXPECT_EQ(region, Region(all));
}

TEST(RegionTest, EqualityIsStructural)
{
    // The same pixels built in different orders.
    const Region a = Region(Rect(0, 0, 4, 4)) + Rect(4, 0, 4, 2) + Rect(0, 4, 2, 2);
    const Region b = Region(Rect(0, 4, 2, 2)) + Rect(0, 0, 8, 2) + Rect(0, 2, 4, 2);
    EXPECT_EQ(a, b);
    EXPECT_EQ(a.toString(), b.toString());
}

static Rect randomRect(std::mt19937& random)
{
    std::uniform_int_distribution<int> coordinate(0, kGrid - 1);
    const int x1 = coordinate(random);
    const int y1 = coordinate(random);
    const int x2 = coordinate(random);
    const int y2 = coordinate(random);
    return Rect::fromEdges(std::min(x1, x2), std::min(y1, y2), std::max(x1, x2) + 1, std::max(y1, y2) + 1);
}

static Region randomRegion(std::mt19937& random, Bitmap& bitmap)
{
    std::uniform_int_distribution<int> count(0, 6);
    std::uniform_int_distribution<int> operation(0, 3);
    Region region;
    bitmap.reset();
    const int n = count(random);
    for (int i = 0; i < n; ++i) {
        const Rect rect = randomRect(random);
        const Bitmap pixels = rasterize(rect);
        switch (operation(random)) {
        case 0:
        case 1:
            region += rect;
            bitmap |= pixels;
            break;
        case 2:
            region -= rect;
            bitmap &= ~pixels;
            break;
        case 3:
            region ^= rect;
            bitmap ^= pixels;
            break;
        }
    }
    return region;
}

TEST(RegionTest, RandomAgainstBitmap)
{
    std::mt19937 random(20261015);
    for (int iteration = 0; iteration < 4000; ++iteration) {
        Bitmap bitsA;
        Bitmap bitsB;
        const Region a = randomRegion(random, bitsA);
        const Region b = randomRegion(random, bitsB);
        ASSERT_EQ(rasterize(a), bitsA) << a;
        ASSERT_EQ(rasterize(b), bitsB) << b;
        expectCanonical(a);

        const Region united = a | b;
        const Region intersected = a & b;
        const Region subtracted = a - b;
        const Region xored = a ^ b;
        ASSERT_EQ(rasterize(united), bitsA | bitsB) << a << " | " << b;
        ASSERT_EQ(rasterize(intersected), bitsA & bitsB) << a << " & " << b;
        ASSERT_EQ(rasterize(subtracted), bitsA & ~bitsB) << a << " - " << b;
        ASSERT_EQ(rasterize(xored), bitsA ^ bitsB) << a << " ^ " << b;
        expectCanonical(united);
        expectCanonical(intersected);
        expectCanonical(subtracted);
        expectCanonical(xored);

        EXPECT_EQ(a.intersects(b), (bitsA & bitsB).any());
        EXPECT_EQ(a == b, bitsA == bitsB);

        const Rect rect = randomRect(random);
        const Bitmap rectBits = rasterize(rect);
        EXPECT_EQ(a.contains(rect), (rectBits & ~bitsA).none());
        EXPECT_EQ(a.intersects(rect), (rectBits & bitsA).any());
        const Point point(rect.x(), rect.y());
        EXPECT_EQ(a.contains(point), bitsA[size_t(point.y() * kGrid + point.x())]);

        // Building from the bitmap's own rows yields the same canonical form.
        std::vector<Rect> pixels;
        for (int y = 0; y < kGrid; ++y) {
            for (int x = 0; x < kGrid; ++x) {
                if (bitsA[size_t(y * kGrid + x)])
                    pixels.emplace_back(x, y, 1, 1);
            }
        }
        ASSERT_EQ(Region::fromRects(pixels), a);
    }
}
