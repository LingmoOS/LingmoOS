// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <vector>

#include "base/signal.h"
#include "ui/geometry.h"

namespace Lingmo::timedate {
class TimeDate;
}

namespace settings {

// Every zone of zone.tab placed on the world map, and the zones a click on
// the map picks.
class TimeZoneMap
{
public:
    struct Point
    {
        std::string id;
        // Fractions of the map's size.
        double x = 0;
        double y = 0;
        int offsetSeconds = 0;
    };

    struct Info
    {
        std::string id;
        std::string name;
        std::string region;
        int offsetSeconds = 0;
        // The wall clock there now.
        std::string time;
        // "Tomorrow" or "Yesterday" when its date is not today's here.
        std::string day;
        // The gap to the zone in use, in words.
        std::string difference;
    };

    explicit TimeZoneMap(Lingmo::timedate::TimeDate& timeDate);
    ~TimeZoneMap();

    // Picks the zones close enough to a click on a map of the given size, or
    // the nearest one.
    void clicked(int x, int y, int width, int height);
    // Asks timedated to use the zone.
    void setTimeZone(const std::string& id);
    // Makes the zone the only candidate, as a click on its spot would.
    void select(const std::string& id);

    // Where the zone sits on a map of the given size; (-1, -1) if unknown.
    Lingmo::ui::PointF position(const std::string& id, int width, int height) const;
    // The zone nearest to a point on a map of the given size; empty if there
    // are no zones.
    std::string nearest(int x, int y, int width, int height) const;
    std::vector<Point> points() const;
    // Empty when the zone is unknown.
    Info info(const std::string& id) const;
    // Zones whose id, city or country matches the text, best first.
    std::vector<Info> search(const std::string& text, int limit = 12) const;

    // The city of the zone in use, and its id.
    std::string currentTimeZone() const;
    const std::string& currentTimeZoneId() const { return m_currentTimeZone; }
    // The candidates of the last click or selection, sorted.
    const std::vector<std::string>& availableList() const { return m_currentList; }

    // "America/Los_Angeles" reads "Los Angeles".
    static std::string localeTimeZoneName(const std::string& id);
    // The territory's English name, "United States" for "US".
    static std::string territoryName(const std::string& code);
    // The clock in the zone at the given UTC time, in the user's locale.
    static std::string shortTime(const std::string& id, int64_t utcSeconds);

    Lingmo::base::Signal<> availableListChanged;
    Lingmo::base::Signal<> currentTimeZoneChanged;

private:
    struct Item
    {
        std::string country;
        std::string timeZone;
        double latitude = 0;
        double longitude = 0;
    };

    void setCurrentTimeZone(const std::string& id);
    int nearestIndex(int x, int y, int width, int height) const;
    const Item* find(const std::string& id) const;

    Lingmo::timedate::TimeDate& m_timeDate;
    Lingmo::base::Subscription m_timeDateChanged;
    std::vector<Item> m_list;
    std::string m_currentTimeZone;
    std::vector<std::string> m_currentList;
};

} // namespace settings
