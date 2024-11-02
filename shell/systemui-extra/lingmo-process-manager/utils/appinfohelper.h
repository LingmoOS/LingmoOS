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

#ifndef APPINFOHELPER_H
#define APPINFOHELPER_H

#include <sys/types.h>
#include <vector>
#include <string>

namespace appinfo_helper {
    std::string generateDesktopFileId(const std::string &desktopFile);
    bool isFromSameProcess(const std::string &wid1, const std::string &wid2);
    bool areAllTheWindowMinimize(const std::vector<std::string> &wids);
    bool hasActiveWindow(const std::vector<std::string> &wids);
    int getPidByWid(const std::string &wid);
    bool isSameDesktopFile(const std::string &desktopFile1, const std::string &desktopFile2);
    bool hasSameDesktopName(const std::string &desktopFile1, const std::string &desktopFile2);
    bool hasSameMD5Hash(const std::string &desktopFile1, const std::string &desktopFile2);
    bool isKmreApp(const std::string &desktopFile);
    std::string getCmdlineFromDesktopFile(
        const std::string &desktopFile, const std::vector<std::string> &args);
    std::string findDesktopFileFromCmdline(const std::string &cmdline);
    // name.desktop
    std::string getDesktopFileName(const std::string &desktopFile);
    bool isTopApp(const std::string &desktopFile, const std::vector<std::string> &defaultTopApps);
    bool isSessionApp(const std::string &desktopFile, const std::vector<std::string> &availableFiles);
}

#endif // APPINFOHELPER_H
