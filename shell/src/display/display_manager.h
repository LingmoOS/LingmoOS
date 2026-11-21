// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/display/manager/display_manager.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: one model owns immutable hardware snapshots
// and applied logical display batches without Ash observers or preferences.

#pragma once

#include <functional>
#include <optional>

#include "base/geometry.h"
#include "base/signal.h"

#include "display/display_transform.h"
#include "display/display_types.h"
#include "display/mirror_transform.h"

namespace Lingmo::Display {

class DisplayManager
{
public:
    DisplayManager() = default;
    DisplayManager(const DisplayManager &) = delete;
    DisplayManager &operator=(const DisplayManager &) = delete;

    const std::optional<HardwareSnapshot> &hardware() const { return m_hardware; }

    const std::optional<AppliedConfiguration> &applied() const { return m_applied; }

    // The runtime publisher completes consumer updates before public observers run.
    void setPublisher(std::function<void()> publisher) { m_publisher = std::move(publisher); }

    bool isPublishing() const { return m_publishing; }

    static base::PointF globalToSurfaceLocal(const base::PointF &point,
                                             const base::PointF &surfaceOrigin)
    {
        return point - surfaceOrigin;
    }

    static base::Rect surfaceLocalToGlobal(const base::Rect &rect, const base::Point &surfaceOrigin)
    {
        return rect.translated(surfaceOrigin);
    }

    base::PointF normalizedToGlobal(const base::PointF &point) const;
    // A point on the touchscreen, 0..1 across its panel in the panel's own
    // orientation, on the desktop. Null when no display shows it.
    std::optional<base::PointF> touchscreenToGlobal(const base::PointF &normalized) const;
    std::optional<base::PointF> globalToOutputLocal(const DisplayId &id,
                                                    const base::PointF &point) const;
    std::optional<base::PointF> outputLocalToGlobal(const DisplayId &id,
                                                    const base::PointF &point) const;

    bool installHardware(HardwareSnapshot snapshot, DisplayError *error = nullptr);
    bool installApplied(AppliedConfiguration configuration, DisplayError *error = nullptr);
    void clearApplied(uint64_t topologyGeneration);

    const LogicalDisplay *display(const DisplayId &id) const;
    const LogicalDisplay *primaryDisplay() const;
    const LogicalDisplay *displayAt(const base::PointF &point) const;
    const LogicalDisplay *nearestDisplay(const base::PointF &point) const;
    const LogicalDisplay *displayForRect(const base::Rect &rect) const;
    base::Rect desktopBounds() const;

    struct DestinationMapping {
        base::Rect logicalBounds;
        base::Rect nativeContent;
        double renderScale;
        DisplayRotation rotation;

        MirrorTransform transform() const { return {logicalBounds, nativeContent, rotation}; }
    };

    std::optional<DestinationMapping> mappingFor(const DisplayId &destination) const;
    std::optional<DisplayTransform> transformFor(const DisplayId &id) const;

    base::Signal<> displayChangeStarted;
    // The topology generation and the applied revision.
    base::Signal<uint64_t, uint64_t> displaysChanged;

private:
    const DisplaySnapshot *physicalDisplay(const DisplayId &id) const;

    bool m_publishing = false;
    std::function<void()> m_publisher;
    std::optional<HardwareSnapshot> m_hardware;
    std::optional<AppliedConfiguration> m_applied;
};

} // namespace Lingmo::Display
