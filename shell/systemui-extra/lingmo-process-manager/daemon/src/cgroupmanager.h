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

#ifndef CGROUPMANAGER_H
#define CGROUPMANAGER_H

#include <QList>
#include <QString>
#include <memory>
#include <functional>
#include <libcgroup.h>
#include "cgroupv2releasenotification.h"

class CGroupManager
{
public:
    bool createProcessGroup(const QString &groupPath, 
        const QStringList &controllers, const QList<int> &pids);

    bool moveProcessToGroup(const QString &groupPath, 
        const QStringList &controllers, const QList<int> &pids);
        
    bool setProcessGroupResourceLimit(
        const QString &groupPath, const QString &controllerName, 
        const QString &attrFile, const QString &value);

    void removeProcessCGroup(const QString &groupPath);

    QList<int> getGroupPids(const QString &groupName) const;

private:
    using CgroupUniquePtr = std::unique_ptr<cgroup, std::function<void(cgroup *)>>;

    bool cgroupExists(const QString &groupPath);
    CgroupUniquePtr initCgroupWithPath(const QString &groupPath);
    CgroupUniquePtr createCgroup(const QString &cgroupName, const QStringList &controllers);
    void setCGroupUidGidByPath(cgroup *cgroup, const QString &groupPath);

    void initCGroupV1Controllers(
        cgroup *cgroup, const QStringList &controllers, 
        const QString &notifyOnReleaseValue);
    void setCGroupV1ReleaseAgent(const QStringList &controllers);

    void initCGroupV2Controllers(
        cgroup *cgroup, const QStringList &controllers);

    void writePidToProcsFile(int pid, const QString &procsFile);

    void attchPidsToCGroup(
        cgroup *cgroup, const QStringList &controllers, 
        const QList<int> &pids);

    void attchPidsToCGroupV1(
        cgroup *cgroup, const QStringList &controllers, 
        const QList<int> &pids);
    void writePidsToProcsFileV1(
        cgroup *cgroup, cgroup_controller *controller, 
        const QList<int> &pids);

    void attchPidsToCGroupV2(cgroup *cgroup, const QList<int> &pids);

private:
    CGroupV2ReleaseNotification m_cgroupv2ReleaseNotification;
};

#endif // CGROUPMANAGER_H
