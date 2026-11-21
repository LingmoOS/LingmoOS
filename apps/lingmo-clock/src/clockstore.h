// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "clockcore.h"
#include <sqlite3.h>
#include <string>
#include <vector>

namespace Clock {
class Store {
  public:
    explicit Store(std::string path = {});
    ~Store();
    Store(const Store&) = delete;
    Store& operator=(const Store&) = delete;
    std::vector<WorldClock> worldClocks() const;
    void addWorldClock(const WorldClock& clock);
    void removeWorldClock(const std::string& id);
    std::vector<Alarm> alarms() const;
    int64_t saveAlarm(const Alarm& alarm);
    void removeAlarm(int64_t id);
    void setAlarmEnabled(int64_t id, bool enabled);

  private:
    sqlite3* m_db = nullptr;
};
} // namespace Clock
