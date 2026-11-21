// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "test_views.h"
#include "ui/cell_layout.h"
#include "ui/window.h"

using namespace Lingmo::UI;
using Lingmo::UI::test::Box;

// Reference geometry of row, column and grid layouts for the same children
// (rectangles with these implicit sizes and layout hints), measured on the
// test laptop.
namespace {

template<typename Layout>
struct Hosted
{
    Window window;
    Layout* layout = nullptr;

    explicit Hosted(SizeF size)
    {
        auto root = std::make_unique<View>();
        layout = root->addChild<Layout>();
        layout->setGeometry({0, 0, size.width, size.height});
        window.setContentView(std::move(root));
        window.resize({400, 400});
        window.show();
    }

    Box* add(SizeF implicit, LayoutHints hints = {})
    {
        Box* box = layout->template addChild<Box>(implicit);
        box->setLayoutHints(hints);
        return box;
    }

    void run() { window.grab(); }
};

Box* addBox(View* parent, SizeF implicit, LayoutHints hints = {})
{
    Box* box = parent->addChild<Box>(implicit);
    box->setLayoutHints(hints);
    return box;
}

void expectGeometry(const View* view, RectF expected)
{
    const RectF& geometry = view->geometry();
    EXPECT_DOUBLE_EQ(geometry.x, expected.x);
    EXPECT_DOUBLE_EQ(geometry.y, expected.y);
    EXPECT_DOUBLE_EQ(geometry.width, expected.width);
    EXPECT_DOUBLE_EQ(geometry.height, expected.height);
}

} // namespace

TEST(CellLayoutTest, RowSharesBySizeAndRounds)
{
    Hosted<RowLayout> hosted({300, 44});
    Box* a = hosted.add({50.4, 17.2});
    Box* b = hosted.add({30.6, 21.5}, {.fillWidth = true});
    Box* c = hosted.add({12, 10}, {.fillWidth = true});
    hosted.run();
    expectGeometry(a, {0, 13, 51, 18});
    expectGeometry(b, {56, 11, 172, 22});
    expectGeometry(c, {233, 17, 67, 10});
    EXPECT_EQ(hosted.layout->sizeHint(), (SizeF{104, 22}));
}

TEST(CellLayoutTest, RowWithoutFillingChildrenSpreadsCells)
{
    Hosted<RowLayout> hosted({300, 30});
    Box* a = hosted.add({50.4, 17});
    Box* b = hosted.add({80, 17});
    hosted.run();
    expectGeometry(a, {0, 7, 51, 17});
    expectGeometry(b, {120, 7, 80, 17});
}

TEST(CellLayoutTest, RowNarrowerThanPreferred)
{
    Hosted<RowLayout> hosted({100, 30});
    Box* a = hosted.add({80, 17}, {.fillWidth = true, .minimumWidth = 20});
    Box* b = hosted.add({60, 17}, {.fillWidth = true});
    hosted.run();
    expectGeometry(a, {0, 7, 58, 17});
    expectGeometry(b, {63, 7, 37, 17});
}

TEST(CellLayoutTest, ColumnAlignsAndKeepsMargins)
{
    Hosted<ColumnLayout> hosted({200, 150});
    hosted.layout->setSpacing(7);
    Box* a = hosted.add({50.5, 20.3}, {.horizontalAlignment = Alignment::End});
    LayoutHints withMargins{.fillWidth = true};
    withMargins.margins = {4, 3, 0, 0};
    Box* b = hosted.add({40, 15.5}, withMargins);
    Box* c = hosted.add({30, 10}, {.fillHeight = true, .maximumHeight = 40, .horizontalAlignment = Alignment::Center});
    Box* d = hosted.add({30, 10}, {.fillHeight = true});
    hosted.run();
    expectGeometry(a, {149, 0, 51, 21});
    expectGeometry(b, {4, 31, 196, 16});
    expectGeometry(c, {85, 54, 30, 40});
    expectGeometry(d, {0, 101, 30, 49});
    EXPECT_EQ(hosted.layout->sizeHint(), (SizeF{51, 81}));
}

TEST(CellLayoutTest, GridSharesRowsAndColumns)
{
    Hosted<GridLayout> hosted({250, 100});
    hosted.layout->setColumns(2);
    hosted.layout->setRowSpacing(4);
    hosted.layout->setColumnSpacing(12);
    Box* a = hosted.add({60.2, 17});
    Box* b = hosted.add({100, 32}, {.fillWidth = true});
    Box* c = hosted.add({90.5, 17});
    Box* d = hosted.add({40, 20}, {.verticalAlignment = Alignment::End});
    hosted.run();
    expectGeometry(a, {0, 21, 61, 17});
    expectGeometry(b, {103, 14, 147, 32});
    expectGeometry(c, {0, 73, 91, 17});
    expectGeometry(d, {103, 80, 40, 20});
    EXPECT_EQ(hosted.layout->sizeHint(), (SizeF{203, 56}));
}

