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

#ifndef PROCESSINFOHELPER_H
#define PROCESSINFOHELPER_H

#include <string>
#include <vector>

namespace process_info_helper {

std::string cmdline(int pid);
bool processExists(int pid);
int parentPid(int pid);
std::vector<int> childPids(int parentPid);
std::vector<int> childPids(const std::vector<int> &parentPids);
std::string cgroup(int pid);
std::vector<int> pidsOfCgroup(const std::string &cgroupPath);

}

#endif // PROCESSINFOHELPER_H
