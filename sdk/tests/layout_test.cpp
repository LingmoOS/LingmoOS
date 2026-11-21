// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "test_views.h"
#include "ui/cell_layout.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/test_input.h"
#include "ui/window.h"

using namespace Lingmo::UI;
using test::Box;

// A view whose size hint changes, counting its measurements and layouts.
class CountingView : public View
{
public:
    explicit CountingView(SizeF size)
        : m_size(size)
    {
    }

    // Without asking for layout, as a view should not.
    void setSizeQuietly(SizeF size) { m_size = size; }

    void setSize(SizeF size)
    {
        m_size = size;
        requestLayout();
    }

    SizeF sizeHint() const override
    {
        ++measurements;
        return m_size;
    }

    mutable int measurements = 0;
    int layouts = 0;

protected:
    void layout() override { ++layouts; }

private:
    SizeF m_size;
};

// Lays the content out at the given size by rendering it once.
template<typename T>
static T* host(Window& window, std::unique_ptr<T> content, SizeF size)
{
    T* view = content.get();
    window.resize(size);
    window.setContentView(std::move(content));
    window.show();
    window.grab();
    return view;
}

TEST(LayoutTest, ColumnStacksPreferredSizes)
{
    Window window;
    auto content = std::make_unique<Column>();
    content->setSpacing(5);
    content->setMargins(Margins::all(10));
    Box* first = content->addChild<Box>(SizeF{50, 10});
    Box* second = content->addChild<Box>(SizeF{60, 20});
    Box* third = content->addChild<Box>(SizeF{70, 30});
    Column* column = host(window, std::move(content), {200, 200});

    EXPECT_EQ(first->geometry(), (RectF{10, 10, 50, 10}));
    EXPECT_EQ(second->geometry(), (RectF{10, 25, 60, 20}));
    EXPECT_EQ(third->geometry(), (RectF{10, 50, 70, 30}));
    EXPECT_EQ(column->sizeHint(), (SizeF{90, 90}));
}

TEST(LayoutTest, ColumnSharesLeftoverByStretch)
{
    Window window;
    auto content = std::make_unique<Column>();
    Box* top = content->addChild<Box>(SizeF{10, 20});
    Box* one = content->addChild<Box>(SizeF{10, 0});
    one->setLayoutHints({.fillWidth = true, .fillHeight = true, .stretch = 1});
    Box* two = content->addChild<Box>(SizeF{10, 0});
    two->setLayoutHints({.fillHeight = true, .stretch = 2});
    host(window, std::move(content), {100, 110});

    EXPECT_EQ(top->geometry(), (RectF{0, 0, 10, 20}));
    EXPECT_EQ(one->geometry(), (RectF{0, 20, 100, 30}));
    EXPECT_EQ(two->geometry(), (RectF{0, 50, 10, 60}));
}

TEST(LayoutTest, RowCentresAcrossByDefault)
{
    Window window;
    auto content = std::make_unique<Row>();
    content->setSpacing(4);
    Box* small = content->addChild<Box>(SizeF{20, 10});
    Box* tall = content->addChild<Box>(SizeF{30, 30});
    View* stretch = content->addStretch();
    Box* end = content->addChild<Box>(SizeF{10, 10});
    end->setLayoutHints({.alignment = Alignment::End});
    host(window, std::move(content), {100, 50});

    EXPECT_EQ(small->geometry(), (RectF{0, 20, 20, 10}));
    EXPECT_EQ(tall->geometry(), (RectF{24, 10, 30, 30}));
    EXPECT_EQ(stretch->geometry().width, 100 - 20 - 30 - 10 - 3 * 4);
    EXPECT_EQ(end->geometry(), (RectF{90, 40, 10, 10}));
}

TEST(LayoutTest, HiddenChildrenTakeNoSpace)
{
    Window window;
    auto content = std::make_unique<Column>();
    content->setSpacing(10);
    content->addChild<Box>(SizeF{10, 10});
    Box* hidden = content->addChild<Box>(SizeF{10, 10});
    hidden->setVisible(false);
    Box* last = content->addChild<Box>(SizeF{10, 10});
    host(window, std::move(content), {100, 100});

    EXPECT_EQ(last->geometry().y, 20);
}

