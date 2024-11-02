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

#include "cgroupmanager.h"
#include <pwd.h>
#include <QCoreApplication>
#include <QFile>
#include <QDebug>
#include "misc.h"

#define LOG_WARNING(str, errorno) qWarning() << __FUNCTION__ << str << cgroup_strerror(errorno);

namespace {

const char *cleaner_path = "/usr/bin/lingmo-process-manager-cleaner";
const char *freeze_controller = "freezer";
const char *cgroup_root_path = "/sys/fs/cgroup/";

void cgroupDeleter(cgroup *cg) {
    // Free internal cgroup structure. This function frees also all controllers
    // attached to the cgroup, including all parameters and their values.
    if (cg) { cgroup_free(&cg); }
};

QString getRealControllerName(const QString &sourceControllerName) {
    if (misc::version::cgroupVersion() == 1) {
        return sourceControllerName;
    }

    if (sourceControllerName == freeze_controller) {
        return "misc";
    }
    return sourceControllerName;
}

long long getUidFromCGroupPath(const QString &cgroupPath) {
    const QStringList cgroupNames = cgroupPath.split('/');
    for (const auto &cgroupName : cgroupNames) {
        if (cgroupName.contains(QRegExp("user@\\d+.service"))) {
            int uidStartIndex = cgroupName.indexOf(QRegExp("\\d+"));
            int uidEndIndex = cgroupName.indexOf(".service");
            int uidLength = uidEndIndex - uidStartIndex;
            return cgroupName.mid(uidStartIndex, uidLength).toLongLong();
        }
    }

    return -1;
}

long long getGidByUid(long long uid) {
    passwd *userInfo = getpwuid(uid);
    if (!userInfo) {
        qWarning() << QString("Get gid by uid %1 failed: %2").arg(uid, errno);
        return -1;
    }

    return userInfo->pw_gid;
}

}

bool CGroupManager::createProcessGroup(
    const QString &groupPath, const QStringList &controllers, const QList<int> &pids)
{
    auto cgroup = cgroupExists(groupPath) ?
        initCgroupWithPath(groupPath) : createCgroup(groupPath, controllers);

    if (cgroup == nullptr) {
        return false;
    }

    attchPidsToCGroup(cgroup.get(), controllers, pids);

    if (misc::version::cgroupVersion() == 2 &&
        groupPath.endsWith(".scope")) {
        m_cgroupv2ReleaseNotification.addWatchGroup(
            cgroup_root_path +  groupPath.toStdString(), 
            [this](std::string groupPath) {
                groupPath.erase(0, strlen(cgroup_root_path));
                removeProcessCGroup(QString::fromStdString(groupPath));
            });
    }
    
    return true;
}

bool CGroupManager::moveProcessToGroup(
    const QString &groupPath, const QStringList &controllers, const QList<int> &pids)
{
    return createProcessGroup(groupPath, controllers, pids);
}

bool CGroupManager::setProcessGroupResourceLimit(
    const QString &groupPath, const QString &controllerName, 
    const QString &attrFile, const QString &value)
{
    auto cgroup = initCgroupWithPath(groupPath);
    if (cgroup == nullptr)  {
        return false;
    }

    const QString realControllerName = getRealControllerName(controllerName);    
    cgroup_controller *controller = cgroup_add_controller(
        cgroup.get(), realControllerName.toLocal8Bit().data());

    if (!controller) {
        LOG_WARNING(
            QString("Add cgroup controller failed on path %1.").arg(groupPath),
            cgroup_get_last_errno());
        return false;
    }

    int ret = cgroup_set_value_string(
        controller, attrFile.toLocal8Bit().data(), value.toLocal8Bit().data());
    
    if (ret != 0) {
        LOG_WARNING(
            QString("Set cgroup string value failed on path%1.").arg(groupPath), ret);
        
        return false;
    }
    
    ret = cgroup_modify_cgroup(cgroup.get());

    if (ret != 0) {
        LOG_WARNING(
            QString("Modify cgroup %1 failed.").arg(groupPath), ret);
        return false;
    }

    return true;
}

