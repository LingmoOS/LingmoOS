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

#include "resourcemanagerinterface.h"
#include "misc.h"
#include <QDebug>

void ResourceManagerInterface::createPersistentProcessGroup(
    const std::string &path, const std::vector<std::string> &cgroupControllers)
{
    if (misc::version::cgroupVersion() == 1) {
        m_daemonDbus.createProcessGroup(path, cgroupControllers, std::vector<int>{});
    } else if (misc::version::cgroupVersion() == 2) {
        m_systemdDbus.createPersistentProcessGroup(getGroupNameFromPath(path));
    }
}

void ResourceManagerInterface::createTransientProcessGroup(
    const std::string &path, const std::vector<int> &pids,
    const std::vector<std::string> &cgroupControllers)
{
    if (misc::version::cgroupVersion() == 1) {
        m_daemonDbus.createProcessGroup(path, cgroupControllers, pids);
    }
    m_systemdDbus.createTransientProcessGroup(
        getGroupNameFromPath(path), pids, getGroupParentSliceFromPath(path));
}

void ResourceManagerInterface::setProcessGroupResourceLimit(
    const std::string &attributeName,
    const std::string &path, const std::string &controller,
    const std::string &file, const std::string &value)
{
    if (misc::version::cgroupVersion() == 1) {
        m_daemonDbus.setProcessGroupResourceLimit(path, controller, file, value);
    } else if (misc::version::cgroupVersion() == 2) {
        const std::string unitName = getGroupNameFromPath(path);
        m_systemdDbus.setProcessGroupResourceLimit(
            unitName, attributeName, value);
    }
}

void ResourceManagerInterface::moveProcessToGroup(
    const std::string &path, const std::vector<std::string> &controllers,
    const std::vector<int> &pids)
{
    m_daemonDbus.moveProcessToGroup(path, controllers, pids);
}

void ResourceManagerInterface::setCurrentUserServiceUnitPropertyEnabled(
    const std::string &property, bool enabled)
{
    const auto &currentUserService = m_systemdDbus.currentUserServiceName();
    m_daemonDbus.setSystemdUnitPropertyEnabled(currentUserService, property, enabled);
}

void ResourceManagerInterface::reclaimProcessGroups(const std::vector<std::string> &groupNames)
{
    m_daemonDbus.reclaimProcessGroups(groupNames);
}

std::string ResourceManagerInterface::getGroupNameFromPath(const std::string &path) const
{
    auto stringList = misc::string::split(path, '/');
    if (stringList.empty()) {
        return std::string();
    }

    return stringList.back();
}

std::string ResourceManagerInterface::getGroupParentSliceFromPath(const std::string &path) const
{
    auto stringList = misc::string::split(path, '/');
    if (stringList.size() < 2) {
        return std::string();
    }

    return stringList.at(stringList.size() - 2);
}
