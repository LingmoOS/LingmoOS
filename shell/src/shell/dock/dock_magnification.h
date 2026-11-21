// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

#include "ui/frame_client.h"

namespace Lingmo::Shell {

// The dock's hover magnification, as macOS has it: an item grows as the pointer
// comes near it, its neighbours grow less the further they are, and the row
// spreads so the items never overlap, while the item under the pointer keeps
// its place. The values are smoothed on the window's frames, quickly after the
// pointer and slowly back to rest, so entering and leaving the dock does not
// jump. The DockWindow draws with what placements() gives and asks for the
// items again whenever its rest layout changes.
class DockMagnification : public ui::FrameClient
{
public:
    // An item where it rests, along the dock.
    struct Item
    {
        // The centre of the icon, in the layout's content coordinates.
        double centre = 0;
        // The icon's width at rest.
        double width = 0;

        bool operator==(const Item &) const = default;
    };

    // How the item is drawn now: scaled about its centre and moved along.
    struct Placement
    {
        double scale = 1;
        double shift = 0;
    };

    // How far an item feels the pointer, in slots (the dock's pitch).
    static constexpr double kReachSlots = 2.5;
    // Seconds to follow the pointer, and to come back to rest.
    static constexpr double kFollowTau = 0.045;
    static constexpr double kReturnTau = 0.09;
    // Below this the placement is taken to be at its target.
    static constexpr double kSettle = 5e-4;
    // The frame interval a transition is backdated by, so its first frame
    // already shows one step of motion.
    static constexpr double kFrameIntervalMs = 1000.0 / 60;

    explicit DockMagnification(ui::Window *window);
    ~DockMagnification() override;

    void setEnabled(bool enabled);
    bool enabled() const { return m_enabled; }
    // The largest scale: 1 for no magnification.
    void setFactor(double factor);
    double factor() const { return m_factor; }

    // The pointer's place along the dock, in the layout's content coordinates;
    // nullopt when it is not on the dock.
    void setPointer(std::optional<double> along);
    std::optional<double> pointer() const { return m_pointer; }

    // The rest layout, in any storage order, and the distance between two
    // items at rest. Placements keep the supplied order as positions cross.
    void setItems(std::vector<Item> items, double pitch);

    // What to draw with, in the order of the last setItems().
    const std::vector<Placement> &placements() const { return m_placements; }
    // Where the placements are heading, for tests.
    std::vector<Placement> targets() const;
    bool isRunning() const { return isTicking(); }

    std::function<void()> onChanged;

protected:
    void tick(uint64_t frameTimeNs) override;

private:
    std::vector<Placement> computeTargets() const;
    // Brings a changed item count in line, new items already at their target.
    void resizePlacements(const std::vector<Placement> &target);

    bool m_enabled = true;
    double m_factor = 1.5;
    std::optional<double> m_pointer;
    std::vector<Item> m_items;
    double m_pitch = 70;
    std::vector<Placement> m_placements;
    uint64_t m_lastFrameNs = 0;
};

} // namespace Lingmo::Shell
