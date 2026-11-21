// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "model/positioner.h"
#include "core/standardfolders.h"
#include "model/foldercontents.h"
#include "model/foldermodel.h"

#include "base/collator.h"
#include "base/url.h"

#include <algorithm>
#include <charconv>
#include <cstdlib>
#include <optional>
#include <set>
#include <tuple>
#include <utility>

namespace fm {

static std::string nameOf(const std::string& url)
{
    return fileName(base::url::toLocalFile(url).value_or(url));
}

// In the order the folder view lists names, not the order the folder was
// read in; names the collator equates ("a1", "a01") keep one fixed order.
static void sortByName(std::vector<std::string>& urls)
{
    base::Collator collator;
    collator.setNumericMode(true);
    std::sort(urls.begin(), urls.end(), [&collator](const std::string& a, const std::string& b) {
        const int order = collator.compare(nameOf(a), nameOf(b));
        return order != 0 ? order < 0 : nameOf(a) < nameOf(b);
    });
}

static std::optional<int> number(const std::string& text)
{
    int value = 0;
    const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
    if (text.empty() || error != std::errc() || end != text.data() + text.size())
        return std::nullopt;
    return value;
}

void Positioner::Layout::put(int cell, const std::string& url)
{
    urls[cell] = url;
    cells[url] = cell;
}

void Positioner::Layout::takeUrl(const std::string& url)
{
    if (const auto found = cells.find(url); found != cells.end()) {
        urls.erase(found->second);
        cells.erase(found);
    }
}

Positioner::Positioner(FolderModel& model)
    : m_model(model)
{
    m_save.setSingleShot(true);
    m_save.setInterval(std::chrono::milliseconds(0));
    m_save.onTimeout = [this] { savePositions(); };

    m_subscriptions.push_back(model.rowsChanged.connect([this](const ContentsChange& change) {
        contentsChanged(change);
    }));
    m_subscriptions.push_back(model.statusChanged.connect([this] {
        if (m_model.status() != FolderModel::Status::Listing)
            listed();
    }));
    m_subscriptions.push_back(model.urlChanged.connect([this] { reset(); }));
    // A rename keeps the icon where it is, under its new URL.
    m_subscriptions.push_back(model.renaming.connect([this](const std::string& from, const std::string& to) {
        m_renames[from] = to;
    }));
    m_subscriptions.push_back(model.renameFailed.connect([this](const std::string& from, const std::string& to) {
        if (const auto found = m_renames.find(from); found != m_renames.end() && found->second == to)
            m_renames.erase(found);
    }));
    placeAllByDefault();
}

Positioner::~Positioner() = default;

void Positioner::setGrid(int stripes, int perStripe)
{
    if (stripes == m_stripes && perStripe == m_perStripe)
        return;

    // A change not saved yet is saved against the grid it was made on.
    if (m_save.isActive()) {
        m_save.stop();
        savePositions();
    }
    m_stripes = stripes;
    m_perStripe = perStripe;

    // Without a saved layout every icon sits on its default cell, which depends on the grid.
    if (m_positions.empty()) {
        if (!m_deferApply)
            placeAllByDefault();
    } else if (gridCells() > 0 && !m_layout.urls.empty()) {
        applyPositions();
    }
    changed.emit();
}

void Positioner::setCorner(Corner corner)
{
    if (m_corner == corner)
        return;
    m_corner = corner;
    // Re-derive default cells; only affects icons that have no saved position.
    if (m_positions.empty() && !m_deferApply)
        placeAllByDefault();
    changed.emit();
}

int Positioner::cellCount() const
{
    return m_layout.last() + 1;
}

int Positioner::rowAt(int cell) const
{
    const auto found = m_layout.urls.find(cell);
    return found == m_layout.urls.end() ? -1 : m_model.rowForUrl(found->second);
}

int Positioner::cellOfRow(int row) const
{
    const FileItemPtr item = m_model.item(row);
    return item ? cellOfUrl(item->url) : -1;
}

int Positioner::cellOfUrl(const std::string& url) const
{
    const auto found = m_layout.cells.find(url);
    return found == m_layout.cells.end() ? -1 : found->second;
}

// Zero while the view has not reported its grid yet.
int Positioner::gridCells() const
{
    return m_stripes > 0 && m_perStripe > 0 ? m_stripes * m_perStripe : 0;
}

// Outside the grid an icon would sit off screen.
bool Positioner::insideGrid(int cell) const
{
    return cell >= 0 && (gridCells() == 0 || cell < gridCells());
}

// The corner stripe from the top or bottom, then the next stripe inward; past
// the grid, or with no grid yet, the cells simply follow on.
int Positioner::defaultCell(int ordinal) const
{
    if (ordinal >= gridCells())
        return ordinal;

    const int stripeOrdinal = ordinal / m_perStripe;
    const int posOrdinal    = ordinal % m_perStripe;

    // Stripe index (column): left corners fill 0→N, right corners fill N→0.
    const bool rightSide = (m_corner == Corner::TopRight || m_corner == Corner::BottomRight);
    const int stripe = rightSide ? m_stripes - 1 - stripeOrdinal : stripeOrdinal;

    // Position within the stripe: top corners fill 0→N, bottom corners fill N→0.
    const bool bottomSide = (m_corner == Corner::BottomLeft || m_corner == Corner::BottomRight);
    const int pos = bottomSide ? m_perStripe - 1 - posOrdinal : posOrdinal;

    return stripe * m_perStripe + pos;
}

// The first free default cell, else the first gap, else the cell after the last.
int Positioner::firstFreeCell(const Layout& layout) const
{
    for (int ordinal = 0; ordinal < gridCells(); ++ordinal) {
        const int cell = defaultCell(ordinal);
        if (!layout.taken(cell))
            return cell;
    }

    int cell = 0;
    while (layout.taken(cell))
        ++cell;
    return cell;
}

// The free grid cell closest to (stripe, pos), the lower one on a tie; -1 when
// the grid is full or unknown.
int Positioner::nearestFreeCell(const Layout& layout, int stripe, int pos) const
{
    int nearest = -1;
    int nearestDistance = 0;
    for (int cell = 0; cell < gridCells(); ++cell) {
        if (layout.taken(cell))
            continue;

        const int ds = cell / m_perStripe - stripe;
        const int dp = cell % m_perStripe - pos;
        const int distance = ds * ds + dp * dp;
        if (nearest < 0 || distance < nearestDistance) {
            nearest = cell;
            nearestDistance = distance;
        }
    }
    return nearest;
}

// `from` itself when free, else the next free cell after it, wrapping inside
// the grid; past the last cell when the grid is full.
int Positioner::nextFreeCell(const Layout& layout, int from) const
{
    const int cells = gridCells();
    if (cells == 0 || !layout.taken(from)) {
        while (layout.taken(from))
            ++from;
        return from;
    }

    for (int step = 0; step < cells; ++step) {
        const int cell = (from + step) % cells;
        if (!layout.taken(cell))
            return cell;
    }
    return layout.last() + 1;
}

void Positioner::placeAllByDefault()
{
    std::vector<std::string> urls;
    for (int row = 0; row < m_model.count(); ++row)
        urls.push_back(m_model.item(row)->url);
    sortByName(urls);

    m_layout = Layout();
    for (size_t ordinal = 0; ordinal < urls.size(); ++ordinal)
        m_layout.put(defaultCell(int(ordinal)), urls[ordinal]);
}

void Positioner::reset()
{
    m_save.stop();
    m_renames.clear();
    m_deferredMoves.clear();
    placeAllByDefault();
    if (!m_positions.empty()) {
        m_positions.clear();
        positionsChanged.emit();
    }
    changed.emit();
}

void Positioner::contentsChanged(const ContentsChange& change)
{
    // The saved layout is applied to the whole listing once it is complete.
    if (m_deferApply)
        return;
    if (change.removed.empty() && change.added.empty())
        return;

    std::set<std::string> carried;
    for (const std::string& url : change.removed) {
        const int cell = cellOfUrl(url);
        if (cell < 0)
            continue;

        m_layout.takeUrl(url);
        if (const auto rename = m_renames.find(url); rename != m_renames.end()) {
            m_layout.put(cell, rename->second);
            carried.insert(rename->second);
            m_renames.erase(rename);
        }
    }

    std::vector<std::string> added = change.added;
    sortByName(added);

    const bool fresh = m_layout.urls.empty();
    int ordinal = 0;
    for (const std::string& url : added) {
        if (carried.contains(url) || m_layout.cells.contains(url))
            continue;

        // A cell asked for up front, such as a new folder on the clicked cell,
        // wins over the default order as long as it is in the grid and blank.
        int cell = m_model.newDocumentCell(nameOf(url));
        if (cell >= 0 && (!insideGrid(cell) || m_layout.taken(cell)))
            cell = -1;
        if (cell < 0)
            cell = fresh ? defaultCell(ordinal) : firstFreeCell(m_layout);
        if (m_layout.taken(cell))
            cell = firstFreeCell(m_layout);

        m_layout.put(cell, url);
        ++ordinal;
    }

    changed.emit();
    m_save.start();
}

void Positioner::listed()
{
    if (m_deferApply || m_positions.size() >= 5)
        applyPositions();

    if (!m_deferredMoves.empty())
        move(std::exchange(m_deferredMoves, {}));
}

void Positioner::setPositions(std::vector<std::string> positions)
{
    if (positions == m_positions)
        return;

    m_save.stop();
    m_positions = std::move(positions);
    positionsChanged.emit();
    applyPositions();
}

void Positioner::applyPositions()
{
    if (m_model.status() == FolderModel::Status::Listing) {
        m_deferApply = true;
        return;
    }

    const bool deferred = std::exchange(m_deferApply, false);
    Layout restored;
    if (!restore(&restored)) {
        // Nothing to apply: whatever came while waiting gets default cells.
        if (deferred) {
            placeAllByDefault();
            changed.emit();
        }
        return;
    }

    // A reflow onto another grid is not saved; only file changes and moves are.
    if (!(restored == m_layout)) {
        m_layout = std::move(restored);
        changed.emit();
    }
}

// The listed files on the cells the saved layout gives them, on the current grid.
bool Positioner::restore(Layout* layout) const
{
    if (m_positions.size() < 5 || (m_positions.size() - 2) % 3 != 0)
        return false;
    const std::optional<int> savedStripes = number(m_positions[0]);
    if (!savedStripes || *savedStripes <= 0)
        return false;

    std::set<std::string> listed;
    for (int row = 0; row < m_model.count(); ++row)
        listed.insert(m_model.item(row)->url);

    struct Record
    {
        std::string url;
        int stripe;
        int pos;
    };
    std::vector<Record> displaced;

    for (size_t at = 2; at + 2 < m_positions.size(); at += 3) {
        const std::string& url = m_positions[at];
        const std::optional<int> stripe = number(m_positions[at + 1]);
        const std::optional<int> pos = number(m_positions[at + 2]);
        if (!stripe || !pos || !listed.contains(url) || layout->cells.contains(url))
            continue;

        // Kept at the same distance from the right edge when the grid's width changes.
        const Record record {url, *stripe + m_stripes - *savedStripes, *pos};
        const bool fits = gridCells() > 0 && record.stripe >= 0 && record.stripe < m_stripes && record.pos >= 0
                && record.pos < m_perStripe;
        const int cell = fits ? record.stripe * m_perStripe + record.pos : -1;
        if (cell < 0 || layout->taken(cell)) {
            displaced.push_back(record);
            continue;
        }
        layout->put(cell, url);
    }

    // A record that no longer fits, or lost its cell, stays near where it was.
    for (const Record& record : displaced) {
        if (layout->cells.contains(record.url))
            continue;

        const int stripe = std::clamp(record.stripe, 0, std::max(0, m_stripes - 1));
        const int pos = std::clamp(record.pos, 0, std::max(0, m_perStripe - 1));
        const int cell = nearestFreeCell(*layout, stripe, pos);
        layout->put(cell >= 0 ? cell : firstFreeCell(*layout), record.url);
    }

    // Files without a record fill the free cells by name.
    std::vector<std::string> unrecorded;
    for (const std::string& url : listed) {
        if (!layout->cells.contains(url))
            unrecorded.push_back(url);
    }
    sortByName(unrecorded);
    for (const std::string& url : unrecorded) {
        // A renamed file has no record under its new URL but already has a cell.
        const int current = cellOfUrl(url);
        const bool keep = current >= 0 && insideGrid(current) && !layout->taken(current);
        layout->put(keep ? current : firstFreeCell(*layout), url);
    }
    return true;
}

void Positioner::move(const std::vector<std::pair<int, int>>& moves)
{
    // The rows are about to change under the cells.
    if (m_model.status() == FolderModel::Status::Listing) {
        m_deferredMoves = moves;
        return;
    }

    // Every moving icon leaves its cell before any lands, so icons dragged
    // together can take each other's cells.
    std::vector<std::pair<std::string, int>> moving;
    for (const auto& [from, to] : moves) {
        const auto found = m_layout.urls.find(from);
        if (found != m_layout.urls.end() && from != to)
            moving.emplace_back(found->second, to);
    }
    for (const auto& [url, to] : moving)
        m_layout.takeUrl(url);
    for (const auto& [url, to] : moving)
        m_layout.put(to < 0 ? firstFreeCell(m_layout) : nextFreeCell(m_layout, to), url);

    changed.emit();
    m_save.start();
}

int Positioner::nearestItem(int cell, Direction direction) const
{
    if (m_layout.urls.empty())
        return -1;
    if (cell < 0)
        return m_layout.urls.begin()->first;
    if (m_perStripe <= 0)
        return -1;

    // (x, y) on a grid of perStripe columns; the view maps its arrows into it.
    const bool vertical = direction == Direction::Up || direction == Direction::Down;
    const int sign = direction == Direction::Right || direction == Direction::Down ? 1 : -1;
    const int x = cell % m_perStripe;
    const int y = cell / m_perStripe;

    // Closest first, then one in line with the cell, then the lower cell.
    int nearest = -1;
    std::tuple<int, bool, int> nearestScore;
    for (const auto& [candidate, url] : m_layout.urls) {
        const int cx = candidate % m_perStripe;
        const int cy = candidate / m_perStripe;
        const int ahead = sign * (vertical ? cy - y : cx - x);
        if (ahead <= 0)
            continue;

        const std::tuple<int, bool, int> score {std::abs(cx - x) + std::abs(cy - y), vertical ? cx != x : cy != y,
                                                candidate};
        if (nearest < 0 || score < nearestScore) {
            nearest = candidate;
            nearestScore = score;
        }
    }
    return nearest;
}

void Positioner::savePositions()
{
    std::vector<std::string> positions;
    if (!m_layout.urls.empty() && m_perStripe > 0) {
        positions.push_back(std::to_string(m_stripes));
        positions.push_back(std::to_string(m_perStripe));
        // In cell order, so an unchanged desktop saves the same list.
        for (const auto& [cell, url] : m_layout.urls) {
            positions.push_back(url);
            positions.push_back(std::to_string(cell / m_perStripe));
            positions.push_back(std::to_string(cell % m_perStripe));
        }
    }

    if (positions != m_positions) {
        m_positions = std::move(positions);
        positionsChanged.emit();
    }
}

} // namespace fm
