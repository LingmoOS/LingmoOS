// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdio>
#include <ctime>
#include <functional>
#include <string>
#include <vector>

#include "ui/image.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/test_input.h"
#include "ui/text_area.h"
#include "ui/text_field.h"
#include "ui/text_layout.h"
#include "ui/painter.h"
#include "ui/window.h"

using namespace Lingmo::UI;

namespace {

// This thread's CPU time, so the tests running beside this one do not count.
double threadMilliseconds()
{
    timespec time;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &time);
    return double(time.tv_sec) * 1000 + double(time.tv_nsec) / 1e6;
}

// The median of the step's cost over the runs, in milliseconds.
double medianCost(int runs, const std::function<void(int)>& step)
{
    std::vector<double> costs;
    for (int i = 0; i < runs; ++i) {
        const double start = threadMilliseconds();
        step(i);
        costs.push_back(threadMilliseconds() - start);
    }
    std::sort(costs.begin(), costs.end());
    return costs[costs.size() / 2];
}

// FNV-1a over the pixels, to compare frames between builds.
uint64_t hashOf(const Image& image)
{
    uint64_t hash = 1469598103934665603ull;
    for (int i = 0; i < image.width() * image.height(); ++i) {
        hash ^= image.bits()[i];
        hash *= 1099511628211ull;
    }
    return hash;
}

const char* kDescription = "Turn this on to let the system adjust the setting automatically when the "
                           "conditions around it change during the day.";

// A settings page: cards of rows, each a title, a wrapped description and an
// elided value.
struct SettingsPage
{
    Window window;
    std::vector<Label*> values;

    explicit SettingsPage(double scale)
    {
        auto page = std::make_unique<Column>();
        page->setSpacing(12);
        page->setMargins({24, 24, 24, 24});
        for (int c = 0; c < 6; ++c) {
            Column* card = page->addChild<Column>();
            card->setLayoutHints({.fillWidth = true});
            card->setSpacing(6);
            for (int r = 0; r < 4; ++r) {
                Row* row = card->addChild<Row>();
                row->setLayoutHints({.fillWidth = true});
                Column* texts = row->addChild<Column>();
                texts->setLayoutHints({.fillWidth = true});
                texts->addChild<Label>("Setting number " + std::to_string(c * 4 + r));
                Label* description = texts->addChild<Label>(kDescription);
                description->setWrap(true);
                description->setFontSize(11);
                description->setLayoutHints({.fillWidth = true});
                Label* value = row->addChild<Label>("A value that may be long, " + std::to_string(c * 4 + r));
                value->setLayoutHints({.maximumWidth = 160});
                values.push_back(value);
            }
        }
        window.setContentView(std::move(page));
        window.resize({900, 1100});
        window.setScale(scale);
        window.show();
        window.grab();
    }
};

std::string longText(int lines)
{
    std::string text;
    for (int i = 0; i < lines; ++i) {
        text += "Line " + std::to_string(i) + ": the quick brown fox jumps over the lazy dog, ";
        if (i % 3 == 0)
            text += "and a longer line that wraps at the width of the area on most windows ";
        text += "\n";
    }
    return text;
}

} // namespace

TEST(TextCostTest, SettingsPage)
{
    for (const double scale : {1.0, 1.5}) {
        SettingsPage page(scale);
        Image last;
        const double still = medianCost(30, [&](int) {
            page.window.contentView()->requestPaint();
            last = page.window.grab();
        });
        const uint64_t stillHash = hashOf(last);
        const double values = medianCost(30, [&](int i) {
            for (size_t v = 0; v < page.values.size(); ++v)
                page.values[v]->setText("Value " + std::to_string(i * 7 + int(v)));
            last = page.window.grab();
        });
        const uint64_t valuesHash = hashOf(last);
        const double resize = medianCost(30, [&](int i) {
            page.window.resize({900.0 - 10 * (i % 30), 1100});
            last = page.window.grab();
        });
        printf("settings page at %.1f: still %.2f ms, values %.2f ms, resize %.2f ms; hashes %016llx %016llx %016llx\n",
               scale, still, values, resize, (unsigned long long)stillHash, (unsigned long long)valuesHash,
               (unsigned long long)hashOf(last));
    }
}

