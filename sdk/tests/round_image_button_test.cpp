// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <climits>
#include <cmath>

#include "ui/round_image_button.h"
#include "ui/test_input.h"
#include "ui/theme.h"
#include "ui/window.h"

using namespace Lingmo::UI;

namespace {

// Puts its one child at a fixed place.
class Place : public View
{
public:
    explicit Place(RectF rect)
        : m_rect(rect)
    {
    }

protected:
    void layout() override { children().front()->setGeometry(m_rect); }

private:
    RectF m_rect;
};

struct Fixture
{
    explicit Fixture(RectF rect, double scale = 1, std::string glyph = {})
    {
        auto place = std::make_unique<Place>(rect);
        button = place->addChild<RoundImageButton>(std::move(glyph), rect.width);
        window.resize({120, 80});
        window.setScale(scale);
        window.setBackgroundColor(Color::rgb(0xFFFFFF));
        window.setContentView(std::move(place));
        window.show();
        window.grab();
    }

    // The device pixels that differ from the white background.
    Rect inked()
    {
        const Image image = window.grab();
        int left = INT_MAX;
        int top = INT_MAX;
        int right = INT_MIN;
        int bottom = INT_MIN;
        for (int y = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x) {
                if (image.pixel(x, y) == Color::rgb(0xFFFFFF))
                    continue;
                left = std::min(left, x);
                top = std::min(top, y);
                right = std::max(right, x + 1);
                bottom = std::max(bottom, y + 1);
            }
        }
        return left > right ? Rect{} : Rect{left, top, right - left, bottom - top};
    }

    Window window;
    RoundImageButton* button;
};

} // namespace

TEST(RoundImageButtonTest, ClicksOnReleaseInside)
{
    Fixture f({10, 10, 24, 24});
    int clicks = 0;
    f.button->onClicked = [&clicks] { ++clicks; };
    TestInput::click(f.window, {22, 22});
    EXPECT_EQ(clicks, 1);
    TestInput::press(f.window, {22, 22});
    EXPECT_TRUE(f.button->isPressed());
    TestInput::movePointer(f.window, {80, 60});
    TestInput::release(f.window, {80, 60});
    EXPECT_EQ(clicks, 1);
    EXPECT_FALSE(f.button->isPressed());
    f.button->setEnabled(false);
    TestInput::click(f.window, {22, 22});
    EXPECT_EQ(clicks, 1);
}

TEST(RoundImageButtonTest, IconSizeFollowsTheButtonAndMargins)
{
    RoundImageButton button("chevron-left", 24);
    EXPECT_EQ(button.iconSize(), 24);
    EXPECT_EQ(button.sizeHint(), (SizeF{24, 24}));
    button.setIconMargins(4.3);
    EXPECT_EQ(button.iconSize(), 15);
    button.setIconSize(16);
    EXPECT_EQ(button.iconSize(), 16);
}

// The plate shows hovered, pressed, checked or highlighted, never disabled,
// in the text colour at FishUI's alphas over the white window.
TEST(RoundImageButtonTest, PlateFollowsTheState)
{
    Fixture f({10, 10, 24, 24});
    f.button->setIconSize(16);
    const Theme& theme = Theme::current();
    const auto plateColour = [&f] {
        const Image image = f.window.grab();
        return image.pixel(22, 22);
    };
    const auto over = [&theme](double alpha) {
        const auto mix = [alpha](int c) { return int(std::lround(c * alpha + 255 * (1 - alpha))); };
        return Color{uint8_t(mix(theme.text.r)), uint8_t(mix(theme.text.g)), uint8_t(mix(theme.text.b)), 255};
    };
    const auto near = [](Color a, Color b) {
        return std::abs(a.r - b.r) <= 1 && std::abs(a.g - b.g) <= 1 && std::abs(a.b - b.b) <= 1;
    };
    ASSERT_FALSE(theme.darkMode);
    EXPECT_EQ(plateColour(), Color::rgb(0xFFFFFF));
    TestInput::movePointer(f.window, {22, 22});
    EXPECT_TRUE(near(plateColour(), over(0.11)));
    TestInput::press(f.window, {22, 22});
    EXPECT_TRUE(near(plateColour(), over(0.21)));
    TestInput::release(f.window, {22, 22});
    TestInput::leavePointer(f.window);
    EXPECT_EQ(plateColour(), Color::rgb(0xFFFFFF));
    f.button->setHighlighted(true);
    EXPECT_TRUE(near(plateColour(), over(0.11)));
    f.button->setChecked(true);
    EXPECT_TRUE(near(plateColour(), over(0.21)));
    f.button->setEnabled(false);
    EXPECT_EQ(plateColour(), Color::rgb(0xFFFFFF));
}

// The plate is inset a tenth of the button and keeps the icon's device
// pixel parity, so both share a centre: at 125% a 24 button with a 16 icon
// (20 px) gets 24 px, not 25.
TEST(RoundImageButtonTest, PlateKeepsTheIconsParity)
{
    for (double scale : {1.0, 1.25, 1.5, 1.75, 2.0}) {
        Fixture f({10.5, 11, 24, 24}, scale);
        f.button->setIconSize(16);
        f.button->setHighlighted(true);
        const Rect plate = f.inked();
        const int icon = int(std::lround(16 * scale));
        int expected = int(std::lround(20 * scale));
        if ((expected - icon) % 2 != 0)
            expected -= 1;
        EXPECT_EQ(plate.width, expected) << scale;
        EXPECT_EQ(plate.height, expected) << scale;
        // Centred on the icon's square, snapped as an icon snaps.
        const double iconLeft = std::ceil((10.5 + 4) * scale - 1e-9);
        const double iconTop = std::ceil((11 + 4) * scale - 1e-9);
        EXPECT_EQ(plate.x * 2 + plate.width, iconLeft * 2 + icon) << scale;
        EXPECT_EQ(plate.y * 2 + plate.height, iconTop * 2 + icon) << scale;
    }
}

// A glyph keeps its colour when disabled and fades to 0.35, as FishUI.Icon
// under the button's opacity.
TEST(RoundImageButtonTest, DisabledGlyphFades)
{
    Fixture f({10, 10, 24, 24}, 1, "chevron-left");
    f.button->setIconSize(16);
    f.button->setIconColor(Color::rgb(0x000000));
    const auto darkest = [&f] {
        const Image image = f.window.grab();
        int value = 255;
        for (int y = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x)
                value = std::min(value, int(image.pixel(x, y).r));
        }
        return value;
    };
    EXPECT_LT(darkest(), 40);
    f.button->setEnabled(false);
    const int faded = darkest();
    EXPECT_NEAR(faded, 255 * 0.65, 25);
}
