// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Upstream: chromium/src/ash/display/cros_display_config.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Cutefish: plain value types for the compositor's versioned
// session D-Bus tuples, without Ash or Mojo.

#pragma once

#include <cstdint>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "base/dbus_types.h"

namespace Lingmo::Display {

// OperationFinished's errorCode, in the compositor's DisplayErrorCode order.
enum class ErrorCode : uint32_t {
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

struct Mode
{
    int width = 0;
    int height = 0;
    int refreshMilliHz = 0;
    int pixelClockKHz = 0;
    int horizontalTotal = 0;
    int verticalTotal = 0;
    uint32_t flags = 0;
    bool preferred = false;
    bool current = false;
    std::string name;

    auto tie() { return std::tie(width, height, refreshMilliHz, pixelClockKHz, horizontalTotal, verticalTotal, flags,
                                 preferred, current, name); }
};

struct RequestedMode
{
    int width = 0;
    int height = 0;
    int refreshMilliHz = 0;
    int pixelClockKHz = 0;
    int horizontalTotal = 0;
    int verticalTotal = 0;
    uint32_t flags = 0;
    bool preferred = false;
    std::string name;

    auto tie() { return std::tie(width, height, refreshMilliHz, pixelClockKHz, horizontalTotal, verticalTotal, flags,
                                 preferred, name); }
};

struct PhysicalDisplay
{
    std::string sessionId;
    std::string persistentKey;
    std::string portKey;
    std::string edidKey;
    std::string connectorName;
    std::string devicePath;
    std::string baseConnectorKey;
    std::vector<std::string> pathTopology;
    std::string make;
    std::string model;
    int physicalWidthMm = 0;
    int physicalHeightMm = 0;
    uint32_t connectionType = 0;
    uint32_t capabilities = 0;
    uint32_t recommendedScale120 = 120;
    std::vector<Mode> modes;

    auto tie() { return std::tie(sessionId, persistentKey, portKey, edidKey, connectorName, devicePath,
                                 baseConnectorKey, pathTopology, make, model, physicalWidthMm,
                                 physicalHeightMm, connectionType, capabilities,
                                 recommendedScale120, modes); }
};

// The capabilities bit for a display the night light can tint, the
// compositor's DisplayCapability::ColorTransform.
inline constexpr uint32_t ColorTransformCapability = 1u << 7;

struct LogicalDisplay
{
    std::string id;
    std::string waylandName;
    std::vector<std::string> destinations;
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    int workX = 0;
    int workY = 0;
    int workWidth = 0;
    int workHeight = 0;
    uint32_t scale120 = 120;
    uint32_t rotation = 0;
    bool primary = false;

    auto tie() { return std::tie(id, waylandName, destinations, x, y, width, height, workX, workY, workWidth,
                                 workHeight, scale120, rotation, primary); }
};

struct DisplayRequest
{
    std::string id;
    bool enabled = true;
    RequestedMode mode;
    uint32_t scale120 = 120;
    uint32_t rotation = 0;

    auto tie() { return std::tie(id, enabled, mode, scale120, rotation); }
};

struct Placement
{
    std::string displayId;
    std::string parentId;
    uint32_t edge = 0;
    int offset = 0;

    auto tie() { return std::tie(displayId, parentId, edge, offset); }
};

using MirrorGroups = std::vector<std::vector<std::string>>;

} // namespace Lingmo::Display

namespace Lingmo::base::dbus {

template<typename Tie>
struct TiedValues;
template<typename... Ts>
struct TiedValues<std::tuple<Ts...>>
{
    using Tuple = std::tuple<std::remove_cvref_t<Ts>...>;
};

// A struct whose tie() lists its members in the order of its D-Bus tuple.
template<typename T>
struct TiedType
{
    using Tuple = typename TiedValues<decltype(std::declval<T&>().tie())>::Tuple;

    static std::string signature() { return Type<Tuple>::signature(); }
    // tie() only reads here.
    static Variant toVariant(const T& value) { return Type<Tuple>::toVariant(Tuple(const_cast<T&>(value).tie())); }
    static bool fromVariant(const Variant& variant, T& out)
    {
        Tuple values;
        if (!Type<Tuple>::fromVariant(variant, values))
            return false;
        out.tie() = std::move(values);
        return true;
    }
};

template<> struct Type<Lingmo::Display::Mode> : TiedType<Lingmo::Display::Mode> {};
template<> struct Type<Lingmo::Display::RequestedMode> : TiedType<Lingmo::Display::RequestedMode> {};
template<> struct Type<Lingmo::Display::PhysicalDisplay> : TiedType<Lingmo::Display::PhysicalDisplay> {};
template<> struct Type<Lingmo::Display::LogicalDisplay> : TiedType<Lingmo::Display::LogicalDisplay> {};
template<> struct Type<Lingmo::Display::DisplayRequest> : TiedType<Lingmo::Display::DisplayRequest> {};
template<> struct Type<Lingmo::Display::Placement> : TiedType<Lingmo::Display::Placement> {};

} // namespace Lingmo::Base::DBus
