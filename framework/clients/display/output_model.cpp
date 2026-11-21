// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Upstream: chromium/src/ash/display/cros_display_config.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Cutefish: built on the compositor's native display requests.

#include "lingmo/display/output_model.h"

#include <algorithm>
#include <charconv>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <map>
#include <set>

#include "framework_i18n.h"

using Lingmo::framework::tr;
using Lingmo::i18n::format;

namespace Lingmo::Display {

namespace {

struct Size
{
    int width = 0;
    int height = 0;

    bool operator==(const Size&) const = default;
};

struct Rect
{
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;

    bool isEmpty() const { return width <= 0 || height <= 0; }
    // At least one pixel in common; an empty rectangle intersects nothing.
    bool intersects(const Rect& other) const
    {
        return !isEmpty() && !other.isEmpty() && x < other.x + other.width && other.x < x + width
               && y < other.y + other.height && other.y < y + height;
    }
};

} // namespace

// qBound's order: the minimum wins when the bounds cross.
static int bound(int minimum, int value, int maximum)
{
    return std::max(minimum, std::min(maximum, value));
}

OutputModel::OutputModel() = default;

std::string OutputModel::name(const Entry& entry) const
{
    const auto plainName = [](const Entry& candidate) -> std::string {
        if (candidate.physical.connectionType == 1)
            return tr("Built-in Display");
        // The compositor falls back to the connector when the EDID has no name.
        return candidate.physical.model.empty() ? candidate.physical.connectorName : candidate.physical.model;
    };
    const std::string plain = plainName(entry);
    for (const Entry& other : m_entries) {
        // Two of the same monitor are told apart by where they are plugged in.
        if (other.physical.sessionId != entry.physical.sessionId && plainName(other) == plain
            && !entry.physical.connectorName.empty() && plain != entry.physical.connectorName)
            // TRANSLATORS: a monitor and the port it is plugged into, such as "DELL U2720Q (DP-1)".
            return format(tr("{name} ({connector})"), {{"name", plain}, {"connector", entry.physical.connectorName}});
    }
    return plain;
}

const Mode* OutputModel::selectedMode(const Entry& entry) const
{
    return entry.modeIndex >= 0 && entry.modeIndex < int(entry.physical.modes.size())
               ? &entry.physical.modes[entry.modeIndex]
               : nullptr;
}

std::vector<int> OutputModel::resolutionModes(const Entry& entry) const
{
    std::vector<int> result;
    std::vector<Size> seen;
    for (int i = 0; i < int(entry.physical.modes.size()); ++i) {
        const Mode& mode = entry.physical.modes[i];
        const Size size{mode.width, mode.height};
        if (std::find(seen.begin(), seen.end(), size) == seen.end()) {
            seen.push_back(size);
            result.push_back(i);
        }
    }
    std::stable_sort(result.begin(), result.end(), [&entry](int a, int b) {
        const Mode& x = entry.physical.modes[a];
        const Mode& y = entry.physical.modes[b];
        return int64_t(x.width) * x.height > int64_t(y.width) * y.height;
    });
    return result;
}

std::vector<int> OutputModel::refreshModes(const Entry& entry) const
{
    std::vector<int> result;
    const Mode* selected = selectedMode(entry);
    if (!selected)
        return result;
    for (int i = 0; i < int(entry.physical.modes.size()); ++i) {
        const Mode& mode = entry.physical.modes[i];
        if (mode.width == selected->width && mode.height == selected->height)
            result.push_back(i);
    }
    std::stable_sort(result.begin(), result.end(), [&entry](int a, int b) {
        return entry.physical.modes[a].refreshMilliHz < entry.physical.modes[b].refreshMilliHz;
    });
    return result;
}

std::string OutputModel::name(int row) const
{
    return isRow(row) ? name(m_entries[row]) : std::string();
}

std::string OutputModel::id(int row) const
{
    return isRow(row) ? m_entries[row].physical.sessionId : std::string();
}

bool OutputModel::isEnabled(int row) const
{
    return isRow(row) && m_entries[row].enabled;
}

bool OutputModel::isInternal(int row) const
{
    return isRow(row) && m_entries[row].physical.connectionType == 1;
}

bool OutputModel::isPrimary(int row) const
{
    return isRow(row) && m_entries[row].primary;
}

OutputModel::Point OutputModel::position(int row) const
{
    return isRow(row) ? m_entries[row].position : Point();
}

int OutputModel::rotation(int row) const
{
    if (!isRow(row))
        return None;
    const uint32_t rotation = m_entries[row].rotation;
    return rotation == 0 ? None : rotation == 1 ? Left : rotation == 2 ? Inverted : Right;
}

double OutputModel::scale(int row) const
{
    return isRow(row) ? m_entries[row].scale120 / 120.0 : 1.0;
}

std::vector<std::string> OutputModel::resolutions(int row) const
{
    std::vector<std::string> result;
    if (!isRow(row))
        return result;
    const Entry& entry = m_entries[row];
    for (int index : resolutionModes(entry)) {
        const Mode& mode = entry.physical.modes[index];
        // TRANSLATORS: a screen resolution in pixels, such as "1920 × 1080".
        result.push_back(format(tr("{width} × {height}"),
                                {{"width", std::to_string(mode.width)}, {"height", std::to_string(mode.height)}}));
    }
    return result;
}

int OutputModel::resolutionIndex(int row) const
{
    if (!isRow(row))
        return 0;
    const Entry& entry = m_entries[row];
    const Mode* mode = selectedMode(entry);
    const std::vector<int> resolutions = resolutionModes(entry);
    for (int i = 0; i < int(resolutions.size()); ++i) {
        const Mode& candidate = entry.physical.modes[resolutions[i]];
        if (mode && candidate.width == mode->width && candidate.height == mode->height)
            return i;
    }
    return 0;
}

int OutputModel::recommendedResolutionIndex(int row) const
{
    if (!isRow(row))
        return -1;
    // The display's preferred mode, the one the compositor starts it in.
    const Entry& entry = m_entries[row];
    const std::vector<int> resolutions = resolutionModes(entry);
    for (int i = 0; i < int(resolutions.size()); ++i) {
        const Mode& mode = entry.physical.modes[resolutions[i]];
        for (const Mode& preferred : entry.physical.modes) {
            if (preferred.preferred && mode.width == preferred.width && mode.height == preferred.height)
                return i;
        }
    }
    return -1;
}

double OutputModel::recommendedScale(int row) const
{
    return isRow(row) ? m_entries[row].physical.recommendedScale120 / 120.0 : 1.0;
}

std::vector<std::string> OutputModel::refreshRates(int row) const
{
    std::vector<std::string> result;
    if (!isRow(row))
        return result;
    const Entry& entry = m_entries[row];
    for (int index : refreshModes(entry)) {
        char text[32];
        const auto end = std::to_chars(text, text + sizeof(text), entry.physical.modes[index].refreshMilliHz / 1000.0,
                                       std::chars_format::fixed, 2);
        // TRANSLATORS: a refresh rate, such as "60.00 Hz".
        result.push_back(format(tr("{rate} Hz"), {{"rate", std::string(text, end.ptr)}}));
    }
    return result;
}

int OutputModel::refreshRateIndex(int row) const
{
    if (!isRow(row))
        return -1;
    const std::vector<int> modes = refreshModes(m_entries[row]);
    const auto found = std::find(modes.begin(), modes.end(), m_entries[row].modeIndex);
    return found == modes.end() ? -1 : int(found - modes.begin());
}

std::vector<std::string> OutputModel::replicationSources(int row) const
{
    // TRANSLATORS: the choice that shows other content on this display instead of mirroring another one.
    std::vector<std::string> result{tr("Extend")};
    if (!isRow(row))
        return result;
    for (const Entry& other : m_entries) {
        if (other.physical.sessionId != m_entries[row].physical.sessionId)
            result.push_back(name(other));
    }
    return result;
}

int OutputModel::replicationSourceIndex(int row) const
{
    if (!isRow(row) || m_entries[row].mirrorSource.empty())
        return 0;
    const Entry& entry = m_entries[row];
    int index = 1;
    for (const Entry& other : m_entries) {
        if (other.physical.sessionId == entry.physical.sessionId)
            continue;
        if (other.physical.sessionId == entry.mirrorSource)
            return index;
        ++index;
    }
    return 0;
}

uint32_t OutputModel::capabilities(int row) const
{
    return isRow(row) ? m_entries[row].physical.capabilities : 0;
}

std::vector<double> OutputModel::supportedScales() const
{
    return {1.0, 1.25, 1.5, 1.75, 2.0, 2.5, 3.0, 4.0};
}

std::string OutputModel::drmConnector(int row) const
{
    if (!isRow(row))
        return {};
    const Entry& entry = m_entries[row];
    std::error_code error;
    const std::filesystem::path device = std::filesystem::canonical(entry.physical.devicePath, error);
    if (error || !device.string().starts_with("/dev/dri/card") || entry.physical.connectorName.empty())
        return {};
    return device.filename().string() + "-" + entry.physical.connectorName;
}

void OutputModel::markDirty()
{
    changed.emit();
    if (!m_dirty) {
        m_dirty = true;
        dirtyChanged.emit();
    }
}

bool OutputModel::setEnabled(int row, bool enabled)
{
    if (!isRow(row))
        return false;
    Entry& entry = m_entries[row];
    if (!enabled) {
        const auto count = std::count_if(m_entries.begin(), m_entries.end(), [](const Entry& e) { return e.enabled; });
        if (count <= 1)
            return false;
    }
    if (entry.enabled == enabled)
        return false;
    entry.enabled = enabled;
    m_layoutDirty = true;
    if (!entry.enabled) {
        // A display that showed this one's picture extends the desktop instead.
        for (Entry& mirror : m_entries) {
            if (mirror.mirrorSource == entry.physical.sessionId)
                mirror.mirrorSource.clear();
        }
    }
    if (!entry.enabled && entry.primary) {
        entry.primary = false;
        for (Entry& candidate : m_entries) {
            if (candidate.enabled && candidate.mirrorSource.empty()) {
                candidate.primary = true;
                break;
            }
        }
    }
    markDirty();
    return true;
}

bool OutputModel::setPrimary(int row)
{
    if (!isRow(row))
        return false;
    Entry& entry = m_entries[row];
    if (!entry.enabled || entry.primary)
        return false;
    for (Entry& other : m_entries)
        other.primary = false;
    entry.primary = true;
    markDirty();
    return true;
}

bool OutputModel::setPosition(int row, Point position)
{
    if (!isRow(row) || m_entries[row].position == position)
        return false;
    m_entries[row].position = position;
    m_layoutDirty = true;
    markDirty();
    return true;
}

bool OutputModel::setRotation(int row, int rotation)
{
    if (!isRow(row))
        return false;
    const uint32_t value = rotation == None       ? 0
                           : rotation == Left     ? 1
                           : rotation == Inverted ? 2
                           : rotation == Right    ? 3
                                                  : 99;
    if (value > 3 || m_entries[row].rotation == value)
        return false;
    m_entries[row].rotation = value;
    markDirty();
    return true;
}

bool OutputModel::setScale(int row, double scale)
{
    if (!isRow(row))
        return false;
    const int scale120 = int(std::lround(scale * 120));
    if (scale120 < 120 || scale120 > 480 || m_entries[row].scale120 == uint32_t(scale120))
        return false;
    m_entries[row].scale120 = uint32_t(scale120);
    markDirty();
    return true;
}

bool OutputModel::setResolutionIndex(int row, int index)
{
    if (!isRow(row))
        return false;
    Entry& entry = m_entries[row];
    const std::vector<int> choices = resolutionModes(entry);
    if (index < 0 || index >= int(choices.size()))
        return false;
    const Mode& target = entry.physical.modes[choices[index]];
    const Mode* current = selectedMode(entry);
    const int currentRefresh = current ? current->refreshMilliHz : 0;
    int best = choices[index];
    int difference = INT_MAX;
    for (int i = 0; i < int(entry.physical.modes.size()); ++i) {
        const Mode& candidate = entry.physical.modes[i];
        if (candidate.width == target.width && candidate.height == target.height) {
            const int distance = std::abs(candidate.refreshMilliHz - currentRefresh);
            if (distance < difference) {
                best = i;
                difference = distance;
            }
        }
    }
    if (entry.modeIndex == best)
        return false;
    entry.modeIndex = best;
    markDirty();
    return true;
}

bool OutputModel::setRefreshRateIndex(int row, int index)
{
    if (!isRow(row))
        return false;
    Entry& entry = m_entries[row];
    const std::vector<int> choices = refreshModes(entry);
    if (index < 0 || index >= int(choices.size()) || entry.modeIndex == choices[index])
        return false;
    entry.modeIndex = choices[index];
    markDirty();
    return true;
}

bool OutputModel::setReplicationSourceIndex(int row, int index)
{
    if (!isRow(row))
        return false;
    Entry& entry = m_entries[row];
    std::string source;
    int n = 1;
    for (const Entry& other : m_entries) {
        if (other.physical.sessionId == entry.physical.sessionId)
            continue;
        if (n++ == index) {
            source = other.physical.sessionId;
            break;
        }
    }
    if (entry.mirrorSource == source)
        return false;
    entry.mirrorSource = source;
    m_layoutDirty = true;
    if (!entry.mirrorSource.empty()) {
        // The displays that mirrored this one follow it to its source; a mirror
        // cannot be the source of another.
        for (Entry& mirror : m_entries) {
            if (mirror.mirrorSource == entry.physical.sessionId)
                mirror.mirrorSource = mirror.physical.sessionId == source ? std::string() : source;
        }
    }
    if (entry.primary && !entry.mirrorSource.empty()) {
        // The compositor takes the primary only from a mirror source.
        for (Entry& candidate : m_entries) {
            if (candidate.physical.sessionId == entry.mirrorSource && candidate.enabled
                && candidate.mirrorSource.empty()) {
                entry.primary = false;
                candidate.primary = true;
                break;
            }
        }
    }
    markDirty();
    return true;
}

void OutputModel::setState(const DisplayClient::State& state)
{
    m_applied = state;
    m_dirty = false;
    m_layoutDirty = false;
    std::set<std::string> connected;
    for (const PhysicalDisplay& physical : state.physical)
        connected.insert(physical.sessionId);
    const size_t oldCount = m_entries.size();
    std::erase_if(m_entries, [&connected](const Entry& entry) { return !connected.contains(entry.physical.sessionId); });
    bool rowsAdded = false;
    for (const PhysicalDisplay& physical : state.physical) {
        auto found = std::find_if(m_entries.begin(), m_entries.end(), [&physical](const Entry& entry) {
            return entry.physical.sessionId == physical.sessionId;
        });
        Entry entry = found == m_entries.end() ? Entry() : *found;
        entry.physical = physical;
        entry.enabled = false;
        entry.primary = false;
        entry.mirrorSource.clear();
        for (int i = 0; i < int(physical.modes.size()); ++i) {
            if (physical.modes[i].current) {
                entry.modeIndex = i;
                break;
            }
        }
        for (const LogicalDisplay& logical : state.logical) {
            if (std::find(logical.destinations.begin(), logical.destinations.end(), physical.sessionId)
                == logical.destinations.end())
                continue;
            entry.enabled = true;
            entry.primary = logical.primary && logical.destinations.front() == physical.sessionId;
            entry.position = {logical.x, logical.y};
            entry.scale120 = logical.scale120;
            entry.rotation = logical.rotation;
            if (logical.destinations.front() != physical.sessionId)
                entry.mirrorSource = logical.destinations.front();
            break;
        }
        if (found == m_entries.end()) {
            m_entries.push_back(std::move(entry));
            rowsAdded = true;
        } else {
            *found = std::move(entry);
        }
    }
    if (rowsAdded || m_entries.size() != oldCount)
        rowsChanged.emit();
    changed.emit();
    dirtyChanged.emit();
}

void OutputModel::discard()
{
    const DisplayClient::State applied = m_applied;
    setState(applied);
}

bool OutputModel::needsConfirmation() const
{
    for (const Entry& entry : m_entries) {
        const Mode* mode = selectedMode(entry);
        if (!entry.enabled || !mode || mode->current)
            continue;
        // A display being switched on has no current mode to leave behind.
        for (const Mode& applied : entry.physical.modes) {
            if (applied.current)
                return true;
        }
    }
    return false;
}

std::vector<DisplayRequest> OutputModel::requests() const
{
    std::vector<DisplayRequest> result;
    for (const Entry& entry : m_entries) {
        const Mode* mode = selectedMode(entry);
        if (!mode)
            continue;
        result.push_back({entry.physical.sessionId,
                          entry.enabled,
                          {mode->width, mode->height, mode->refreshMilliHz, mode->pixelClockKHz,
                           mode->horizontalTotal, mode->verticalTotal, mode->flags, mode->preferred, mode->name},
                          entry.scale120,
                          entry.rotation});
    }
    return result;
}

std::string OutputModel::primary() const
{
    for (const Entry& entry : m_entries) {
        if (entry.primary)
            return entry.physical.sessionId;
    }
    return {};
}

std::vector<Placement> OutputModel::placements() const
{
    if (!m_layoutDirty)
        return m_applied.placements;
    std::vector<Placement> result;
    const std::string root = primary();
    const Entry* parent = nullptr;
    for (const Entry& entry : m_entries) {
        if (entry.physical.sessionId == root)
            parent = &entry;
    }
    if (!parent)
        return result;
    std::vector<const Entry*> placed{parent};
    const auto logicalSize = [this](const Entry& entry) {
        const Mode* mode = selectedMode(entry);
        if (!mode)
            return Size{1, 1};
        Size size{mode->width, mode->height};
        if (entry.rotation % 2)
            std::swap(size.width, size.height);
        const int scale = int(entry.scale120);
        return Size{(size.width * 120 + scale - 1) / scale, (size.height * 120 + scale - 1) / scale};
    };
    std::map<std::string, Rect> resolved;
    const Size rootSize = logicalSize(*parent);
    resolved[parent->physical.sessionId] = {parent->position.x, parent->position.y, rootSize.width, rootSize.height};
    for (const Entry& entry : m_entries) {
        if (!entry.enabled || entry.physical.sessionId == root || !entry.mirrorSource.empty())
            continue;
        int bestDistance = INT_MAX;
        Placement best;
        Rect bestBounds;
        const Size childSize = logicalSize(entry);
        for (const Entry* candidate : placed) {
            const Size parentSize = logicalSize(*candidate);
            const Rect& parentRect = resolved[candidate->physical.sessionId];
            const Point delta{entry.position.x - parentRect.x, entry.position.y - parentRect.y};
            for (uint32_t edge = 0; edge < 4; ++edge) {
                const bool horizontal = edge % 2;
                const int childExtent = horizontal ? childSize.height : childSize.width;
                const int parentExtent = horizontal ? parentSize.height : parentSize.width;
                const int overlap = std::min(32, std::min(childExtent, parentExtent));
                const int cross = horizontal ? delta.y : delta.x;
                const int offset = bound(-childExtent + overlap, cross, parentExtent - overlap);
                const int normal = edge == 0   ? -childSize.height
                                   : edge == 1 ? parentSize.width
                                   : edge == 2 ? parentSize.height
                                               : -childSize.width;
                const int distance = std::abs((horizontal ? delta.x : delta.y) - normal) + std::abs(cross - offset);
                const Rect bounds{parentRect.x + (horizontal ? normal : offset),
                                  parentRect.y + (horizontal ? offset : normal), childSize.width, childSize.height};
                bool overlaps = false;
                for (const auto& [id, occupied] : resolved)
                    overlaps |= occupied.intersects(bounds);
                if (overlaps)
                    continue;
                if (distance < bestDistance) {
                    bestDistance = distance;
                    best = {entry.physical.sessionId, candidate->physical.sessionId, edge, offset};
                    bestBounds = bounds;
                }
            }
        }
        result.push_back(best);
        placed.push_back(&entry);
        resolved[entry.physical.sessionId] = bestBounds;
    }
    return result;
}

MirrorGroups OutputModel::mirrorGroups() const
{
    MirrorGroups result;
    for (const Entry& entry : m_entries) {
        if (!entry.enabled || entry.mirrorSource.empty())
            continue;
        auto group = std::find_if(result.begin(), result.end(), [&entry](const std::vector<std::string>& members) {
            return members.front() == entry.mirrorSource;
        });
        if (group == result.end())
            result.push_back({entry.mirrorSource, entry.physical.sessionId});
        else
            group->push_back(entry.physical.sessionId);
    }
    return result;
}

} // namespace Lingmo::Display