TEST(TextCostTest, LongTextArea)
{
    Window window;
    auto root = std::make_unique<Column>();
    TextArea* area = root->addChild<TextArea>(longText(5000));
    area->setLayoutHints({.preferredWidth = 700, .preferredHeight = 560});
    window.setContentView(std::move(root));
    window.resize({720, 580});
    window.setScale(1.5);
    window.show();
    double start = threadMilliseconds();
    window.grab();
    const double first = threadMilliseconds() - start;
    TestInput::setKeyboardFocus(window, true);
    area->setFocus();
    area->setCursorPosition(area->text().size() / 2);
    window.grab();

    Image last;
    const double repaint = medianCost(10, [&](int) {
        area->requestPaint();
        last = window.grab();
    });
    const uint64_t repaintHash = hashOf(last);
    const double typing = medianCost(20, [&](int i) {
        TestInput::typeText(window, std::string(1, char('a' + i % 26)));
        last = window.grab();
    });
    const uint64_t typingHash = hashOf(last);
    const double moving = medianCost(20, [&](int i) {
        TestInput::typeKey(window, i % 2 ? key::Up : key::Down);
        last = window.grab();
    });
    area->select(area->text().size() / 2 - 3000, area->text().size() / 2 + 200);
    const double selected = medianCost(10, [&](int) {
        area->requestPaint();
        last = window.grab();
    });
    printf("text area of 5000 lines: first %.1f ms, repaint %.2f ms, typing %.2f ms per key, moving %.2f ms per key, "
           "selected %.2f ms; hashes %016llx %016llx %016llx\n",
           first, repaint, typing, moving, selected, (unsigned long long)repaintHash, (unsigned long long)typingHash,
           (unsigned long long)hashOf(last));
}

TEST(TextCostTest, TextFieldTyping)
{
    Window window;
    auto root = std::make_unique<Column>();
    TextField* field = root->addChild<TextField>(std::string(60, 'x'));
    field->setLayoutHints({.preferredWidth = 300});
    window.setContentView(std::move(root));
    window.resize({320, 60});
    window.setScale(1.5);
    window.show();
    window.grab();
    TestInput::setKeyboardFocus(window, true);
    field->setFocus();
    Image last;
    const double typing = medianCost(100, [&](int i) {
        TestInput::typeText(window, std::string(1, char('a' + i % 26)));
        last = window.grab();
    });
    printf("text field: typing %.3f ms per key; hash %016llx\n", typing, (unsigned long long)hashOf(last));
}

namespace {

// Code lines as an editor draws them: one layout per line.
class CodeView : public View
{
public:
    std::vector<TextLayout> layouts;

protected:
    void paint(Painter& painter) override
    {
        double y = 0;
        for (TextLayout& layout : layouts) {
            if (y > geometry().height)
                break;
            layout.draw(painter, {0, y});
            y += 18;
        }
    }
};

} // namespace

TEST(TextCostTest, CodeLines)
{
    Window window;
    auto root = std::make_unique<Column>();
    CodeView* view = root->addChild<CodeView>();
    view->setLayoutHints({.fillWidth = true, .fillHeight = true});
    window.setContentView(std::move(root));
    window.resize({800, 600});
    window.setScale(1.5);
    window.show();
    window.grab();
    const FontSpec font = systemFixedFont();
    const double layout = medianCost(5, [&](int) {
        view->layouts.clear();
        for (int i = 0; i < 2000; ++i) {
            TextLayout& line = view->layouts.emplace_back(
                "    if (value_" + std::to_string(i) + " > limit) { return compute(value, \"text\", 0x" + std::to_string(i * 31) + "); }",
                font, 1.5);
            line.setFormats({{4, 2, Color::fromArgb(0xff0000ff), true}, {30, 7, Color::fromArgb(0xff008000)}});
            line.beginLayout();
            line.createLine(780, {0, 0});
            line.endLayout();
        }
    });
    Image last;
    const double repaint = medianCost(20, [&](int) {
        view->requestPaint();
        last = window.grab();
    });
    printf("code lines: 2000 laid out %.2f ms, repaint %.2f ms; hash %016llx\n", layout, repaint,
           (unsigned long long)hashOf(last));
}
