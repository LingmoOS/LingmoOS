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

#include "groupmanagementunit.h"
#include <QDebug>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "profileaction.h"
#include "schedpolicy.h"
#include "misc.h"

namespace {

const char *name_key = "Name";
const char *value_key = "Value";
const char *path_key = "Path";
const char *profiles_key = "Profiles";
const char *controller_key = "Controller";
const char *controllers_key = "Controllers";
const char *actions_key = "Actions";
const char *param_key = "Param";
const char *file_key = "File";
const char *set_attribute_action_name = "SetAttribute";
const char *user_prefix = "user/";
const char *user_slice_prefix = "user.slice/";
const char *service_prefix = "service/";
const char *session_scope = "session.scope";

}

GroupManagementUnitCreator::GroupManagementUnitCreator(
    const std::shared_ptr<ResourceManagerInterface> &resourceInterface,
    const Json::Value &controllerList, const Json::Value &attributes)
    : m_controllerList(controllerList)
    , m_attributes(attributes)
    , m_resourceInterface(resourceInterface)
{
}

std::unique_ptr<GroupManagementUnit> GroupManagementUnitCreator::createGroupManagementUnit(const Json::Value &groupInfo)
{
    auto controllers = createControllers(groupInfo);
    auto groupUnit = std::unique_ptr<GroupManagementUnit>(
        new GroupManagementUnit(createProcessGroupInfo(groupInfo), std::move(controllers), m_resourceInterface));
    createProcessGroup(*groupUnit);
    return groupUnit;
}

void GroupManagementUnitCreator::createProcessGroup(const GroupManagementUnit &processGroupUnit)
{
    const QString groupPath = QString::fromStdString(processGroupUnit.groupPath());
    if (groupPath.endsWith(".scope")) {
        return;
    }

    m_resourceInterface->createPersistentProcessGroup(processGroupUnit.groupPath(), processGroupUnit.cgroupControllerNames());
}

std::unique_ptr<ProcessGroupInfo> GroupManagementUnitCreator::createProcessGroupInfo(const Json::Value &groupInfo)
{
    auto groupName = groupInfo[name_key].asString();
    auto path = groupInfo[path_key].asString();
    auto type = sched_policy::groupTypeFromString(groupName);
    return std::unique_ptr<ProcessGroupInfo>(new ProcessGroupInfo{groupName, cgroupPath(path), type});
}

GroupManagementUnit::PolicyControllerMap GroupManagementUnitCreator::createControllers(const Json::Value &groupInfo)
{
    GroupManagementUnit::PolicyControllerMap policyControllers;
    auto profiles = groupInfo[profiles_key];
    auto deviceModeNames = profiles.getMemberNames();
    for (const auto &deviceModeName : deviceModeNames) {
        auto deviceModeProfiles = profiles[deviceModeName];
        auto powerModeNames = deviceModeProfiles.getMemberNames();
        for (const auto &powerModeName : powerModeNames) {
            auto controllers = createResouceControllers(deviceModeName, powerModeName, deviceModeProfiles);
            auto policyId = sched_policy::generatePolicyId(deviceModeName, powerModeName);
            policyControllers[policyId] = std::move(controllers);
        }
    }

    return policyControllers;
}

std::vector<std::unique_ptr<ResourceController>> GroupManagementUnitCreator::createResouceControllers(
    const std::string &deviceMode, const std::string powerMode, const Json::Value &profiles)
{
    std::vector<std::unique_ptr<ResourceController>> resourceControllers;
    auto controllers = profiles[powerMode][controllers_key];
    if (controllers.empty()) {
        resourceControllers.emplace_back(std::unique_ptr<ResourceController>(new ResourceController("")));
        return resourceControllers;
    }
    for (Json::Value::ArrayIndex j=0; j<controllers.size(); ++j) {
        if (controllers[j].empty()) {
            continue;
        }
        auto controllerName = controllers[j][name_key].asString();
        auto controllerValuesArray = controllers[j][value_key];
        std::vector<std::string> controllerValues;
        for (Json::Value::ArrayIndex h=0; h<controllerValuesArray.size(); ++h) {
            controllerValues.emplace_back(controllerValuesArray[h].asString());
        }
        auto resourceController = createResourceController(controllerName, controllerValues);
        if (!resourceController) {
            qWarning() << QString("Resource controller '%1' is empty.")
                              .arg(QString::fromStdString(controllerName));
            continue;
        }
        resourceControllers.emplace_back(std::move(resourceController));
    }
    return resourceControllers;
}

std::string GroupManagementUnitCreator::cgroupPath(const std::string &path)
{
    const std::string userPrefix(user_prefix);
    if (misc::string::startWith(path, userPrefix)) {
        return cgroupPathFromCurrentUser(path.substr(userPrefix.length(), path.length() - userPrefix.length()));
    }

    return std::string();
}

