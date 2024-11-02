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

#ifndef GROUPMANAGEMENTUNIT_H
#define GROUPMANAGEMENTUNIT_H

#include <map>
#include <memory>
#include <jsoncpp/json/json.h>
#include "schedpolicy.h"
#include "resourcecontroller.h"
#include "resourcemanagerinterface.h"

struct ProcessGroupInfo
{
    std::string name;
    std::string path;
    sched_policy::GroupType type;
    std::vector<std::string> cgroupControllers;
};

class GroupManagementUnit
{
public:
    using PolicyControllerMap = 
        std::map<std::string, std::vector<std::unique_ptr<ResourceController> >>;

    GroupManagementUnit(std::unique_ptr<ProcessGroupInfo> groupInfo,
                        PolicyControllerMap policyControllers,
                        const std::shared_ptr<ResourceManagerInterface> &resourceInterface);

    void executeResourceLimit(const std::string &policyId);
    std::string createTransientProcessGroup(const std::string &cgroupName, const std::vector<int> &pids);
    void moveProcessToGroup(const std::string &cgroupPath, const std::vector<int> &pids);

    sched_policy::GroupType groupType() const
    { return m_groupInfo->type; }

    std::string groupPath() const
    { return m_groupInfo->path; }

    std::vector<std::string> cgroupControllerNames() const;

private:
    std::unique_ptr<ProcessGroupInfo> m_groupInfo;
    PolicyControllerMap m_policyControllers;
    std::shared_ptr<ResourceManagerInterface> m_resourceInterface;
};

class GroupManagementUnitCreator
{
public:
    GroupManagementUnitCreator(
        const std::shared_ptr<ResourceManagerInterface> &resourceInterface,
        const Json::Value &controllerList, const Json::Value &attributes);
    std::unique_ptr<GroupManagementUnit> createGroupManagementUnit(const Json::Value &groupInfo);

private:
    void createProcessGroup(const GroupManagementUnit &processGroupUnit);
    std::unique_ptr<ProcessGroupInfo> createProcessGroupInfo(const Json::Value &groupInfo);
    GroupManagementUnit::PolicyControllerMap createControllers(const Json::Value &groupInfo);
    std::vector<std::unique_ptr<ResourceController>> createResouceControllers(
        const std::string &deviceMode, const std::string powerMode, const Json::Value &profiles);

    std::string cgroupPath(const std::string &path);
    std::string cgroupPathFromCurrentUser(const std::string &path);

    std::unique_ptr<ResourceController> createResourceController(const std::string &controllerName,
                                                                 const std::vector<std::string> &values);
    std::unique_ptr<ProfileAttribute> createProfileAttribute(const std::string &attrName);

private:
    Json::Value m_controllerList;
    Json::Value m_attributes;
    std::shared_ptr<ResourceManagerInterface> m_resourceInterface;
};

#endif // GROUPMANAGEMENTUNIT_H
