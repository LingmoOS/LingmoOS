// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <cstdlib>

#include "ui/geometry.h"

using namespace Lingmo::UI;

static void expectNear(Color actual, Color expected)
{
    EXPECT_LE(std::abs(actual.r - expected.r), 1) << int(actual.r) << " vs " << int(expected.r);
    EXPECT_LE(std::abs(actual.g - expected.g), 1) << int(actual.g) << " vs " << int(expected.g);
    EXPECT_LE(std::abs(actual.b - expected.b), 1) << int(actual.b) << " vs " << int(expected.b);
    EXPECT_EQ(actual.a, expected.a);
}

TEST(ColorTest, DarkerDividesTheValue)
{
    expectNear(Color::rgb(0xF2F4F5).darker(1.1), Color::rgb(0xDCDEDF));
    expectNear(Color::rgb(0xF2F4F5).darker(1.2), Color::rgb(0xCACBCC));
    expectNear(Color::rgb(0x3385FF).darker(2), Color::rgb(0x1A4380));
}

TEST(ColorTest, LighterMultipliesTheValueThenWhitens)
{
    expectNear(Color::rgb(0x3C3C3D).lighter(1.2), Color::rgb(0x484849));
    // Past full value the saturation goes instead.
    const Color light = Color::rgb(0x3385FF).lighter(1.5);
    EXPECT_EQ(light.b, 255);
    EXPECT_GT(light.r, 0x33);
}

TEST(ColorTest, KeepsAlpha)
{
    EXPECT_EQ(Color::rgb(0x808080).withAlpha(0.5).darker(1.5).a, 128);
}
