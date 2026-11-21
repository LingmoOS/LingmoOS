// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "ui/drag.h"
#include "ui/file_drop_area.h"
#include "ui/test_input.h"
#include "ui/window.h"

using namespace Lingmo::UI;

namespace {

// A drag from another client; reads answer when told to, as a pipe does.
class Offer final : public DropOffer
{
public:
    Offer(std::vector<std::string> types, std::string data, uint32_t actions = CopyAction | MoveAction)
        : m_types(std::move(types))
        , m_data(std::move(data))
        , m_actions(actions)
    {
    }

    const std::vector<std::string>& mimeTypes() const override { return m_types; }
    uint32_t sourceActions() const override { return m_actions; }
    void read(const std::string& mimeType, std::function<void(std::string data)> done) override
    {
        reads.push_back(mimeType);
        pending.push_back([this, done = std::move(done)] { done(m_data); });
    }

    void answer()
    {
        auto tasks = std::move(pending);
        for (auto& task : tasks)
            task();
    }

    std::vector<std::string> reads;
    std::vector<std::function<void()>> pending;

private:
    std::vector<std::string> m_types;
    std::string m_data;
    uint32_t m_actions;
};

// A child that takes its own kind of drag.
class EventTarget : public View
{
public:
    bool canDrop(const DropOffer& offer) const override { return offer.hasMimeType("application/x-event"); }
    uint32_t dragUpdate(const DragEvent&) override { return MoveAction; }
    uint32_t drop(const DragEvent&) override
    {
        ++drops;
        return MoveAction;
    }

    int drops = 0;
};

struct Fixture
{
    Fixture()
    {
        auto area = std::make_unique<FileDropArea>();
        dropArea = area.get();
        child = area->addChild<EventTarget>();
        dropArea->filter = [](const std::string& url) {
            return url.size() > 4 && url.compare(url.size() - 4, 4, ".ics") == 0;
        };
        dropArea->onDropped = [this](const std::vector<std::string>& urls) { dropped.push_back(urls); };
        dropArea->onContainsDragChanged = [this] { containsChanges.push_back(dropArea->containsDrag()); };
        window.resize({300, 200});
        window.setContentView(std::move(area));
        window.show();
        window.grab();
    }

    Window window;
    FileDropArea* dropArea;
    EventTarget* child;
    std::vector<std::vector<std::string>> dropped;
    std::vector<bool> containsChanges;
};

const std::string kList = "file:///home/a.ics\r\nfile:///home/b.txt\r\nfile:///home/c%20d.ics\r\n";

} // namespace

TEST(FileDropAreaTest, TakesWantedFiles)
{
    Fixture f;
    Offer offer({"text/plain", "text/uri-list"}, kList);
    EXPECT_EQ(TestInput::dragEnter(f.window, {50, 50}, &offer), uint32_t(CopyAction));
    EXPECT_EQ(offer.reads, std::vector<std::string>{"text/uri-list"});
    EXPECT_TRUE(f.dropArea->containsDrag());
    offer.answer();
    EXPECT_EQ(TestInput::dragMove(f.window, {60, 60}), uint32_t(CopyAction));
    EXPECT_EQ(TestInput::drop(f.window), uint32_t(CopyAction));
    ASSERT_EQ(f.dropped.size(), 1u);
    EXPECT_EQ(f.dropped[0], (std::vector<std::string>{"file:///home/a.ics", "file:///home/c%20d.ics"}));
    EXPECT_EQ(f.containsChanges, (std::vector<bool>{true, false}));
}

// Nothing wanted: refused once the list is known, and no drop follows.
TEST(FileDropAreaTest, RefusesWhenNothingIsWanted)
{
    Fixture f;
    Offer offer({"text/uri-list"}, "file:///home/b.txt\n");
    TestInput::dragEnter(f.window, {50, 50}, &offer);
    offer.answer();
    EXPECT_EQ(TestInput::dragMove(f.window, {60, 60}), uint32_t(NoAction));
    EXPECT_EQ(TestInput::drop(f.window), uint32_t(NoAction));
    EXPECT_TRUE(f.dropped.empty());
    EXPECT_FALSE(f.dropArea->containsDrag());
}

// Dropped before the list arrived: taken, and delivered when it does.
TEST(FileDropAreaTest, DeliversALateList)
{
    Fixture f;
    Offer offer({"text/uri-list"}, kList);
    TestInput::dragEnter(f.window, {50, 50}, &offer);
    EXPECT_EQ(TestInput::drop(f.window), uint32_t(CopyAction));
    EXPECT_TRUE(f.dropped.empty());
    offer.answer();
    ASSERT_EQ(f.dropped.size(), 1u);
    EXPECT_EQ(f.dropped[0].size(), 2u);
}

// A drag that left before its list arrived reports nothing.
TEST(FileDropAreaTest, IgnoresTheListOfALeftDrag)
{
    Fixture f;
    Offer offer({"text/uri-list"}, kList);
    TestInput::dragEnter(f.window, {50, 50}, &offer);
    TestInput::dragLeave(f.window);
    offer.answer();
    EXPECT_TRUE(f.dropped.empty());
    EXPECT_EQ(f.containsChanges, (std::vector<bool>{true, false}));
}

// Only the source's actions; a child keeps the drags it takes.
TEST(FileDropAreaTest, FollowsTheSourceAndLeavesChildrenTheirDrags)
{
    Fixture f;
    Offer moveOnly({"text/uri-list"}, kList, MoveAction);
    EXPECT_EQ(TestInput::dragEnter(f.window, {50, 50}, &moveOnly), uint32_t(MoveAction));
    TestInput::dragLeave(f.window);

    Offer event({"application/x-event"}, {});
    EXPECT_EQ(TestInput::dragEnter(f.window, {50, 50}, &event), uint32_t(MoveAction));
    EXPECT_EQ(TestInput::drop(f.window), uint32_t(MoveAction));
    EXPECT_EQ(f.child->drops, 1);
    EXPECT_TRUE(event.reads.empty());

    Offer text({"text/plain"}, "x");
    EXPECT_EQ(TestInput::dragEnter(f.window, {50, 50}, &text), uint32_t(NoAction));
}
