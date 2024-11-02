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

#include "exceptiongroupprocesswatcher.h"
#include <QDebug>

ExceptionGroupProcessWatcher::ExceptionGroupProcessWatcher(
    const std::shared_ptr<ProcessInfoManager> &processInfoManager, QObject *parent)
    : QObject{parent}
    , m_processInfoManager(processInfoManager)
    , m_watcherRunning(false)
{
    qRegisterMetaType<std::string>("std::string");
}
void ExceptionGroupProcessWatcher::addPath(std::string path, sched_policy::GroupType groupType)
{
    if (m_watchedGroupPaths.find(path) != m_watchedGroupPaths.end()) {
        qDebug() << QString("path '%1' already exists").arg(QString::fromStdString(path));
        return;
    }
    m_watchedGroupPaths[path] = { path, groupType };
}

void ExceptionGroupProcessWatcher::stopWatcher()
{
    m_watcherRunning = false;
    m_stopWatcherConditionVar.notify_one();
}

void ExceptionGroupProcessWatcher::startWatcher()
{
    m_watcherRunning = true;
    m_watcherFuture = std::async(std::launch::async, [this]() {
        int sleepDuration = 1;
        while (m_watcherRunning) {
            for (auto &groupInfos : m_watchedGroupPaths) {
                auto pids = readFileToPids(groupInfos.second.path + "/cgroup.procs");
                auto exceptionPids = findExceptionPids(groupInfos.second.pids, pids);
                handleExceptionPids(exceptionPids, groupInfos.second.groupType);
                groupInfos.second.pids = pids;
            }
            std::unique_lock<std::mutex> lock(m_mutex);
            m_stopWatcherConditionVar.wait_for(lock, std::chrono::seconds(sleepDuration));
            sleepDuration = sleepDuration > 20 ? 20 : sleepDuration + 1;
        }
    });
}

std::vector<int> ExceptionGroupProcessWatcher::readFileToPids(const std::string &file) const
{
    std::vector<int> pids;
    std::ifstream ifs(file);
    if (!ifs.is_open()) {
        std::cout << "open file failed" << std::endl;
        return pids;
    }
    std::string line;
    while (std::getline(ifs, line)) {
        std::stringstream ss(line);
        int pid;
        ss >> pid;
        pids.push_back(pid);
    }
    ifs.close();
    return pids;
}

std::vector<int> ExceptionGroupProcessWatcher::findExceptionPids(const std::vector<int> &oldPids, const std::vector<int> &newPids) const
{
    std::vector<int> res;
    std::unordered_map<int, int> mp;
    for (int i=0; i<oldPids.size(); ++i) {
        mp[oldPids[i]]++;
    }
    for (int i=0; i<newPids.size(); ++i) {
        if (mp.find(newPids[i]) == mp.end() || mp[newPids[i]] == 0) {
            res.push_back(newPids[i]);
        } else {
            mp[newPids[i]]--;
        }
    }
    return res;
}

void ExceptionGroupProcessWatcher::handleExceptionPids(const std::vector<int> &pids, sched_policy::GroupType groupType)
{
    for (const auto &pid : pids) {
        auto appType = m_processInfoManager->getAppTypeByPid(pid);
        auto desktop = m_processInfoManager->syncGetDesktopFileByPid(pid);
        // only watch session scope group
        if (groupType == sched_policy::GroupType::SessionScopeGroup &&
            appType == sched_policy::AppType::Session) {
            qDebug() << "exceptionSessionAppCaught" << QString::fromStdString(desktop) << pid;
            Q_EMIT exceptionSessionAppCaught(desktop, pid);
        }
    }
}
