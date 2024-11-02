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

#include "appinfo.h"
#include <QCryptographicHash>
#include <QFile>
#include <QDebug>
#include <kdesktopfile.h>
#include <vector>
#include <string>
#include <algorithm>
#include "misc.h"
#include "processinfohelper.h"

struct AppInfo::AppInfoPrivate
{
    void generateBaseInfoFromDesktopFile(const std::string &desktopFile, int pid);
    void generateNameFromDesktopFile(const std::string &desktopFile);
    bool groupHasCreated() const;
    std::vector<int> pidsInGroup() const;

    std::string appId;
    std::string name;
    std::string desktopFileId;
    std::string desktopFile;
    std::string cmdline;
    std::string mprisDbusService;
    int64_t launchTimestamp;
    // 相对路径
    std::string groupName;
    std::vector<std::string> args;
    std::vector<std::string> wids;
    sched_policy::AppState state;
    sched_policy::AppType type;
    bool isAvailable = false;
    int launcherPid = -1;
    bool isSystemTrayIconApp = false;
    AppInfo::AppStateChangedCallback stateChangedCallback;
};

void AppInfo::AppInfoPrivate::generateBaseInfoFromDesktopFile(const std::string &desktopFile, int pid)
{
    QCryptographicHash hash(QCryptographicHash::Md5);
    QFile file(QString::fromStdString(desktopFile));
    if (!file.open(QIODevice::ReadOnly)) {
        isAvailable = false;
        qWarning() << QString("Generate id failed, the desktop file '%1' dose not exist.").arg(QString::fromStdString(desktopFile));
        return;
    }
    hash.addData(file.readAll());
    desktopFileId = hash.result().toHex().toStdString();
    appId = desktopFileId + "_" + std::to_string(pid);
    generateNameFromDesktopFile(desktopFile);
    isAvailable = true;
}

void AppInfo::AppInfoPrivate::generateNameFromDesktopFile(const std::string &desktopFile)
{
    const std::string suffix = ".desktop";
    auto strList = misc::string::split(desktopFile, '/');
    if (strList.size() > 0) {
        name = strList.back();
        auto pos = name.find(suffix);
        if (pos != name.npos) {
            name.erase(pos, suffix.length());
        }
    }
}

bool AppInfo::AppInfoPrivate::groupHasCreated() const
{
    return !groupName.empty();
}

std::vector<int> AppInfo::AppInfoPrivate::pidsInGroup() const
{
    const std::string groupPrefixPath =
        misc::version::cgroupVersion() == 1 ? "/sys/fs/cgroup/cpu/" : "/sys/fs/cgroup/";
    return process_info_helper::pidsOfCgroup(groupPrefixPath + groupName);
}

AppInfo::AppInfo()
    : pImpl(new AppInfoPrivate())
{
    pImpl->isAvailable = false;
}

AppInfo::AppInfo(const std::string &desktopFile,
                 int launcherPid,
                 int64_t launchTimestap,
                 sched_policy::AppType type,
                 sched_policy::AppState appState,
                 AppStateChangedCallback stateChangedCallback)
    : pImpl(new AppInfoPrivate())
{
    pImpl->generateBaseInfoFromDesktopFile(desktopFile, launcherPid);
    pImpl->desktopFile = desktopFile;
    pImpl->type = type;
    pImpl->launchTimestamp = launchTimestap;
    pImpl->state = appState;
    pImpl->launcherPid = launcherPid;
    pImpl->stateChangedCallback = std::move(stateChangedCallback);
}

AppInfo::AppInfo(const AppInfo &rhs)
    : pImpl(new AppInfoPrivate(*rhs.pImpl))
{

}

AppInfo &AppInfo::operator=(const AppInfo &rhs)
{
    pImpl.reset(new AppInfoPrivate(*rhs.pImpl));
    return *this;
}