std::string GroupManagementUnitCreator::cgroupPathFromCurrentUser(const std::string &path)
{
    static SystemdDbusInterface systemdInterface;
    static auto currentUserSlice = systemdInterface.currentUserSliceName();
    static auto currentUserService = systemdInterface.currentUserServiceName();
    static auto currentSessionScope = systemdInterface.currentSessionScopeName();

    if (path == session_scope) {
        return user_slice_prefix + currentUserSlice + "/" + currentSessionScope;
    } else if (misc::string::startWith(path, service_prefix)) {
        std::string prefix = user_slice_prefix + currentUserSlice + "/" + currentUserService;
        return prefix + "/" + path.substr(8, path.size() - 8);
    }
    return std::string();
}

std::unique_ptr<ResourceController> GroupManagementUnitCreator::createResourceController(
    const std::string &controllerName, const std::vector<std::string> &values)
{
    for (Json::Value::ArrayIndex i = 0; i < m_controllerList.size(); ++i) {
        auto name = m_controllerList[i][name_key].asString();
        if (name != controllerName) {
            continue;
        }

        auto actions = m_controllerList[i][actions_key];
        std::unique_ptr<ResourceController> resourceController(new ResourceController(name));
        if (values.size() != actions.size()) {
            qWarning() << QString("Can not find the controller '%1' info.")
                              .arg(QString::fromStdString(controllerName));
            return nullptr;
        }
        for (Json::Value::ArrayIndex j=0; j<actions.size(); ++j) {
            auto actionName = actions[j][name_key].asString();
            auto actionParam = actions[j][param_key].asString();
            if (actionName == set_attribute_action_name) {
                auto attribute = createProfileAttribute(actionParam);
                std::unique_ptr<ProfileAction> action(
                    new SetAttributeAction(std::move(attribute), values[j], m_resourceInterface));
                resourceController->addAction(std::move(action));
            }
        }
        return resourceController;
    }
    return nullptr;
}

std::unique_ptr<ProfileAttribute> GroupManagementUnitCreator::createProfileAttribute(const std::string &attrName)
{
    for (Json::Value::ArrayIndex i = 0; i < m_attributes.size(); ++i) {
        auto name = m_attributes[i][name_key].asString();
        if (attrName != name) {
            continue;
        }
        auto controllerName = m_attributes[i][controller_key].asString();
        auto fileAttr = m_attributes[i][file_key].asString();
        return std::unique_ptr<ProfileAttribute>(new CgroupProfileAttribute{name, controllerName, fileAttr});
    }
    return nullptr;
}

GroupManagementUnit::GroupManagementUnit(
    std::unique_ptr<ProcessGroupInfo> groupInfo, PolicyControllerMap policyControllers,
    const std::shared_ptr<ResourceManagerInterface> &resourceInterface)
    : m_groupInfo(std::move(groupInfo))
    , m_policyControllers(std::move(policyControllers))
    , m_resourceInterface(resourceInterface)
{
}

void GroupManagementUnit::executeResourceLimit(const std::string &policyId)
{
    if (m_policyControllers.find(policyId) == m_policyControllers.end()) {
        qWarning() << QString("Exceute resource limit failed, can not find the policy id %1.")
                          .arg(QString::fromStdString(policyId));
        return;
    }

    for (const auto &controller : m_policyControllers[policyId]) {
        controller->executeForCgroup(m_groupInfo->path);
    }
}

std::string GroupManagementUnit::createTransientProcessGroup(
    const std::string &cgroupName, const std::vector<int> &pids)
{
    const std::string groupName = groupPath() + "/" + cgroupName;
    m_resourceInterface->createTransientProcessGroup(
        groupName, pids, cgroupControllerNames());
    
    return groupName;
}

void GroupManagementUnit::moveProcessToGroup(const std::string &cgroupPath, const std::vector<int> &pids)
{
    m_resourceInterface->moveProcessToGroup(cgroupPath, cgroupControllerNames(), pids);
}

std::vector<std::string> GroupManagementUnit::cgroupControllerNames() const
{
    std::vector<std::string> controllerNames;

    auto mergeControllerNames = [&controllerNames](const std::vector<std::string> &newNames) {
        for (const auto &name : newNames) {
            auto it = std::find(controllerNames.begin(), controllerNames.end(), name);
            if (it == controllerNames.end()) {
                controllerNames.emplace_back(name);
            }
        }
    };

    for (const auto &controllers : m_policyControllers) {
        for (const auto &controller : controllers.second) {
            mergeControllerNames(controller->cgroupControllerNames());
        }
    }

    return controllerNames;
}
