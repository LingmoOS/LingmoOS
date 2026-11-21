// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <cmath>
#include <set>
#include <string>

#include "ui/painter.h"
#include "ui/view.h"
#include "ui/window.h"

using namespace Lingmo::UI;

static constexpr Color kBackground = Color::rgb(0xF5F6FA);
static constexpr Color kLine = Color::rgb(0xE3E4E8);
static constexpr Color kFill = Color::rgb(0x0176D3);

// Square shapes at fractional logical positions, which must still land on
// whole device pixels.
namespace {

class Shapes : public View
{
protected:
    void paint(Painter& painter) override
    {
        painter.strokeRoundedRect({10.3, 20.6, 100.45, 50.2}, 0, kLine);
        painter.fillRect({130.7, 20.2, 60.3, 40.9}, kFill);
        painter.fillRect({190.7 + 0.3, 20.2, 33.3, 40.9}, kFill);
        painter.drawHorizontalLine(5.5, 90.5, 300.3, kLine);
        painter.drawVerticalLine(320.6, 5.2, geometry().height - 10.4, kLine);
        painter.strokeRoundedRect(localRect().adjusted(2.5, geometry().height - 40.7, -2.5, -2.5), 0,
                                  kLine);
    }
};

} // namespace

class PixelTest : public testing::TestWithParam<std::tuple<double, SizeF>>
{
};

TEST_P(PixelTest, EdgesLandOnDevicePixels)
{
    const auto [scale, size] = GetParam();
    Window window;
    window.setScale(scale);
    window.resize(size);
    window.setBackgroundColor(kBackground);
    window.setContentView(std::make_unique<Shapes>());
    window.show();

    const Image image = window.grab();
    ASSERT_EQ(image.width(), int(std::lround(size.width * scale)));
    ASSERT_EQ(image.height(), int(std::lround(size.height * scale)));

    std::set<uint32_t> colors;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x)
            colors.insert(image.pixel(x, y).argb());
    }
    EXPECT_EQ(colors, (std::set<uint32_t>{kBackground.argb(), kLine.argb(), kFill.argb()}));
}

INSTANTIATE_TEST_SUITE_P(Scales, PixelTest,
                         testing::Combine(testing::Values(1.0, 1.25, 1.5, 1.75, 2.0),
                                          testing::Values(SizeF{720, 480}, SizeF{721, 481},
                                                          SizeF{723, 483})),
                         [](const testing::TestParamInfo<PixelTest::ParamType>& info) {
                             const double scale = std::get<0>(info.param);
                             const SizeF size = std::get<1>(info.param);
                             return "scale" + std::to_string(int(scale * 100)) + "_"
                                     + std::to_string(int(size.width)) + "x"
                                     + std::to_string(int(size.height));
                         });
