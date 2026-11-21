// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <cstdio>
#include <cstring>
#include <ctime>
#include <functional>
#include <memory>
#include <string>

#include "ui/header_backdrop.h"
#include "ui/image.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/painter.h"
#include "ui/scroll_view.h"
#include "ui/switch.h"
#include "ui/test_input.h"
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

// A filled rect whose colour changes on request.
class Swatch : public View
{
public:
    void setColor(Color color)
    {
        m_color = color;
        requestPaint();
    }

    SizeF sizeHint() const override { return {0, 40}; }

protected:
    void paint(Painter& painter) override { painter.fillRect(localRect(), m_color); }

private:
    Color m_color = Color::rgb(0x3080F0);
};

// A settings page under a frosted header with a button over the bar.
struct Page
{
    Window window;
    ScrollView* view = nullptr;
    Swatch* swatch = nullptr;
    View* button = nullptr;

    explicit Page(bool backdrop = true)
    {
        window.setScale(1.5);
        auto root = std::make_unique<View>();
        auto scroll = std::make_unique<ScrollView>();
        view = scroll.get();
        Column* page = view->setContent<Column>();
        page->setSpacing(12);
        page->setMargins({24, 64, 24, 24});
        for (int c = 0; c < 10; ++c) {
            if (c == 1) {
                swatch = page->addChild<Swatch>();
                swatch->setLayoutHints({.fillWidth = true});
            }
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
        root->addChild(std::move(scroll));
        HeaderBackdrop* bar = backdrop ? root->addChild<HeaderBackdrop>(view) : nullptr;
        button = root->addChild<View>();
        window.setContentView(std::move(root));
        window.resize({900, 700});
        window.show();
        window.grab();
        view->setGeometry({0, 0, 900, 700});
        if (bar)
            bar->setGeometry({0, 0, 900, 52});
        button->setGeometry({840, 10, 32, 32});
        window.grab();
    }

    // Where the swatch lies under the bar.
    double underTheBar() const { return swatch->geometry().y - 10; }
};

bool same(const Image& a, const Image& b)
{
    return a.width() == b.width() && a.height() == b.height()
            && std::memcmp(a.bits(), b.bits(), size_t(a.width()) * size_t(a.height()) * 4) == 0;
}

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

// Repaints that leave the content under the bar as it was draw the same
// blur; a change there, or a scroll, shows at once.
TEST(HeaderBackdropTest, BlursWhatTheContentShowsNow)
{
    Page page;
    const double top = page.underTheBar();
    page.view->setScrollPosition({0, top});
    const Image scrolled = page.window.grab();

    page.button->requestPaint();
    EXPECT_TRUE(same(page.window.grab(), scrolled));

    page.swatch->setColor(Color::rgb(0xF04020));
    const Image changed = page.window.grab();
    EXPECT_FALSE(same(changed, scrolled));
    Page fresh;
    fresh.swatch->setColor(Color::rgb(0xF04020));
    fresh.view->setScrollPosition({0, top});
    EXPECT_TRUE(same(changed, fresh.window.grab()));

    page.view->setScrollPosition({0, top + 1});
    fresh.view->setScrollPosition({0, top + 1});
    EXPECT_TRUE(same(page.window.grab(), fresh.window.grab()));
}

// A theme change repaints what the content shows under the bar.
TEST(HeaderBackdropTest, FollowsTheTheme)
{
    Page page;
    page.view->setScrollPosition({0, page.underTheBar()});
    page.window.grab();
    Theme::setCurrent(Theme::dark());
    const Image dark = page.window.grab();
    Page fresh;
    fresh.view->setScrollPosition({0, fresh.underTheBar()});
    const Image expected = fresh.window.grab();
    Theme::setCurrent(Theme::light());
    EXPECT_TRUE(same(dark, expected));
}

// What the bar adds to a whole-window repaint, at 1.5.
TEST(HeaderBackdropTest, FrameCost)
{
    Page plain(false);
    plain.view->setScrollPosition({0, plain.underTheBar()});
    Page page;
    const double top = page.underTheBar();
    page.view->setScrollPosition({0, top});

    const double without = frameCost(plain.window, 30, [](int) {});
    const double still = frameCost(page.window, 30, [](int) {});
    const double changing = frameCost(page.window, 30, [&page](int i) {
        page.swatch->setColor(Color::rgb(i % 2 ? 0xF04020 : 0x3080F0));
    });
    const double scrolling = frameCost(page.window, 30, [&page, top](int i) {
        page.view->setScrollPosition({0, top + i});
    });
    printf("header backdrop at 1.5: page %.2f ms, with the bar %.2f ms still, %.2f ms content changing, "
           "%.2f ms scrolling\n",
           without, still, changing, scrolling);
}

// A bar taken out of its window and then destroyed, as a page switch drops
// it, stops watching the window's damage.
TEST(HeaderBackdropTest, ForgetsItsWindowWhenTakenOut)
{
    Page page;
    page.view->setScrollPosition({0, page.underTheBar()});
    page.window.grab();
    View* root = page.window.contentView();
    for (int i = 0; i < 5; ++i) {
        HeaderBackdrop* bar = root->addChild<HeaderBackdrop>(page.view);
        bar->setGeometry({0, 0, 900, 52});
        page.window.grab();
        root->takeChild(bar).reset();
    }
    EXPECT_EQ(TestInput::damageWatches(page.window), 1u);
}
