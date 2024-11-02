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

#include "processinfohelper.h"
#include <QDebug>
#include <QFile>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <proc/readproc.h>

namespace process_info_helper {

std::string cmdline(int pid)
{
    if (pid <= 0) {
        return std::string();
    }
    std::string cmdlineFilePath = "/proc/" + std::to_string(pid) + "/cmdline";
    std::ifstream cmdlineFile(cmdlineFilePath);

    if (!cmdlineFile.is_open()) {
        qWarning() << "cmdlineFile open failed!" << pid;
        return std::string();
    }

    std::stringstream buffer;
    buffer << cmdlineFile.rdbuf();
    std::string cmdline = buffer.str();

    // Replace '\0' with ' '
    for (size_t i = 0; i < cmdline.length(); ++i) {
        if (cmdline[i] == '\0') {
            cmdline[i] = ' ';
        }
    }

    if (!cmdline.empty() && cmdline.at(cmdline.length() - 1) == ' ') {
        cmdline.pop_back();
    }

    return cmdline;
}

bool processExists(int pid)
{
    std::ifstream file("/proc/" + std::to_string(pid));
    return file.good();
}

int parentPid(int pid)
{
    QString procStatus = QString("/proc/%1/status").arg(pid);
    QFile file(procStatus);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Get parent id failed, " << pid << file.errorString();
        return -1;
    }
    QTextStream ts(&file);
    QString line;
    while (ts.readLineInto(&line)) {
        if (line.contains("PPid:")) {
            QString strPpid = line.split(":").last();
            strPpid.remove(QChar::Space);
            return strPpid.toUInt();
        }
    }
    return -1;
}

std::vector<int> childPids(int parentPid)
{
    std::vector<int> pids;
    PROCTAB *tab = openproc(PROC_FILLSTAT);
    proc_t proc_info;
    memset(&proc_info, 0, sizeof(proc_info));
    while (readproc(tab, &proc_info) != nullptr) {
        if (proc_info.ppid == parentPid) {
            pids.emplace_back(proc_info.tid);
        }
    }
    closeproc(tab);

    return pids;
}

std::vector<int> childPids(const std::vector<int> &parentPids)
{
    std::vector<int> childPids;
    std::unordered_map<int, int> mp;
    for (int i=0; i<parentPids.size(); ++i) {
        mp[parentPids[i]]++;
    }

    PROCTAB *tab = openproc(PROC_FILLSTAT);
    proc_t proc_info;
    memset(&proc_info, 0, sizeof(proc_info));
    while (readproc(tab, &proc_info) != nullptr) {
        if (mp.find(proc_info.ppid) != mp.end()) {
            childPids.emplace_back(proc_info.tid);
        }
    }
    closeproc(tab);

    if (childPids.empty()) {
        return childPids;
    }

    std::vector<int> nextSubPids = process_info_helper::childPids(childPids);
    if (nextSubPids.empty()) {
        return childPids;
    }

    for (const auto &pid : childPids) {
        if (std::find(nextSubPids.cbegin(), nextSubPids.cend(), pid) != nextSubPids.cend()) {
            nextSubPids.erase(std::remove(nextSubPids.begin(), nextSubPids.end(), pid), nextSubPids.end());
        }
    }

    childPids.insert(childPids.end(), nextSubPids.begin(), nextSubPids.end());

    return childPids;
}

std::string cgroup(int pid)
{
    if (pid <= 0) {
        return std::string();
    }
    std::string cgroupFilePath = "/proc/" + std::to_string(pid) + "/cgroup";
    std::ifstream cgroupFile(cgroupFilePath);

    if (!cgroupFile.is_open()) {
        qWarning() << "cgroupFile open failed!" << pid;
        return std::string();
    }

    std::string cgroup, line;
    while (std::getline(cgroupFile, line)) {
        auto lastPos = line.rfind("::");
        if (lastPos != std::string::npos) {
            cgroup = line.substr(lastPos + 2);
            break;
        }
    }
    cgroupFile.close();
    return cgroup;
}

std::vector<int> pidsOfCgroup(const std::string &cgroupPath)
{
    std::ifstream cgroupFile(cgroupPath + "/cgroup.procs");

    if (!cgroupFile.is_open()) {
        return {};
    }

    std::string line;
    std::vector<int> pids;
    while (std::getline(cgroupFile, line)) {
        pids.emplace_back(std::stoi(line));
    }
    cgroupFile.close();
    return pids;
}

}
