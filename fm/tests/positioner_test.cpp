// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testutil.h"

#include "model/foldermodel.h"
#include "model/positioner.h"

#include "base/url.h"
#include "ui/application.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <set>

using namespace Lingmo;
using fm::FolderModel;
using fm::Positioner;
using fmtest::runUntil;
using fmtest::TemporaryDir;

extern ui::Application* s_app;

namespace {

struct Desktop
{
    explicit Desktop(const TemporaryDir& directory)
        : model(nullptr, true)
        , positioner(model)
    {
        model.setSortMode(-1);
        model.setUrl(base::url::fromLocalFile(directory.path()));
    }

    bool listed(int count)
    {
        return runUntil(s_app->loop(), [&] {
            return model.status() == FolderModel::Status::Ready && model.count() == count
                    && positioner.cellCount() > 0;
        });
    }

    int cellOf(const TemporaryDir& directory, const std::string& name) const
    {
        return positioner.cellOfUrl(base::url::fromLocalFile(directory.filePath(name)));
    }

    FolderModel model;
    Positioner positioner;
};

} // namespace

// New icons fill the rightmost column from the top, then the one beside it.
TEST(PositionerTest, DefaultCellsStartAtTheRight)
{
    TemporaryDir directory;
    for (const char* name : {"a", "b", "c"})
        fmtest::writeFile(directory.filePath(name));
    Desktop desktop(directory);
    desktop.positioner.setGrid(4, 2);
    ASSERT_TRUE(desktop.listed(3));
    std::vector<int> cells;
    for (const char* name : {"a", "b", "c"})
        cells.push_back(desktop.cellOf(directory, name));
    std::sort(cells.begin(), cells.end());
    // Column 3 is cells 6 and 7, column 2 cells 4 and 5.
    EXPECT_EQ(cells, (std::vector<int> {4, 6, 7}));
}

TEST(PositionerTest, SavedLayoutKeepsTheDistanceFromTheRightEdge)
{
    TemporaryDir directory;
    fmtest::writeFile(directory.filePath("a"));
    fmtest::writeFile(directory.filePath("b"));
    const std::string a = base::url::fromLocalFile(directory.filePath("a"));
    const std::string b = base::url::fromLocalFile(directory.filePath("b"));
    Desktop desktop(directory);
    desktop.positioner.setGrid(4, 3);
    ASSERT_TRUE(desktop.listed(2));
    // Saved on a grid of 4 columns: a in column 1 row 2, b in column 3 row 0.
    desktop.positioner.setPositions({"4", "3", a, "1", "2", b, "3", "0"});
    EXPECT_EQ(desktop.positioner.cellOfUrl(a), 1 * 3 + 2);
    EXPECT_EQ(desktop.positioner.cellOfUrl(b), 3 * 3 + 0);

    // Two columns more: both move right with the edge.
    desktop.positioner.setGrid(6, 3);
    EXPECT_EQ(desktop.positioner.cellOfUrl(a), 3 * 3 + 2);
    EXPECT_EQ(desktop.positioner.cellOfUrl(b), 5 * 3 + 0);
}

TEST(PositionerTest, MovesAvoidTakenCellsAndAreSaved)
{
    TemporaryDir directory;
    fmtest::writeFile(directory.filePath("a"));
    fmtest::writeFile(directory.filePath("b"));
    Desktop desktop(directory);
    desktop.positioner.setGrid(3, 3);
    ASSERT_TRUE(desktop.listed(2));
    const int a = desktop.cellOf(directory, "a");
    const int b = desktop.cellOf(directory, "b");

    desktop.positioner.move({{a, 0}});
    EXPECT_EQ(desktop.cellOf(directory, "a"), 0);
    // Onto a taken cell: the next blank one.
    desktop.positioner.move({{b, 0}});
    EXPECT_EQ(desktop.cellOf(directory, "b"), 1);

    bool saved = false;
    const base::Subscription subscription = desktop.positioner.positionsChanged.connect([&] { saved = true; });
    EXPECT_TRUE(runUntil(s_app->loop(), [&] { return saved; }));
    const auto& positions = desktop.positioner.positions();
    ASSERT_EQ(positions.size(), 8u);
    EXPECT_EQ(positions[0], "3");
    EXPECT_EQ(positions[1], "3");
}