void CGroupManager::removeProcessCGroup(const QString &groupPath)
{
    qInfo() << "Remove process group:" << groupPath;
    auto cgroup = initCgroupWithPath(groupPath);
    if (cgroup == nullptr)  {
        return;
    }

    int ret = cgroup_get_cgroup(cgroup.get());
    if (ret != 0) {
        qDebug() << cgroup_strerror(ret);
        LOG_WARNING(QString("Get cgroup %1 faild.").arg(groupPath), cgroup_get_last_errno());
        return;
    }

    ret = cgroup_delete_cgroup(cgroup.get(), 1);
    if (ret != 0) {
        LOG_WARNING(QString("Delete cgroup %1 faild.").arg(groupPath), ret);
    }
}

bool CGroupManager::cgroupExists(const QString &groupPath)
{
    auto cgroup = initCgroupWithPath(groupPath);
    if (!cgroup) {
        return false;
    }

    return cgroup_get_cgroup(cgroup.get()) == 0;
}

CGroupManager::CgroupUniquePtr CGroupManager::initCgroupWithPath(const QString &groupPath)
{
    int ret = cgroup_init();
    if (ret != 0) {
        LOG_WARNING(QString("Init cgroup %1 failed.").arg(groupPath), ret);
        return nullptr;
    }

    CgroupUniquePtr cgroup(cgroup_new_cgroup(groupPath.toLatin1().data()), cgroupDeleter);

    if (cgroup == nullptr) {
        LOG_WARNING(QString("New cgroup %1 failed.").arg(groupPath), cgroup_get_last_errno());
        return nullptr;
    }
    return cgroup;
}

CGroupManager::CgroupUniquePtr CGroupManager::createCgroup(
    const QString &cgroupPath, const QStringList &controllers)
{
    auto cgroup = initCgroupWithPath(cgroupPath);
    if (!cgroup)  {
        return nullptr;
    }

    if (misc::version::cgroupVersion() == 1) {
        QString notifyOnReleaseValue =
            cgroupPath.endsWith(".scope") ? "1" : "0";
        initCGroupV1Controllers(cgroup.get(), controllers, notifyOnReleaseValue);
        setCGroupV1ReleaseAgent(controllers);
    }

    if (misc::version::cgroupVersion() == 2) {
        initCGroupV2Controllers(cgroup.get(), controllers);
        setCGroupUidGidByPath(cgroup.get(), cgroupPath);
    }

    int ret = cgroup_create_cgroup(cgroup.get(), 0);
    if (ret != 0) {
        LOG_WARNING(QString("Create cgroup %1 failed.").arg(cgroupPath), ret);
        return nullptr;
    }

    return cgroup;
}

void CGroupManager::setCGroupUidGidByPath(cgroup *cgroup, const QString &groupPath)
{
    long long uid = getUidFromCGroupPath(groupPath);
    if (uid < 0) {
        return;
    }

    long long gid = getGidByUid(uid);
    if (gid < 0) {
        return;
    }

    int ret = cgroup_set_uid_gid(cgroup, uid, gid, uid, gid);
    if (ret != 0) {
        LOG_WARNING(QString("Set cgroup owner failed."), ret);
    }
}

void CGroupManager::initCGroupV1Controllers(
    cgroup *cgroup, const QStringList &controllers, const QString &notifyOnReleaseValue)
{
    for (auto &controllerName : qAsConst(controllers)) {
        cgroup_controller *controller = 
            cgroup_add_controller(cgroup, controllerName.toLocal8Bit().data());
        if (!controller) {
            LOG_WARNING(
                QString("Add cgroup controller %1 failed.").arg(controllerName),
                cgroup_get_last_errno());
            continue;
        }

        int ret = cgroup_set_value_string(
            controller, "notify_on_release",
            notifyOnReleaseValue.toLocal8Bit().data());
        if (ret != 0) {
            LOG_WARNING(
                QString("Set %1 notify_on_release failed.").arg(controllerName),
                ret);
        }
    }
}

