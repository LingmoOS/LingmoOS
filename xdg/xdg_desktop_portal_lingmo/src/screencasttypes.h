#pragma once

#include "base/dbus_types.h"

#include <cstdint>
#include <string>
#include <tuple>

// (ii): stream position and size in the compositor coordinate space.
using IntPair = std::tuple<int32_t, int32_t>;

// (ua{sv}): a PipeWire node id and its stream properties.
using StreamInfo = std::tuple<uint32_t, Lingmo::base::dbus::VariantMap>;

// (suv): vendor, format version and private data of a restorable selection.
using RestoreData = std::tuple<std::string, uint32_t, Lingmo::base::dbus::Variant>;