bool AppInfo::operator==(const AppInfo &rhs)
{
    return this->pImpl->appId == rhs.pImpl->appId;
}

AppInfo::~AppInfo() = default;

std::string AppInfo::appId() const
{
    return pImpl->appId;
}

std::string AppInfo::name() const
{
    return pImpl->name;
}

std::string AppInfo::desktopFileId() const
{
    return pImpl->desktopFileId;
}

std::string AppInfo::desktopFile() const
{
    return pImpl->desktopFile;
}

std::string AppInfo::cmdline() const
{
    return pImpl->cmdline;
}

std::string AppInfo::mprisDbusService() const
{
    return pImpl->mprisDbusService;
}

int64_t AppInfo::launchTimestamp() const
{
    return pImpl->launchTimestamp;
}

std::string AppInfo::groupName() const
{
    return pImpl->groupName;
}

const std::vector<std::string> &AppInfo::args() const
{
    return pImpl->args;
}

int AppInfo::launcherPid() const
{
    return pImpl->launcherPid;
}

std::vector<int> AppInfo::pids() const
{
    if (pImpl->groupHasCreated()) {
        auto groupIds = pImpl->pidsInGroup();
        if (!groupIds.empty()) {
            return groupIds;
        }
    }

    auto chidPids = process_info_helper::childPids(pImpl->launcherPid);
    if (chidPids.empty() && !process_info_helper::processExists(pImpl->launcherPid)) {
        return {};
    }

    auto childrenPids = process_info_helper::childPids(chidPids);
    childrenPids.push_back(pImpl->launcherPid);

    return childrenPids;
}

const std::vector<std::string> &AppInfo::wids() const
{
    return pImpl->wids;
}

sched_policy::AppState AppInfo::appState() const
{
    return pImpl->state;
}

sched_policy::AppType AppInfo::appType() const
{
    return pImpl->type;
}

bool AppInfo::isAvailable() const
{
    return pImpl->isAvailable;
}

void AppInfo::setAppId(const std::string &appId)
{
    pImpl->appId = appId;
}

void AppInfo::setMrprisDbusService(const std::string &service)
{
    pImpl->mprisDbusService = service;
}

bool AppInfo::containsPid(int pid) const
{
    if (pImpl->launcherPid == pid) {
        return true;
    }

    auto allPids = pids();
    return std::find(allPids.begin(), allPids.end(), pid) != allPids.end();
}

bool AppInfo::containsWid(const std::string &wid) const
{
    return std::find(pImpl->wids.begin(), pImpl->wids.end(), wid) != pImpl->wids.end();
}

bool AppInfo::isSystemTrayIconApp() const
{
    return pImpl->isSystemTrayIconApp;
}

void AppInfo::appendWid(const std::string &wid)
{
    pImpl->wids.push_back(wid);
}

void AppInfo::removeWid(const std::string &wid)
{
    auto it = std::find(pImpl->wids.begin(), pImpl->wids.end(), wid);
    if (it != pImpl->wids.end()) {
        pImpl->wids.erase(it);
    }
}

void AppInfo::setAppState(sched_policy::AppState state)
{
    if (pImpl->state != state) {
        pImpl->state = state;
        if (pImpl->stateChangedCallback) {
            pImpl->stateChangedCallback(*this);
        }
    }
}

void AppInfo::setGroupName(const std::string &groupName)
{
    pImpl->groupName = groupName;
}

void AppInfo::setCmdline(const std::string &cmdline)
{
    pImpl->cmdline = cmdline;
}

void AppInfo::setIsSystemTrayIconApp(bool isSystemTrayIconApp)
{
    pImpl->isSystemTrayIconApp = isSystemTrayIconApp;
}

void AppInfo::setArgs(const std::vector<std::string> &args)
{
    pImpl->args = args;
}

void AppInfo::setAppStateChangedCallback(AppStateChangedCallback callback)
{
    pImpl->stateChangedCallback = std::move(callback);
}
