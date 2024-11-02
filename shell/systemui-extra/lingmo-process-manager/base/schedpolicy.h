/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SCHEDPOLICY_H
#define SCHEDPOLICY_H

#include <string>
#include <map>

namespace sched_policy {

enum class AppState {
    Focus,
    Foreground,
    Background,
    Service,
    Cached,
};

enum class AppType {
    Unknown,
    Normal,
    Top,
    Session,
};

enum class GroupType {
    Unknown,
    SessionScopeGroup,
    SessionGroup,
    TopGroup,
    FocusGroup,
    ForegroundGroup,
    BackgroundGroup,
    ServiceGroup,
    CachedGroup,
    DefaultGroup,
};

enum class DeviceMode {
    Unknown,
    PC,
    Tablet,
    SoftFreeze,
};

enum class PowerMode {
    Unknown,
    Balance,
    Save,
    Performance,
};

enum class PowerType {
    Battery,
    Ac
};

DeviceMode deviceModeFromString(const std::string &mode);
PowerMode powerModeFromString(const std::string &mode);
GroupType groupTypeFromString(const std::string &type);

std::string deviceModeToString(DeviceMode mode);

std::string generatePolicyId(const std::string &deviceMode, const std::string &powerMode);
std::string generatePolicyId(DeviceMode deviceMode, PowerMode powerMode);

}

#endif // SCHEDPOLICY_H
