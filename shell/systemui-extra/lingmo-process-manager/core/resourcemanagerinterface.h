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

#ifndef RESOURCEMANAGERINTERFACE_H
#define RESOURCEMANAGERINTERFACE_H

#include "daemondbusinterface.h"
#include "systemddbusinterface.h"

class ResourceManagerInterface
{
public:
    void createPersistentProcessGroup(
        const std::string &path, const std::vector<std::string> &cgroupControllers);

    void createTransientProcessGroup(
        const std::string &path, const std::vector<int> &pids,
        const std::vector<std::string> &cgroupControllers);

    void setProcessGroupResourceLimit(
        const std::string &attributeName,
        const std::string &path, const std::string &controller,
        const std::string &file, const std::string &value);

    void moveProcessToGroup(
        const std::string &path, const std::vector<std::string> &controllers,
        const std::vector<int> &pids);

    void setCurrentUserServiceUnitPropertyEnabled(
        const std::string &property, bool enabled);

    void reclaimProcessGroups(const std::vector<std::string> &groupNames);

private:
    std::string getGroupNameFromPath(const std::string &path) const;
    std::string getGroupParentSliceFromPath(const std::string &path) const;

private:
    DaemonDbusInterface m_daemonDbus;
    SystemdDbusInterface m_systemdDbus;
};

#endif // RESOURCEMANAGERINTERFACE_H