TEST(PositionerTest, NearestItemFollowsTheArrows)
{
    TemporaryDir directory;
    for (const char* name : {"a", "b", "c"})
        fmtest::writeFile(directory.filePath(name));
    Desktop desktop(directory);
    desktop.positioner.setGrid(3, 3);
    ASSERT_TRUE(desktop.listed(3));
    desktop.positioner.move({{desktop.cellOf(directory, "a"), 0}, {desktop.cellOf(directory, "b"), 1},
                             {desktop.cellOf(directory, "c"), 4}});
    // Cells 0 and 1 share a column: the next one down is 1.
    EXPECT_EQ(desktop.positioner.nearestItem(0, Positioner::Direction::Right), 1);
    EXPECT_EQ(desktop.positioner.nearestItem(1, Positioner::Direction::Down), 4);
    EXPECT_EQ(desktop.positioner.nearestItem(4, Positioner::Direction::Up), 1);
}

TEST(PositionerTest, NewFilesTakeFreeCells)
{
    TemporaryDir directory;
    fmtest::writeFile(directory.filePath("a"));
    Desktop desktop(directory);
    desktop.positioner.setGrid(3, 3);
    ASSERT_TRUE(desktop.listed(1));
    EXPECT_EQ(desktop.cellOf(directory, "a"), 6);
    fmtest::writeFile(directory.filePath("b"));
    desktop.model.refresh();
    ASSERT_TRUE(runUntil(s_app->loop(), [&] { return desktop.cellOf(directory, "b") >= 0; }));
    EXPECT_EQ(desktop.cellOf(directory, "b"), 7);
}

// A new folder or file from the menu goes on the cell that was right-clicked.
TEST(PositionerTest, NewDocumentTakesTheClickedCell)
{
    TemporaryDir directory;
    fmtest::writeFile(directory.filePath("a"));
    Desktop desktop(directory);
    desktop.positioner.setGrid(3, 3);
    ASSERT_TRUE(desktop.listed(1));
    EXPECT_EQ(desktop.cellOf(directory, "a"), 6);

    std::vector<std::string> moved;
    const base::Subscription subscription = desktop.model.move.connect(
            [&](int, int, const std::vector<std::string>& urls) { moved = urls; });
    desktop.model.setNewDocumentPosition(40, 300, 1);
    desktop.model.newTextFile();
    desktop.model.refresh();
    ASSERT_TRUE(runUntil(s_app->loop(), [&] { return !moved.empty(); }));
    EXPECT_EQ(desktop.cellOf(directory, "New Text"), 1);
    // The view is asked to put it under the click as well, which finds it there.
    EXPECT_EQ(moved, (std::vector<std::string> {base::url::fromLocalFile(directory.filePath("New Text"))}));
}

// A taken cell first gives way to the default order; the view's follow-up move
// then takes the next blank cell after the clicked one, down the column.
TEST(PositionerTest, NewDocumentOnATakenCellGoesToTheNextBlankOne)
{
    TemporaryDir directory;
    fmtest::writeFile(directory.filePath("a"));
    fmtest::writeFile(directory.filePath("b"));
    Desktop desktop(directory);
    desktop.positioner.setGrid(3, 3);
    ASSERT_TRUE(desktop.listed(2));
    desktop.positioner.move({{desktop.cellOf(directory, "a"), 0}, {desktop.cellOf(directory, "b"), 3}});
    EXPECT_EQ(desktop.cellOf(directory, "a"), 0);
    EXPECT_EQ(desktop.cellOf(directory, "b"), 3);

    desktop.model.setNewDocumentPosition(10, 10, 3);
    desktop.model.newTextFile();
    desktop.model.refresh();
    ASSERT_TRUE(runUntil(s_app->loop(), [&] { return desktop.cellOf(directory, "New Text") >= 0; }));
    EXPECT_EQ(desktop.cellOf(directory, "New Text"), 6);

    desktop.positioner.move({{6, 3}});
    EXPECT_EQ(desktop.cellOf(directory, "New Text"), 4);
}

