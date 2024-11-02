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

#ifndef EXCEPTIONGROUPPROCESSWATCHER_H
#define EXCEPTIONGROUPPROCESSWATCHER_H

#include <QObject>
#include <string>
#include <thread>
#include <future>
#include <iostream>
#include <map>
#include <vector>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <atomic>
#include <condition_variable>
#include "schedpolicy.h"
#include "processinfomanager.h"

class ExceptionGroupProcessWatcher : public QObject
{
    Q_OBJECT
public:
    explicit ExceptionGroupProcessWatcher(
        const std::shared_ptr<ProcessInfoManager> &processInfoManager, QObject *parent = nullptr);
    void addPath(std::string path, sched_policy::GroupType groupType);
    void startWatcher();
    void stopWatcher();

Q_SIGNALS:
    void exceptionSessionAppCaught(const std::string &desktopFile, int pid);

private:
    std::vector<int> readFileToPids(const std::string &file) const;
    std::vector<int> findExceptionPids(const std::vector<int> &oldPids, const std::vector<int> &newPids) const;
    void handleExceptionPids(const std::vector<int> &pids, sched_policy::GroupType groupType);

private:
    struct WatchedGroupInfo {
        std::string path;
        sched_policy::GroupType groupType;
        std::vector<int> pids;
    };

private:
    std::map<std::string, WatchedGroupInfo> m_watchedGroupPaths;
    std::atomic_bool m_watcherRunning;
    std::future<void> m_watcherFuture;
    std::condition_variable m_stopWatcherConditionVar;
    std::mutex m_mutex;
    std::shared_ptr<ProcessInfoManager> m_processInfoManager;
};

#endif // EXCEPTIONGROUPPROCESSWATCHER_H
