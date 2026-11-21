// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktop/widgets/widgetconfig.h"

#include <cmath>

#include "base/json.h"

using Lingmo::base::json::Value;

namespace fm {

const char* widgetKindKey(WidgetKind kind)
{
    switch (kind) {
    case WidgetKind::Clock:
        return "clock";
    case WidgetKind::Calendar:
        return "calendar";
    case WidgetKind::System:
        return "system";
    case WidgetKind::Battery:
        return "battery";
    case WidgetKind::NowPlaying:
        return "nowplaying";
    case WidgetKind::Notes:
        return "notes";
    case WidgetKind::Weather:
        return "weather";
    case WidgetKind::WorldClock:
        return "worldclock";
    case WidgetKind::Countdown:
        return "countdown";
    case WidgetKind::NetworkSpeed:
        return "networkspeed";
    case WidgetKind::Todo:
        return "todo";
    case WidgetKind::Launcher:
        return "launcher";
    }
    return "clock";
}

const char* widgetSizeKey(WidgetSize size)
{
    switch (size) {
    case WidgetSize::Small:
        return "small";
    case WidgetSize::Medium:
        return "medium";
    case WidgetSize::Large:
        return "large";
    }
    return "medium";
}

bool widgetKindFromKey(std::string_view key, WidgetKind& kind)
{
    if (key == "clock")
        kind = WidgetKind::Clock;
    else if (key == "calendar")
        kind = WidgetKind::Calendar;
    else if (key == "system")
        kind = WidgetKind::System;
    else if (key == "battery")
        kind = WidgetKind::Battery;
    else if (key == "nowplaying")
        kind = WidgetKind::NowPlaying;
    else if (key == "notes")
        kind = WidgetKind::Notes;
    else if (key == "weather")
        kind = WidgetKind::Weather;
    else if (key == "worldclock")
        kind = WidgetKind::WorldClock;
    else if (key == "countdown")
        kind = WidgetKind::Countdown;
    else if (key == "networkspeed")
        kind = WidgetKind::NetworkSpeed;
    else if (key == "todo")
        kind = WidgetKind::Todo;
    else if (key == "launcher")
        kind = WidgetKind::Launcher;
    else
        return false;
    return true;
}

bool widgetSizeFromKey(std::string_view key, WidgetSize& size)
{
    if (key == "small")
        size = WidgetSize::Small;
    else if (key == "medium")
        size = WidgetSize::Medium;
    else if (key == "large")
        size = WidgetSize::Large;
    else
        return false;
    return true;
}

ui::SizeF widgetSizeSpec(WidgetSize size)
{
    switch (size) {
    case WidgetSize::Small:
        return {176, 176};
    case WidgetSize::Medium:
        return {368, 176};
    case WidgetSize::Large:
        return {368, 368};
    }
    return {368, 176};
}

std::string serializeWidgets(const std::vector<WidgetPlacement>& placements)
{
    Value::Array array;
    array.reserve(placements.size());
    for (const WidgetPlacement& placement : placements) {
        Value object = Value::object();
        object.insert("kind", Value(widgetKindKey(placement.kind)));
        object.insert("size", Value(widgetSizeKey(placement.size)));
        object.insert("x", Value(placement.x));
        object.insert("y", Value(placement.y));
        if (!placement.id.empty())
            object.insert("id", Value(placement.id));
        array.push_back(std::move(object));
    }
    return Value(std::move(array)).serialize();
}

std::vector<WidgetPlacement> parseWidgets(std::string_view json)
{
    std::vector<WidgetPlacement> placements;
    const auto parsed = Value::parse(json);
    if (!parsed || !parsed->isArray())
        return placements;
    for (const Value& value : parsed->toArray()) {
        if (!value.isObject())
            continue;
        WidgetPlacement placement;
        if (!widgetKindFromKey(value["kind"].toString(), placement.kind))
            continue;
        if (!widgetSizeFromKey(value["size"].toString(), placement.size))
            continue;
        const double x = value["x"].toDouble(0);
        const double y = value["y"].toDouble(0);
        placement.x = std::isfinite(x) ? x : 0;
        placement.y = std::isfinite(y) ? y : 0;
        placement.id = value["id"].toString();
        placements.push_back(placement);
    }
    return placements;
}

} // namespace fm
