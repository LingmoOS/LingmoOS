// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <tuple>

#include "ui/bitmap.h"
#include "ui/fast_blur_effect.h"

using namespace Lingmo::UI;

// The references are the FastBlur effect's own output on a 106 x 40 item, its
// source black text, a bar and a dot, rendered on the GPU at each scale and
// kept as the result's alpha (the source is black, so that is all of it).
static std::string dataPath(const std::string& name)
{
    return std::string(LINGMO_TOOLKIT_TEST_DATA_DIR) + "/fast_blur/" + name;
}

static std::string scaleName(double scale)
{
    char text[16];
    std::snprintf(text, sizeof(text), "%.1f", scale);
    return text;
}

TEST(FastBlurTest, WeightsFollowTheRadius)
{
    const auto weights = fastBlurWeights(6);
    EXPECT_NEAR(weights[0], 0.447, 0.001);
    EXPECT_NEAR(weights[1], 0.380, 0.001);
    EXPECT_NEAR(weights[2], 0.173, 0.001);
    EXPECT_EQ(weights[3], 0);
    // No radius: the source alone.
    EXPECT_EQ(fastBlurWeights(0)[0], 1);
}

class FastBlurReferenceTest : public testing::TestWithParam<std::tuple<double, int>>
{
};

TEST_P(FastBlurReferenceTest, MatchesTheEffect)
{
    const auto [scale, radius] = GetParam();
    const Bitmap source = Bitmap::decodeFile(dataPath("source@" + scaleName(scale) + ".png"));
    const Bitmap reference =
            Bitmap::decodeFile(dataPath("radius" + std::to_string(radius) + "@" + scaleName(scale) + ".png"));
    ASSERT_FALSE(source.isNull());
    ASSERT_FALSE(reference.isNull());

    // Black at the grey's coverage.
    Bitmap texture(source.width(), source.height(), Bitmap::Format::Argb32Premultiplied);
    for (int y = 0; y < source.height(); ++y) {
        for (int x = 0; x < source.width(); ++x)
            texture.setPixel(x, y, (source.pixel(x, y) & 0xff) << 24);
    }

    const Bitmap result = fastBlur(texture, 106, 40, radius, scale);
    ASSERT_EQ(result.width(), reference.width());
    ASSERT_EQ(result.height(), reference.height());
    // The GPU filters with a coarser fraction than the CPU: a level now and
    // then rounds to the next value, never further.
    int differing = 0;
    int largest = 0;
    for (int y = 0; y < result.height(); ++y) {
        for (int x = 0; x < result.width(); ++x) {
            const int ours = result.alpha(x, y);
            const int theirs = int(reference.pixel(x, y) & 0xff);
            const int difference = std::abs(ours - theirs);
            largest = std::max(largest, difference);
            differing += difference > 0;
        }
    }
    EXPECT_LE(largest, 1);
    EXPECT_LT(differing, result.width() * result.height() / 12);
}

INSTANTIATE_TEST_SUITE_P(Scales, FastBlurReferenceTest,
                         testing::Combine(testing::Values(1.0, 1.5, 2.0), testing::Values(6, 20, 48)));