// A cell outside the grid is not kept: the default order applies.
TEST(PositionerTest, NewDocumentOutsideTheGridTakesTheDefaultCell)
{
    TemporaryDir directory;
    fmtest::writeFile(directory.filePath("a"));
    Desktop desktop(directory);
    desktop.positioner.setGrid(3, 3);
    ASSERT_TRUE(desktop.listed(1));
    desktop.model.setNewDocumentPosition(10, 10, 9);
    desktop.model.newTextFile();
    desktop.model.refresh();
    ASSERT_TRUE(runUntil(s_app->loop(), [&] { return desktop.cellOf(directory, "New Text") >= 0; }));
    EXPECT_EQ(desktop.cellOf(directory, "New Text"), 7);
}

// A drop between icons reserves the cell under the pointer for its first file.
TEST(PositionerTest, ReservedCellTakesTheDroppedFile)
{
    TemporaryDir directory;
    fmtest::writeFile(directory.filePath("a"));
    Desktop desktop(directory);
    desktop.positioner.setGrid(3, 3);
    ASSERT_TRUE(desktop.listed(1));
    desktop.model.reserveCell("dropped", 2);
    fmtest::writeFile(directory.filePath("dropped"));
    desktop.model.refresh();
    ASSERT_TRUE(runUntil(s_app->loop(), [&] { return desktop.cellOf(directory, "dropped") >= 0; }));
    EXPECT_EQ(desktop.cellOf(directory, "dropped"), 2);
}

// Without a saved layout the default cells go by name, whatever order the
// folder lists the files in.
TEST(PositionerTest, DefaultCellsFollowNames)
{
    TemporaryDir directory;
    for (const char* name : {"a", "b", "c", "d"})
        fmtest::writeFile(directory.filePath(name));
    Desktop desktop(directory);
    desktop.positioner.setGrid(3, 2);
    ASSERT_TRUE(desktop.listed(4));
    EXPECT_EQ(desktop.cellOf(directory, "a"), 4);
    EXPECT_EQ(desktop.cellOf(directory, "b"), 5);
    EXPECT_EQ(desktop.cellOf(directory, "c"), 2);
    EXPECT_EQ(desktop.cellOf(directory, "d"), 3);
}

// Files the saved layout has no record of fill the default cells in name
// order: the rightmost column from the top, then the one to its left.
TEST(PositionerTest, UnrecordedFilesFillFromTheRight)
{
    TemporaryDir directory;
    for (const char* name : {"d", "c", "b", "a"})
        fmtest::writeFile(directory.filePath(name));
    Desktop desktop(directory);
    desktop.positioner.setGrid(3, 2);
    ASSERT_TRUE(desktop.listed(4));
    const std::string c = base::url::fromLocalFile(directory.filePath("c"));
    desktop.positioner.setPositions({"3", "2", c, "2", "0"});
    EXPECT_EQ(desktop.cellOf(directory, "c"), 4);
    EXPECT_EQ(desktop.cellOf(directory, "a"), 5);
    EXPECT_EQ(desktop.cellOf(directory, "b"), 2);
    EXPECT_EQ(desktop.cellOf(directory, "d"), 3);
}

// A rename that never happened leaves no record: the old file going later
// does not hand its cell, not even for a moment, to the file that has the
// name it asked for.
TEST(PositionerTest, FailedRenameIsForgotten)
{
    TemporaryDir directory;
    fmtest::writeFile(directory.filePath("a"));
    fmtest::writeFile(directory.filePath("z"));
    Desktop desktop(directory);
    desktop.positioner.setGrid(3, 3);
    ASSERT_TRUE(desktop.listed(2));
    const int cell = desktop.cellOf(directory, "z");

    bool failed = false;
    const base::Subscription renameFailed = desktop.model.renameFailed.connect(
            [&](const std::string&, const std::string&) { failed = true; });
    // The test bus has no operations daemon, so the rename fails.
    desktop.model.rename(desktop.model.rowForUrl(base::url::fromLocalFile(directory.filePath("a"))), "z");
    ASSERT_TRUE(runUntil(s_app->loop(), [&] { return failed; }));

    std::set<int> cells;
    const base::Subscription changed = desktop.positioner.changed.connect(
            [&] { cells.insert(desktop.cellOf(directory, "z")); });
    std::filesystem::remove(directory.filePath("a"));
    ASSERT_TRUE(runUntil(s_app->loop(), [&] { return desktop.model.count() == 1; }));
    EXPECT_EQ(cells, std::set<int> {cell});
}
