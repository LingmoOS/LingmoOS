// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <cstdio>
#include <ctime>
#include <functional>
#include <string>

#include "ui/label.h"
#include "ui/layout.h"
#include "ui/painter.h"
#include "ui/switch.h"
#include "ui/theme.h"
#include "ui/window.h"

using namespace Lingmo::UI;

namespace {

// A settings card: a continuous-corner panel holding rows.
class Card : public Column
{
protected:
    void paint(Painter& painter) override
    {
        const Theme& theme = Theme::current();
        painter.drawRoundedRect(localRect(), {theme.bigRadius, 0, Corners::Continuous}, theme.secondBackground,
                                1 / painter.scale(), theme.text.withAlpha(0.08));
    }
};

// This thread's CPU time, so the tests running beside this one do not count.
double threadMilliseconds()
{
    timespec time;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &time);
    return double(time.tv_sec) * 1000 + double(time.tv_nsec) / 1e6;
}

// Milliseconds a whole-window repaint takes, on average over the frames.
double frameCost(Window& window, int frames, const std::function<void(int)>& step)
{
    window.grab();
    const double start = threadMilliseconds();
    for (int i = 0; i < frames; ++i) {
        step(i);
        window.contentView()->requestPaint();
        window.grab();
    }
    return (threadMilliseconds() - start) / frames;
}

} // namespace

// A page of cards repainted every frame, still and while it fades, as a
// parent window behind an opening dialog is: a frame must stay well inside
// 16 ms, whatever the corners cost.
TEST(FrameCostTest, PageOfCardsRepaintsWithinAFrame)
{
    Window window;
    window.setScale(1.5);
    auto page = std::make_unique<Column>();
    page->setSpacing(12);
    page->setMargins({24, 24, 24, 24});
    for (int c = 0; c < 8; ++c) {
        Card* card = page->addChild<Card>();
        card->setLayoutHints({.fillWidth = true});
        card->setMargins({16, 8, 16, 8});
        for (int r = 0; r < 4; ++r) {
            Row* row = card->addChild<Row>();
            row->setLayoutHints({.fillWidth = true});
            Label* label = row->addChild<Label>("Setting " + std::to_string(c * 4 + r));
            label->setLayoutHints({.fillWidth = true});
            row->addChild<Switch>();
        }
    }
    View* root = page.get();
    window.setContentView(std::move(page));
    window.resize({900, 820});
    window.show();

    const double still = frameCost(window, 30, [](int) {});
    const double fading = frameCost(window, 30, [root](int i) { root->setOpacity(1 - i / 60.0); });
    printf("page of 8 cards at 1.5: %.2f ms still, %.2f ms fading\n", still, fading);
    EXPECT_LT(still, 12);
    EXPECT_LT(fading, 12);
}