TEST(LayoutTest, GridSizesTracksByLargestChild)
{
    Window window;
    auto content = std::make_unique<Grid>(2);
    content->setColumnSpacing(10);
    content->setRowSpacing(5);
    Box* a = content->addChild<Box>(SizeF{40, 10});
    Box* b = content->addChild<Box>(SizeF{20, 20});
    b->setLayoutHints({.fillWidth = true});
    Box* c = content->addChild<Box>(SizeF{60, 30});
    Box* d = content->addChild<Box>(SizeF{10, 10});
    host(window, std::move(content), {200, 100});

    EXPECT_EQ(a->geometry(), (RectF{0, 5, 40, 10}));
    EXPECT_EQ(b->geometry(), (RectF{70, 0, 130, 20}));
    EXPECT_EQ(c->geometry(), (RectF{0, 25, 60, 30}));
    EXPECT_EQ(d->geometry(), (RectF{70, 35, 10, 10}));
}

TEST(LayoutTest, WrappedLabelGrowsWhenNarrow)
{
    Window window;
    auto content = std::make_unique<Column>();
    Label* label = content->addChild<Label>(
        "Several words that will not fit on one line once the column gets narrow");
    label->setWrap(true);
    label->setLayoutHints({.fillWidth = true});
    host(window, std::move(content), {600, 200});

    const double wide = label->heightForWidth(600);
    const double narrow = label->heightForWidth(120);
    EXPECT_GT(narrow, wide * 2);
    EXPECT_EQ(label->geometry().height, wide);
}

TEST(LayoutTest, DumpListsLogicalAndDeviceGeometry)
{
    Window window;
    window.setScale(1.5);
    auto content = std::make_unique<Column>();
    content->setName("content");
    content->setMargins(Margins::all(5));
    content->addChild<Box>(SizeF{11, 11})->setName("box");
    host(window, std::move(content), {21, 21});

    EXPECT_EQ(window.layoutDump(),
              "Column \"content\" logical 0 0 21 21 device 0 0 32 32\n"
              "  test::Box \"box\" logical 5 5 11 11 device 8 8 16 16\n");
}

TEST(LayoutTest, RequestLayoutLaysOutTheViewAndItsAncestorsOnly)
{
    Window window;
    auto content = std::make_unique<ColumnLayout>();
    auto* changing = content->addChild<RowLayout>()->addChild<CountingView>(SizeF{20, 10});
    auto* other = content->addChild<ColumnLayout>()->addChild<CountingView>(SizeF{30, 10});
    host(window, std::move(content), {100, 100});
    changing->layouts = 0;
    other->layouts = 0;
    other->measurements = 0;
    const PointF position = other->mapToWindow({});

    changing->setSize({40, 20});
    window.grab();

    EXPECT_EQ(changing->layouts, 1);
    EXPECT_EQ(changing->geometry().size(), (SizeF{40, 20}));
    // Its column moved but kept its size and measurements.
    EXPECT_NE(other->mapToWindow({}), position);
    EXPECT_EQ(other->layouts, 0);
    EXPECT_EQ(other->measurements, 0);
}

TEST(LayoutTest, ResizedViewsLayOutTheirChildren)
{
    Window window;
    auto content = std::make_unique<ColumnLayout>();
    auto* row = content->addChild<RowLayout>();
    row->setLayoutHints({.fillWidth = true});
    auto* filling = row->addChild<CountingView>(SizeF{10, 10});
    filling->setLayoutHints({.fillWidth = true});
    auto* fixed = content->addChild<CountingView>(SizeF{10, 10});
    host(window, std::move(content), {100, 100});
    filling->layouts = 0;
    fixed->layouts = 0;

    TestInput::configure(window, 150, 100, {});
    window.grab();

    EXPECT_EQ(filling->geometry().width, 150);
    EXPECT_EQ(filling->layouts, 1);
    EXPECT_EQ(fixed->layouts, 0);
}

TEST(LayoutTest, ParentPlacesAViewResizedOutsideAPass)
{
    Window window;
    auto content = std::make_unique<ColumnLayout>();
    auto* box = content->addChild<CountingView>(SizeF{20, 10});
    host(window, std::move(content), {100, 100});
    const RectF placed = box->geometry();

    box->setGeometry({0, 0, 50, 50});
    window.grab();

    EXPECT_EQ(box->geometry(), placed);
}

TEST(LayoutTest, ScaleChangeMeasuresEveryViewAgain)
{
    Window window;
    auto content = std::make_unique<ColumnLayout>();
    auto* box = content->addChild<ColumnLayout>()->addChild<CountingView>(SizeF{20, 10});
    host(window, std::move(content), {100, 100});
    box->measurements = 0;

    window.setScale(1.5);
    window.grab();

    EXPECT_GT(box->measurements, 0);
}