void CGroupManager::setCGroupV1ReleaseAgent(const QStringList &controllers)
{
    auto cgroup = initCgroupWithPath("/");
    if (cgroup == nullptr)  {
        return;
    }

    for (auto &controlName : qAsConst(controllers)) {
        cgroup_controller *controller = 
            cgroup_add_controller(cgroup.get(), controlName.toLocal8Bit().data());
        if (!controller) {
            LOG_WARNING(
                QString("Add cgroup controller %1 failed.").arg(controlName),
                cgroup_get_last_errno());
            continue;
        }
        int ret = cgroup_set_value_string(controller, "release_agent", cleaner_path);
        if (ret != 0) {
            LOG_WARNING(QString("Set release_agent %1 failed.").arg(controlName), ret);
        }

        ret = cgroup_create_cgroup(cgroup.get(), 0);
        if (ret != 0) {
            LOG_WARNING(QString("Create cgroup %1 failed.").arg(controlName), ret);
        }   
    }
}

void CGroupManager::initCGroupV2Controllers(
    cgroup *cgroup, const QStringList &controllers)
{
    for (auto &controllerName : qAsConst(controllers)) {
        if (controllerName == freeze_controller) {
            continue;
        }

        cgroup_controller *controller =
            cgroup_add_controller(cgroup, controllerName.toLocal8Bit().data());
        if (!controller) {
            LOG_WARNING(
                QString("Add cgroup controller %1 failed.").arg(controllerName),
                cgroup_get_last_errno());
            continue;
        }
    }
}

void CGroupManager::writePidToProcsFile(int pid, const QString &procsFile)
{
    QFile file(procsFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Unbuffered)) {
        qWarning() << "Write pid to cgroup.procs file failed, " << file.errorString();
        return;
    }
    file.write(QString::number(pid).toLocal8Bit());
}

void CGroupManager::attchPidsToCGroup(
    cgroup *cgroup, const QStringList &controllers, const QList<int> &pids)
{
    if (misc::version::cgroupVersion() == 1) {
        attchPidsToCGroupV1(cgroup, controllers, pids);
    } else if (misc::version::cgroupVersion() == 2) {
        attchPidsToCGroupV2(cgroup, pids);
    }
}

void CGroupManager::attchPidsToCGroupV1(
    cgroup *cgroup, const QStringList &controllers, const QList<int> &pids)
{
    for (const auto &controllerName : controllers) {
        cgroup_controller *controller = 
            cgroup_get_controller(cgroup, controllerName.toLocal8Bit().constData());

        if (!controller) {
            LOG_WARNING(
                QString("Add cgroup controller %1 failed.").arg(controllerName),
                cgroup_get_last_errno());
            continue;
        }
        writePidsToProcsFileV1(cgroup, controller, pids);
    }
}

void CGroupManager::attchPidsToCGroupV2(cgroup *cgroup, const QList<int> &pids)
{
    for (const int &pid : pids) {
        int ret = cgroup_attach_task_pid(cgroup, pid);
        if (ret != 0) {
            LOG_WARNING("Attch pid to cgroup failed: ", ret);
        }
    }
}

void CGroupManager::writePidsToProcsFileV1(
    cgroup *cgroup, cgroup_controller *controller, const QList<int> &pids)
{
    int ret = 0;
    for (const int &pid : pids) {
        ret = cgroup_add_value_int64(controller, "cgroup.procs", pid);
        if (ret != 0) {
            LOG_WARNING("Write pid to cgroup.procs failed: ", ret);
        }
    }

    ret = cgroup_modify_cgroup(cgroup);
    if (ret != 0) {
        LOG_WARNING("Modify cgroup for writing pid to cgroup.procs failed: ", ret);
    }
}

QList<int> CGroupManager::getGroupPids(const QString &groupName) const
{
    int ret = cgroup_init();
    if (ret != 0) {
        LOG_WARNING("Init cgroup failed: ", ret);
        return QList<int>();
    }

    const char *controller = "pids";
    int size = 0;
    pid_t *pids = nullptr;
    QByteArray ba = groupName.toLocal8Bit();

    ret = cgroup_get_procs(ba.data(), (char *)controller, &pids, &size);
    if (ret != 0) {
        qWarning() << "cgroup_get_procs error, " << cgroup_strerror(cgroup_get_last_errno());
        LOG_WARNING("Get cgroup process ids failed: ", ret);
        return QList<int>();
    }

    QList<int> result;
    for (int i = 0; i < size; ++i) {
        result.push_back(pids[i]);
    }

    delete[] pids;
    return result;
}
