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

#ifndef APPLAUNCHER_H
#define APPLAUNCHER_H

#include "base/appinfo.h"

#include <string>
#include <functional>

#include <gio/gdesktopappinfo.h>
#include <gio/gappinfo.h>

class AppInfoGetter {
public:
    using LatestAppInfoByDesktopFileCallback =
        std::function<AppInfo(const std::string &, const std::vector<std::string> &)>;

    using LatestAppInfoByCmdlineCallback =
        std::function<AppInfo(const std::string &)>;

    AppInfoGetter(
        LatestAppInfoByDesktopFileCallback byDesktopFileCallback,
        LatestAppInfoByCmdlineCallback byCmdlineCallback);

    AppInfo getLatestAppInfoByDesktopFile(
        const std::string &desktopFile, const std::vector<std::string> &args) const;

    AppInfo getLatestAppInfoByCmdline(const std::string &cmdline) const;

private:
    LatestAppInfoByDesktopFileCallback m_latestAppInfoByDesktopFileCallback;
    LatestAppInfoByCmdlineCallback m_latestAppInfoByCmdlineCallback;
};


class AppLauncher
{
public:
    // desktop file, args, .pid
    using LauncherProcessCallback =
        std::function<void(const std::string &, const std::vector<std::string> &args, int)>;

    enum class LaunchError {
        NoError,
        FailedToFindDesktopFile,
        FailedToCreateDesktopAppInfo,
        FailedToActiveWindow,
        FailedToLaunchApp,
        FailedToParseCommand,
        FailedToRunCommand,
    };

    struct LaunchErrorMessage {
        LaunchError error;
        std::string errorName;
        std::string errorMessage;
    };

    AppLauncher(
        LauncherProcessCallback launcherProcessCallback,
        AppInfoGetter appInfoGetter);

    void setKeepAppSingleInstance(bool keepAppSingleInstance);

    bool launchApp(const std::string &desktopFile);
    bool launchAppWithArguments(
        const std::string &desktopFile, const std::vector<std::string> &args);
    bool runCommand(const std::string &command);

    LaunchError error() const;
    LaunchErrorMessage errorMessage() const;

private:
    friend void childProcessCallback(GDesktopAppInfo *appInfo, GPid pid, gpointer userData);

    bool launchDesktopApp(
        const std::string &desktopFile, const std::vector<std::string> &args = {});

    bool shouldLaunchNewInstance(
        const std::string &desktopFile, const std::vector<std::string> &args = {});

    bool appWindowHasCreated(const AppInfo &appInfo);
    bool appWindowIsCreating(const AppInfo &appInfo);

    bool doLaunchApp(
        const std::string &desktopFile, const std::vector<std::string> &args = {});

    bool activeAppWindow(
        const std::string &desktopFile, const std::vector<std::string> &args = {});
    bool activeAppWindow(const AppInfo &appInfo);

    GList *createUrisFromStringList(const std::vector<std::string> &args);

    bool runNewCommandInstance(const std::string &command);

    void initLaunchError();
    void setLaunchError(LaunchError launchError);

private:
    LauncherProcessCallback m_launcherProcessCallback;
    AppInfoGetter m_appInfoGetter;
    bool m_keepAppSingleInstance;
    LaunchError m_launchError;
};

#endif // APPLAUNCHER_H
