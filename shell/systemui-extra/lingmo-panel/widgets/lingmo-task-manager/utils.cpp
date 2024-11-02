/*
 *
 *  * Copyright (C) 2023, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include "utils.h"
#include <mutex>

#include "window-manager.h"
static LingmoUISearch::ApplicationInfo *applicationInfo = nullptr;
static std::once_flag flag;
LingmoUISearch::ApplicationInfo *Utils::getApplicationInfo()
{
    std::call_once(flag, [ & ] {
        applicationInfo = new LingmoUISearch::ApplicationInfo();
    });
    return applicationInfo;
}
QString Utils::desktopFileFromPid(uint pid)
{
    QString desktopFile;
    getApplicationInfo()->tranPidToDesktopFp(pid, desktopFile);
    return desktopFile;
}

QString Utils::desktopFileFromWid(const QString &wid)
{
    QString desktopFile;
    getApplicationInfo()->desktopFilePathFromName(LingmoUIQuick::WindowManager::appId(wid), desktopFile);
    if(desktopFile.isEmpty()) {
        getApplicationInfo()->tranPidToDesktopFp(LingmoUIQuick::WindowManager::pid(wid),desktopFile);
    }
    return desktopFile;
}