// A row of labels over a graph, as system-monitor's chart card.
TEST(CellLayoutTest, NestedRowsOfFixedChildrenDoNotGrow)
{
    Hosted<ColumnLayout> hosted({544, 244});
    hosted.layout->setSpacing(10);
    auto* top = hosted.layout->addChild<RowLayout>();
    Box* a = addBox(top, {120.3, 32.4}, {.fillWidth = true});
    Box* b = addBox(top, {60, 20.5});
    Box* c = hosted.add({80, 16.4});
    Box* d = hosted.add({40, 15}, {.horizontalAlignment = Alignment::End});
    Box* graph = hosted.add({0, 0}, {.fillWidth = true, .fillHeight = true});
    auto* bottom = hosted.layout->addChild<RowLayout>();
    Box* e = addBox(bottom, {30.2, 14.6});
    addBox(bottom, {20, 15});
    addBox(bottom, {25, 14.6});
    addBox(bottom, {18, 15});
    Box* i = addBox(bottom, {40.5, 14.6});
    Box* spacer = addBox(bottom, {0, 0}, {.fillWidth = true});
    hosted.run();
    expectGeometry(top, {0, 0, 544, 33});
    expectGeometry(a, {0, 0, 479, 33});
    expectGeometry(b, {484, 6, 60, 21});
    expectGeometry(c, {0, 43, 80, 17});
    expectGeometry(d, {504, 70, 40, 15});
    expectGeometry(graph, {0, 95, 544, 124});
    expectGeometry(bottom, {0, 229, 544, 15});
    expectGeometry(e, {0, 0, 31, 15});
    expectGeometry(i, {114, 0, 41, 15});
    expectGeometry(spacer, {160, 8, 384, 0});
    EXPECT_EQ(hosted.layout->sizeHint(), (SizeF{186, 120}));
}

TEST(CellLayoutTest, NestedColumnsWidenAsFarAsTheirChildren)
{
    Hosted<RowLayout> hosted({300, 60});
    auto* fixed = hosted.layout->addChild<ColumnLayout>();
    Box* a = addBox(fixed, {40.5, 20});
    Box* b = addBox(fixed, {70, 20});
    auto* capped = hosted.layout->addChild<ColumnLayout>();
    Box* c = addBox(capped, {30, 10}, {.fillWidth = true, .maximumWidth = 90});
    Box* d = hosted.add({20, 10}, {.fillWidth = true});
    hosted.run();
    expectGeometry(fixed, {0, 8, 70, 45});
    expectGeometry(a, {0, 0, 41, 20});
    expectGeometry(b, {0, 25, 70, 20});
    expectGeometry(capped, {75, 25, 90, 10});
    expectGeometry(c, {0, 0, 90, 10});
    expectGeometry(d, {170, 25, 130, 10});
}

TEST(CellLayoutTest, NestedRowShrinksToItsChildrensMinimum)
{
    Hosted<RowLayout> hosted({120, 30});
    auto* inner = hosted.layout->addChild<RowLayout>();
    Box* a = addBox(inner, {80, 10}, {.fillWidth = true, .minimumWidth = 50});
    Box* b = addBox(inner, {40, 10});
    Box* c = hosted.add({60, 10}, {.fillWidth = true});
    hosted.run();
    expectGeometry(inner, {0, 10, 104, 10});
    expectGeometry(a, {0, 0, 59, 10});
    expectGeometry(b, {64, 0, 40, 10});
    expectGeometry(c, {109, 10, 11, 10});
}

TEST(CellLayoutTest, NestedRowsHeightenUpToTheirChildrensMaximum)
{
    Hosted<ColumnLayout> hosted({100, 200});
    hosted.layout->setSpacing(4);
    auto* capped = hosted.layout->addChild<RowLayout>();
    Box* a = addBox(capped, {30, 20}, {.fillHeight = true, .maximumHeight = 70});
    Box* b = addBox(capped, {30, 25});
    auto* growing = hosted.layout->addChild<RowLayout>();
    Box* c = addBox(growing, {30, 15}, {.fillHeight = true});
    hosted.run();
    expectGeometry(capped, {0, 0, 65, 70});
    expectGeometry(a, {0, 0, 30, 70});
    expectGeometry(b, {35, 23, 30, 25});
    expectGeometry(growing, {0, 74, 30, 126});
    expectGeometry(c, {0, 0, 30, 126});
}

// A column anchored with fractional margins in a card.
TEST(CellLayoutTest, PaddingIsAnchorsMargins)
{
    Hosted<ColumnLayout> hosted({260, 120});
    hosted.layout->setSpacing(6);
    hosted.layout->setPadding({10.5, 7.25, 10.5, 7.25});
    Box* a = hosted.add({50.4, 17.2}, {.fillWidth = true});
    Box* b = hosted.add({40.6, 20}, {.horizontalAlignment = Alignment::Center});
    Box* c = hosted.add({30, 10}, {.fillHeight = true});
    hosted.run();
    expectGeometry(a, {10.5, 7.25, 239, 18});
    expectGeometry(b, {109.5, 31.25, 41, 20});
    expectGeometry(c, {10.5, 57.25, 30, 56});
    EXPECT_EQ(hosted.layout->sizeHint(), (SizeF{72, 74.5}));
    EXPECT_EQ(hosted.layout->heightForWidth(260), 74.5);
}

// An item's own size counts only when its implicit size is not positive.
TEST(CellLayoutTest, OwnSizeIsAFallback)
{
    Hosted<RowLayout> hosted({200, 60});
    Box* field = hosted.add({80, 34.7}, {.fallbackHeight = 40});
    Box* spacer = hosted.add({0, 0}, {.fallbackWidth = 1, .fallbackHeight = 1});
    hosted.add({30, 17});
    hosted.run();
    EXPECT_EQ(field->geometry().height, 35);
    EXPECT_EQ(spacer->geometry().width, 1);
    EXPECT_EQ(hosted.layout->sizeHint(), (SizeF{80 + 5 + 1 + 5 + 30, 35}));
}

TEST(CellLayoutTest, AnchorsCentreRoundsEachHalf)
{
    EXPECT_EQ(rounding::centeredStart(44, 17), 13);
    EXPECT_EQ(rounding::centeredStart(45, 17), 14);
    EXPECT_EQ(rounding::centeredStart(30, 15.5), 7);
}
