// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/common.h"

#include "base/signal.h"
#include "base/timer.h"

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

namespace fm {

class FolderModel;

// The desktop's icons on fixed cells. The grid runs
// in stripes (columns, as the desktop flows top to bottom) of perStripe
// cells; cell = stripe * perStripe + pos. New icons take the corner stripe
// first (configurable), filled from the top or bottom. Cells are kept by URL,
// saved as [stripes, perStripe, then url, stripe, pos for each icon].
class Positioner
{
public:
    enum class Direction {
        Left,
        Right,
        Up,
        Down,
    };

    // Matches DesktopGridView::IconCorner: TopRight=0, TopLeft=1,
    // BottomLeft=2, BottomRight=3.
    enum class Corner { TopRight = 0, TopLeft = 1, BottomLeft = 2, BottomRight = 3 };

    explicit Positioner(FolderModel& model);
    ~Positioner();

    void setGrid(int stripes, int perStripe);
    // Change the corner new icons fill from; triggers a re-layout of defaults.
    void setCorner(Corner corner);
    int stripes() const { return m_stripes; }
    int perStripe() const { return m_perStripe; }

    // One past the last cell in use: the cells a view lays out.
    int cellCount() const;
    // The model row on a cell, or -1 for a blank one.
    int rowAt(int cell) const;
    int cellOfRow(int row) const;
    int cellOfUrl(const std::string& url) const;
    bool isBlank(int cell) const { return rowAt(cell) < 0; }

    // The saved layout; applied once the folder is listed.
    const std::vector<std::string>& positions() const { return m_positions; }
    void setPositions(std::vector<std::string> positions);
    // Pairs of cells: each icon on the first moves to the second, the next
    // free one when that is taken, -1 for the first free cell.
    void move(const std::vector<std::pair<int, int>>& moves);
    // The icon nearest the cell in a direction, or -1.
    int nearestItem(int cell, Direction direction) const;
    // Every icon on its default cell, the saved layout forgotten.
    void reset();

    // Cells changed: icons came, went or moved.
    base::Signal<> changed;
    base::Signal<> positionsChanged;

private:
    // Which URL is on which cell, both ways.
    struct Layout
    {
        std::map<int, std::string> urls;
        std::unordered_map<std::string, int> cells;

        bool taken(int cell) const { return urls.contains(cell); }
        void put(int cell, const std::string& url);
        void takeUrl(const std::string& url);
        int last() const { return urls.empty() ? -1 : urls.rbegin()->first; }
        bool operator==(const Layout& other) const { return urls == other.urls; }
    };

    void contentsChanged(const struct ContentsChange& change);
    void listed();
    void placeAllByDefault();
    int gridCells() const;
    bool insideGrid(int cell) const;
    int defaultCell(int ordinal) const;
    int firstFreeCell(const Layout& layout) const;
    int nearestFreeCell(const Layout& layout, int stripe, int pos) const;
    int nextFreeCell(const Layout& layout, int from) const;
    bool restore(Layout* layout) const;
    void applyPositions();
    void savePositions();

    FolderModel& m_model;
    int m_stripes = 0;
    int m_perStripe = 0;
    Corner m_corner = Corner::TopRight;
    Layout m_layout;
    std::map<std::string, std::string> m_renames;
    std::vector<std::string> m_positions;
    bool m_deferApply = false;
    std::vector<std::pair<int, int>> m_deferredMoves;
    base::Timer m_save;
    std::vector<base::Subscription> m_subscriptions;
};

} // namespace fm
