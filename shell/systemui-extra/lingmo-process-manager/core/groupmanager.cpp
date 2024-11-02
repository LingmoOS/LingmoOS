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

#include "groupmanager.h"
#include <QDebug>
#include "resourcemanagerinterface.h"
#include "processinfohelper.h"

void GroupManager::initGroups()
{
    auto groups = m_groupConfigs["AggregateProfiles"];
    auto controllers = m_groupConfigs["ControllerList"];
    auto attributes = m_groupConfigs["Attributes"];

    m_unitCreator.reset(new GroupManagementUnitCreator(
        std::make_shared<ResourceManagerInterface>(), controllers, attributes));

    for (Json::Value::ArrayIndex i=0; i<groups.size(); ++i) {
        auto unit = m_unitCreator->createGroupManagementUnit(groups[i]);
        m_groupUnits[unit->groupType()] = std::move(unit);
    }
}

void GroupManager::excutePolicy(const std::string &policyId)
{
    for (const auto &cgroupUnit : m_groupUnits) {
        cgroupUnit.second->executeResourceLimit(policyId);
    }
}

void GroupManager::setGroupConfigs(const Json::Value &configs)
{
    m_groupConfigs = configs;
    m_resourceManagerInterface.setCurrentUserServiceUnitPropertyEnabled("CPUAccounting", true);
    initGroups();
}

std::string GroupManager::createProcessGroup(
    const std::string &cgroupName, const std::vector<int> &pids,
    sched_policy::GroupType parentGroupType)
{
    if (m_groupUnits.find(parentGroupType) == m_groupUnits.end()) {
        qWarning() << QString("Create process %1 group but not loaded the parent group info %2.")
                          .arg(QString::fromStdString(cgroupName))
                          .arg((int)parentGroupType);
        return std::string();
    }
    return m_groupUnits[parentGroupType]->createTransientProcessGroup(cgroupName, pids);
}

void GroupManager::moveProcessToGroup(
    int parentPid, std::vector<int> &childPids, sched_policy::GroupType parentGroupType)
{
    if (m_groupUnits.find(parentGroupType) == m_groupUnits.end()) {
        qWarning() << QString("Move process to group but not loaded the parent group info %2.")
                          .arg((int)parentGroupType);
        return;
    }
    m_groupUnits[parentGroupType]->moveProcessToGroup(process_info_helper::cgroup(parentPid), childPids);
}

std::string GroupManager::groupPath(sched_policy::GroupType type)
{
    if (m_groupUnits.find(type) == m_groupUnits.end()) {
        return std::string();
    }

    return m_groupUnits[type]->groupPath();
}

void GroupManager::reclaimProcessGroups(const std::vector<std::string>& groupNames)
{
    m_resourceManagerInterface.reclaimProcessGroups(groupNames);
}

void GroupManager::freezeGroups(const std::vector<std::string>& groupNames)
{
    auto freezerAttribute = getFreezerAttribute();
    std::string controller = freezerAttribute["Controller"].asString();
    std::string file = freezerAttribute["File"].asString();

    for (const auto& groupName : groupNames) {
        m_resourceManagerInterface.setProcessGroupResourceLimit(
            freezerAttribute["Name"].asString(), groupName, controller, file, "1");
    }
}

Json::Value GroupManager::getFreezerAttribute() const
{
    auto attributes = m_groupConfigs["Attributes"];

    for (Json::Value::ArrayIndex i = 0; i < attributes.size(); ++i) {
        if (attributes[i]["Name"] == "Freezer")
            return attributes[i];
    }

    return Json::Value();
}
