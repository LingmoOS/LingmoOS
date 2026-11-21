// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/display/cros_display_config.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: fixed D-Bus tuples replace Ash service and Mojo
// types while preserving capability-rich display descriptions.

#pragma once

#include <cstdint>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include "base/dbus_types.h"
#include "display/display_types.h"

namespace Lingmo::Display::ipc {

// (iiiiiiubbs)
struct Mode {
    int32_t width = 0;
    int32_t height = 0;
    int32_t refreshMilliHz = 0;
    int32_t pixelClockKHz = 0;
    int32_t horizontalTotal = 0;
    int32_t verticalTotal = 0;
    uint32_t flags = 0;
    bool preferred = false;
    bool current = false;
    std::string name;

    auto tie()
    {
        return std::tie(width, height, refreshMilliHz, pixelClockKHz, horizontalTotal,
                        verticalTotal, flags, preferred, current, name);
    }
};

// (iiiiiiubs)
struct RequestedMode {
    int32_t width = 0;
    int32_t height = 0;
    int32_t refreshMilliHz = 0;
    int32_t pixelClockKHz = 0;
    int32_t horizontalTotal = 0;
    int32_t verticalTotal = 0;
    uint32_t flags = 0;
    bool preferred = false;
    std::string name;

    auto tie()
    {
        return std::tie(width, height, refreshMilliHz, pixelClockKHz, horizontalTotal,
                        verticalTotal, flags, preferred, name);
    }
};

// (sssssssasssiiuuua(iiiiiiubbs))
struct PhysicalDisplay {
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
    int32_t physicalWidthMm = 0;
    int32_t physicalHeightMm = 0;
    uint32_t connectionType = 0;
    uint32_t capabilities = 0;
    uint32_t recommendedScale120 = 120;
    std::vector<Mode> modes;

    auto tie()
    {
        return std::tie(sessionId, persistentKey, portKey, edidKey, connectorName, devicePath,
                        baseConnectorKey, pathTopology, make, model, physicalWidthMm,
                        physicalHeightMm, connectionType, capabilities,
                        recommendedScale120, modes);
    }
};

// (ssasiiiiiiiiuub)
struct LogicalDisplay {
    std::string id;
    std::string waylandName;
    std::vector<std::string> destinations;
    int32_t x = 0;
    int32_t y = 0;
    int32_t width = 0;
    int32_t height = 0;
    int32_t workX = 0;
    int32_t workY = 0;
    int32_t workWidth = 0;
    int32_t workHeight = 0;
    uint32_t scale120 = 120;
    uint32_t rotation = 0;
    bool primary = false;

    auto tie()
    {
        return std::tie(id, waylandName, destinations, x, y, width, height, workX, workY,
                        workWidth, workHeight, scale120, rotation, primary);
    }
};

// (sb(iiiiiiubs)uu)
struct DisplayRequest {
    std::string id;
    bool enabled = true;
    RequestedMode mode;
    uint32_t scale120 = 120;
    uint32_t rotation = 0;

    auto tie() { return std::tie(id, enabled, mode, scale120, rotation); }
};

// (ssui)
struct Placement {
    std::string displayId;
    std::string parentId;
    uint32_t edge = 0;
    int32_t offset = 0;

    auto tie() { return std::tie(displayId, parentId, edge, offset); }
};

using MirrorGroups = std::vector<std::vector<std::string>>;

DisplayConfiguration fromWire(const std::string& instanceId, uint64_t topologyGeneration,
                              uint64_t appliedRevision, const std::vector<DisplayRequest>& displays,
                              const std::string& primary, const std::vector<Placement>& placements,
                              const MirrorGroups& mirrorGroups);
std::vector<PhysicalDisplay> toWire(const std::vector<DisplaySnapshot>& displays);
std::vector<LogicalDisplay> toWire(const std::vector<Lingmo::Display::LogicalDisplay>& displays);
std::vector<Placement> toWire(const std::vector<DisplayPlacement>& placements);

} // namespace Lingmo::Display::ipc

namespace Lingmo::base::dbus {

// A struct whose tie() lists its members in the order of its D-Bus tuple.
template<typename T>
struct TiedDisplayType
{
    template<typename Tie>
    struct Values;
    template<typename... Ts>
    struct Values<std::tuple<Ts...>>
    {
        using Tuple = std::tuple<std::remove_cvref_t<Ts>...>;
    };
    using Tuple = typename Values<decltype(std::declval<T&>().tie())>::Tuple;

    static std::string signature() { return Type<Tuple>::signature(); }
    // tie() only reads here.
    static Variant toVariant(const T& value)
    {
        return Type<Tuple>::toVariant(Tuple(const_cast<T&>(value).tie()));
    }
    static bool fromVariant(const Variant& variant, T& out)
    {
        Tuple values;
        if (!Type<Tuple>::fromVariant(variant, values))
            return false;
        out.tie() = std::move(values);
        return true;
    }
};

template<> struct Type<Lingmo::Display::ipc::Mode> : TiedDisplayType<Lingmo::Display::ipc::Mode> {};
template<> struct Type<Lingmo::Display::ipc::RequestedMode> : TiedDisplayType<Lingmo::Display::ipc::RequestedMode> {};
template<> struct Type<Lingmo::Display::ipc::PhysicalDisplay> : TiedDisplayType<Lingmo::Display::ipc::PhysicalDisplay> {};
template<> struct Type<Lingmo::Display::ipc::LogicalDisplay> : TiedDisplayType<Lingmo::Display::ipc::LogicalDisplay> {};
template<> struct Type<Lingmo::Display::ipc::DisplayRequest> : TiedDisplayType<Lingmo::Display::ipc::DisplayRequest> {};
template<> struct Type<Lingmo::Display::ipc::Placement> : TiedDisplayType<Lingmo::Display::ipc::Placement> {};

} // namespace Lingmo::Base::DBus