TEST(LayoutTest, MovedSubtreeMeasuresAgain)
{
    Window first;
    auto firstContent = std::make_unique<ColumnLayout>();
    auto* moving = firstContent->addChild<ColumnLayout>();
    auto* box = moving->addChild<CountingView>(SizeF{20, 10});
    ColumnLayout* firstRoot = host(first, std::move(firstContent), {100, 100});
    Window second;
    ColumnLayout* secondRoot = host(second, std::make_unique<ColumnLayout>(), {100, 100});
    box->measurements = 0;
    box->layouts = 0;

    secondRoot->addChild(firstRoot->takeChild(moving));
    second.grab();

    EXPECT_GT(box->measurements, 0);
    EXPECT_EQ(box->layouts, 1);
}

TEST(LayoutTest, VerifierReportsSizeHintsChangedWithoutLayout)
{
    Window window;
    auto content = std::make_unique<ColumnLayout>();
    auto* quiet = content->addChild<ColumnLayout>()->addChild<CountingView>(SizeF{20, 10});
    auto* other = content->addChild<CountingView>(SizeF{20, 10});
    host(window, std::move(content), {100, 100});
    const bool verified = TestInput::verifiesLayout();
    const int before = TestInput::layoutMismatches();
    TestInput::setVerifiesLayout(true);

    quiet->setSizeQuietly({40, 10});
    other->setSize({30, 10});
    window.grab();
    const int mismatches = TestInput::layoutMismatches() - before;
    TestInput::setVerifiesLayout(verified);

    EXPECT_GT(mismatches, 0);
}

TEST(LayoutTest, RaisedChildMovesToTheEndOfAFlow)
{
    Window window;
    auto content = std::make_unique<ColumnLayout>();
    auto* first = content->addChild<CountingView>(SizeF{20, 10});
    auto* second = content->addChild<CountingView>(SizeF{20, 10});
    ColumnLayout* column = host(window, std::move(content), {100, 100});
    ASSERT_LT(first->geometry().y, second->geometry().y);

    column->raiseChild(first);
    window.grab();

    EXPECT_GT(first->geometry().y, second->geometry().y);
}

// Places its child at an offset that the child sets from its own layout.
class OffsetParent : public View
{
public:
    double offset = 0;

protected:
    void layout() override
    {
        for (const auto& child : children())
            child->setGeometry({offset, 0, 10, 10});
    }
};

class OffsetChild : public View
{
protected:
    void layout() override
    {
        auto* parent = static_cast<OffsetParent*>(this->parent());
        if (parent->offset != 30) {
            parent->offset = 30;
            parent->requestLayout();
        }
    }
};

TEST(LayoutTest, ParentAskedFromAChildsLayoutLaysOutInTheSameFrame)
{
    Window window;
    auto content = std::make_unique<OffsetParent>();
    auto* child = content->addChild<OffsetChild>();
    host(window, std::move(content), {100, 100});

    EXPECT_EQ(child->geometry().x, 30);
}

// Asks its child for layout from its own, as a view filling its children does.
class FillingParent : public View
{
public:
    int layouts = 0;

protected:
    void layout() override
    {
        ++layouts;
        for (const auto& child : children()) {
            child->setGeometry(localRect());
            child->requestLayout();
        }
    }
};

TEST(LayoutTest, ChildrenAskedFromTheParentsLayoutLayOutInTheSamePass)
{
    Window window;
    auto content = std::make_unique<FillingParent>();
    auto* child = content->addChild<CountingView>(SizeF{10, 10});
    FillingParent* parent = host(window, std::move(content), {100, 100});
    parent->layouts = 0;
    child->layouts = 0;

    parent->requestLayout();
    window.grab();

    EXPECT_EQ(parent->layouts, 1);
    EXPECT_EQ(child->layouts, 1);
}

// Places a marker under the last view of its column, which the column
// places after this view's layout().
class FollowingParent : public View
{
public:
    FollowingParent()
    {
        column = addChild<ColumnLayout>();
        marker = addChild<CountingView>(SizeF{10, 2});
    }

    ColumnLayout* column;
    CountingView* marker;

protected:
    void layout() override { column->setGeometry(localRect()); }

    void childrenLaidOut() override
    {
        const View* last = column->children().back().get();
        marker->setGeometry({0, last->geometry().bottom(), 10, 2});
    }
};

TEST(LayoutTest, ChildrenLaidOutSeesTheirGeometry)
{
    Window window;
    auto content = std::make_unique<FollowingParent>();
    content->column->addChild<CountingView>(SizeF{20, 10});
    auto* growing = content->column->addChild<CountingView>(SizeF{20, 10});
    FollowingParent* parent = host(window, std::move(content), {100, 100});

    growing->setSize({20, 30});
    window.grab();

    EXPECT_EQ(parent->marker->geometry().y, growing->geometry().bottom());
}
