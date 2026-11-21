// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Upstream: chromium/src/ash/display/cros_display_config.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Cutefish: stable-ID draft rows expose native modes, layout,
// primary selection, scale, rotation, and software mirror requests.

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "base/signal.h"
#include "lingmo/display/display_client.h"

namespace Lingmo::Display {

// The displays as the user edits them: one row per physical display, kept
// across state refreshes by its session id, and the requests that apply the
// edits.
class OutputModel
{
public:
    // As the rotation combo box lists them.
    enum Rotation {
        None = 1,
        Left = 2,
        Inverted = 4,
        Right = 8,
    };

    struct Point
    {
        int x = 0;
        int y = 0;

        bool operator==(const Point&) const = default;
    };

    OutputModel();

    int count() const { return int(m_entries.size()); }

    // For people: "Built-in Display", the monitor's model, or its connector.
    std::string name(int row) const;
    std::string id(int row) const;
    bool isEnabled(int row) const;
    bool isInternal(int row) const;
    bool isPrimary(int row) const;
    Point position(int row) const;
    // A Rotation.
    int rotation(int row) const;
    double scale(int row) const;
    // Resolutions, largest first, as "2160 × 1440".
    std::vector<std::string> resolutions(int row) const;
    int resolutionIndex(int row) const;
    // The display's preferred mode's place among the resolutions; -1 without one.
    int recommendedResolutionIndex(int row) const;
    double recommendedScale(int row) const;
    // The current resolution's refresh rates, slowest first, as "60.00 Hz".
    std::vector<std::string> refreshRates(int row) const;
    // -1 when the selected mode is not among them.
    int refreshRateIndex(int row) const;
    // "Extend", then every other display to mirror.
    std::vector<std::string> replicationSources(int row) const;
    int replicationSourceIndex(int row) const;
    uint32_t capabilities(int row) const;
    std::vector<double> supportedScales() const;
    // The sysfs connector name, e.g. "card0-eDP-1"; empty when not known.
    std::string drmConnector(int row) const;

    // Each returns false when the value is refused or does not change anything.
    bool setEnabled(int row, bool enabled);
    // A display only becomes primary; another one taking over ends it.
    bool setPrimary(int row);
    bool setPosition(int row, Point position);
    bool setRotation(int row, int rotation);
    bool setScale(int row, double scale);
    bool setResolutionIndex(int row, int index);
    bool setRefreshRateIndex(int row, int index);
    bool setReplicationSourceIndex(int row, int index);

    void setState(const DisplayClient::State& state);
    void discard();

    bool dirty() const { return m_dirty; }

    // A display switched to another mode may show nothing at all, so that
    // draft is previewed and kept only on confirmation.
    bool needsConfirmation() const;

    std::vector<DisplayRequest> requests() const;
    std::vector<Placement> placements() const;
    MirrorGroups mirrorGroups() const;
    std::string primary() const;

    // A row came or went.
    base::Signal<> rowsChanged;
    // Values of the rows changed.
    base::Signal<> changed;
    base::Signal<> dirtyChanged;

private:
    struct Entry
    {
        PhysicalDisplay physical;
        bool enabled = false;
        bool primary = false;
        Point position;
        uint32_t scale120 = 120;
        uint32_t rotation = 0;
        int modeIndex = 0;
        std::string mirrorSource;
    };

    bool isRow(int row) const { return row >= 0 && row < count(); }
    std::string name(const Entry& entry) const;
    const Mode* selectedMode(const Entry& entry) const;
    std::vector<int> resolutionModes(const Entry& entry) const;
    std::vector<int> refreshModes(const Entry& entry) const;
    void markDirty();

    DisplayClient::State m_applied;
    std::vector<Entry> m_entries;
    bool m_dirty = false;
    bool m_layoutDirty = false;
};

} // namespace Lingmo::Display
