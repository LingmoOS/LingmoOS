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

#ifndef GROUPMANAGER_H
#define GROUPMANAGER_H

#include <jsoncpp/json/json.h>
#include "groupmanagementunit.h"

class GroupManager
{
public:
    GroupManager() = default;

    void excutePolicy(const std::string &policyId);
    void setGroupConfigs(const Json::Value &configs);

    std::string createProcessGroup(
        const std::string &cgroupName, const std::vector<int> &pids,
        sched_policy::GroupType parentGroupType);

    void moveProcessToGroup(
        int parentPid, std::vector<int> &childPids,
        sched_policy::GroupType parentGroupType);

    std::string groupPath(sched_policy::GroupType type);

    void freezeGroups(const std::vector<std::string>& groupNames);
    void reclaimProcessGroups(const std::vector<std::string> &groupNames);

private:
    void initGroups();
    Json::Value getFreezerAttribute() const;

private:
    std::unique_ptr<GroupManagementUnitCreator> m_unitCreator;
    std::map<sched_policy::GroupType, std::unique_ptr<GroupManagementUnit>> m_groupUnits;
    ResourceManagerInterface m_resourceManagerInterface;
    Json::Value m_groupConfigs;
};

#endif // GROUPMANAGER_H
