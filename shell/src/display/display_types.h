// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/display/types/display_snapshot.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: hardware, requested, applied, and persistent-facing
// identities are separate value types with no Chromium platform graph.

#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "base/geometry.h"
#include "compositor/output_scale.h"

namespace Lingmo::Display {

class DisplayId
{
public:
    DisplayId() = default;

    explicit DisplayId(std::string value)
        : m_value(std::move(value))
    {
    }

    bool isValid() const { return !m_value.empty(); }

    const std::string &value() const { return m_value; }

    friend bool operator==(const DisplayId &, const DisplayId &) = default;

    friend bool operator<(const DisplayId &left, const DisplayId &right)
    {
        return left.m_value < right.m_value;
    }

private:
    std::string m_value;
};


enum class DisplayRotation : uint32_t {
    Normal = 0,
    Rotate90 = 1,
    Rotate180 = 2,
    Rotate270 = 3,
};

// D-Bus and the preferences file carry these as plain integers.
inline bool isValid(DisplayRotation rotation)
{
    return uint32_t(rotation) <= uint32_t(DisplayRotation::Rotate270);
}

enum class DisplayConnectionType : uint32_t {
    Unknown,
    Internal,
    External,
    Virtual,
};

enum class DisplayCapability : uint32_t {
    None = 0,
    Scale = 1 << 0,
    Rotation = 1 << 1,
    SoftwareMirror = 1 << 2,
    Power = 1 << 3,
    VariableRefreshRate = 1 << 4,
    HighDynamicRange = 1 << 5,
    PrivacyScreen = 1 << 6,
    // The night light can tint it.
    ColorTransform = 1 << 7,
};

class DisplayCapabilities
{
public:
    constexpr DisplayCapabilities() = default;

    constexpr DisplayCapabilities(DisplayCapability capability)
        : m_value(uint32_t(capability))
    {
    }

    constexpr bool testFlag(DisplayCapability capability) const
    {
        const uint32_t bits = uint32_t(capability);
        return bits == 0 ? m_value == 0 : (m_value & bits) == bits;
    }

    constexpr uint32_t toInt() const { return m_value; }

    constexpr DisplayCapabilities &operator|=(DisplayCapabilities other)
    {
        m_value |= other.m_value;
        return *this;
    }

    friend constexpr DisplayCapabilities operator|(DisplayCapabilities left,
                                                   DisplayCapabilities right)
    {
        return left |= right;
    }

    friend constexpr bool operator==(DisplayCapabilities, DisplayCapabilities) = default;

private:
    uint32_t m_value = 0;
};

constexpr DisplayCapabilities operator|(DisplayCapability left, DisplayCapability right)
{
    return DisplayCapabilities(left) | right;
}

struct DisplayMode {
    // Invalid until set, as a mode without a size has always been.
    base::Size pixelSize {-1, -1};
    int refreshMilliHz = 0;
    int pixelClockKHz = 0;
    int horizontalTotal = 0;
    int verticalTotal = 0;
    uint32_t flags = 0;
    std::string name;
    bool preferred = false;

    bool isValid() const;
    std::string stableKey() const;
    friend bool operator==(const DisplayMode &, const DisplayMode &) = default;
};

struct DisplaySnapshot {
    DisplayId sessionId;
    std::string persistentKey;
    std::string portKey;
    std::string edidKey;
    std::string connectorName;
    std::string devicePath;
    std::string baseConnectorKey;
    std::vector<std::string> pathTopology;
    std::string make;
    std::string model;
    // Negative while unknown, which makes the snapshot invalid.
    base::Size physicalSizeMm {-1, -1};
    DisplayConnectionType connectionType = DisplayConnectionType::Unknown;
    DisplayCapabilities capabilities;
    std::vector<DisplayMode> modes;
    std::optional<DisplayMode> currentMode;
    std::optional<DisplayMode> nativeMode;

    bool isValid() const;
};

struct HardwareSnapshot {
    std::string compositorInstanceId;
    std::string seatId;
    uint64_t topologyGeneration = 0;
    std::vector<DisplaySnapshot> displays;

    bool isValid() const;
};

enum class LayoutEdge : uint32_t {
    Top,
    Right,
    Bottom,
    Left,
};

inline bool isValid(LayoutEdge edge)
{
    return uint32_t(edge) <= uint32_t(LayoutEdge::Left);
}

struct DisplayPlacement {
    DisplayId display;
    DisplayId parent;
    LayoutEdge edge = LayoutEdge::Right;
    int offset = 0;

    friend bool operator==(const DisplayPlacement &, const DisplayPlacement &) = default;
};

struct DisplayConfigurationBase {
    std::string compositorInstanceId;
    uint64_t topologyGeneration = 0;
    uint64_t appliedRevision = 0;
};

struct DisplayRequest {
    DisplayId display;
    bool enabled = true;
    DisplayMode mode;
    compositor::OutputScale scale;
    DisplayRotation rotation = DisplayRotation::Normal;
};

struct DisplayConfiguration {
    DisplayConfigurationBase base;
    std::vector<DisplayRequest> displays;
    DisplayId primary;
    std::vector<DisplayPlacement> placements;
    std::vector<std::vector<DisplayId>> mirrorGroups;
    // Internal recovery policy; never accepted from IPC or persisted.
    bool allowModeFallback = false;
};

struct LogicalDisplay {
    DisplayId id;
    std::vector<DisplayId> destinations;
    std::string waylandName;
    base::Rect bounds;
    base::Rect workArea;
    compositor::OutputScale scale;
    DisplayRotation rotation = DisplayRotation::Normal;
    bool primary = false;

    bool hasDestination(const DisplayId &destination) const
    {
        return std::find(destinations.begin(), destinations.end(), destination)
                != destinations.end();
    }
};

struct AppliedConfiguration {
    std::string compositorInstanceId;
    uint64_t topologyGeneration = 0;
    uint64_t revision = 0;
    std::vector<DisplaySnapshot> physicalDisplays;
    std::vector<LogicalDisplay> logicalDisplays;
    std::vector<DisplayPlacement> placements;
};

enum class DisplayErrorCode : uint32_t {
    None,
    Unavailable,
    Unauthorized,
    Busy,
    StaleConfiguration,
    UnknownOutput,
    UnsupportedMode,
    UnsupportedProperty,
    InvalidLayout,
    NoEnabledOutput,
    ResourceExhausted,
    TestFailed,
    CommitFailed,
    Cancelled,
    RevertFailed,
    PersistenceFailed,
};

struct DisplayError {
    DisplayErrorCode code = DisplayErrorCode::None;
    std::string message;
    std::vector<DisplayId> affectedDisplays;
};

struct ConfigurationResult {
    bool succeeded = false;
    bool degraded = false;
    DisplayError error;
    std::optional<AppliedConfiguration> applied;
    // The successful test candidate is committed exactly, then used for publication.
    std::optional<DisplayConfiguration> selected;
};

} // namespace Lingmo::Display

template<>
struct std::hash<Lingmo::Display::DisplayId>
{
    size_t operator()(const Lingmo::Display::DisplayId &id) const noexcept
    {
        return std::hash<std::string>()(id.value());
    }
};
