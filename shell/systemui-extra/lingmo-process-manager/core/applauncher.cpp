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

#include "applauncher.h"
#include <QDateTime>
#include <QDebug>
#include <QProcess>
#include <windowmanager/windowmanager.h>
#include "appinfohelper.h"
#include "misc.h"

namespace {

struct ChildProcessCallbackUserData
{
    AppLauncher *appLauncher;
    std::vector<std::string> args;
};

}

AppInfoGetter::AppInfoGetter(
    LatestAppInfoByDesktopFileCallback byDesktopFileCallback,
    LatestAppInfoByCmdlineCallback byCmdlineCallback)
    : m_latestAppInfoByDesktopFileCallback(std::move(byDesktopFileCallback))
    , m_latestAppInfoByCmdlineCallback(std::move(byCmdlineCallback))
{

}

AppInfo AppInfoGetter::getLatestAppInfoByDesktopFile(
    const std::string &desktopFile, const std::vector<std::string> &args) const
{
    return m_latestAppInfoByDesktopFileCallback(desktopFile, args);
}

AppInfo AppInfoGetter::getLatestAppInfoByCmdline(const std::string &cmdline) const
{
    return m_latestAppInfoByCmdlineCallback(cmdline);
}

void childProcessCallback(GDesktopAppInfo *appInfo, GPid pid, gpointer userData)
{
    ChildProcessCallbackUserData *callbackUserData =
        static_cast<ChildProcessCallbackUserData *>(userData);
    if (!callbackUserData || !appInfo ||
        !callbackUserData->appLauncher->m_launcherProcessCallback) {
        return;
    }
    callbackUserData->appLauncher->m_launcherProcessCallback(
        g_desktop_app_info_get_filename(appInfo), callbackUserData->args, pid);
}

AppLauncher::AppLauncher(
    LauncherProcessCallback launcherProcessCallback,
    AppInfoGetter appInfoGetter)
    : m_keepAppSingleInstance(false)
    , m_launcherProcessCallback(std::move(launcherProcessCallback))
    , m_appInfoGetter(std::move(appInfoGetter))
{
}

void AppLauncher::setKeepAppSingleInstance(bool keepAppSingleInstance)
{
    m_keepAppSingleInstance = keepAppSingleInstance;
}

bool AppLauncher::launchApp(const std::string &desktopFile)
{
    return launchDesktopApp(desktopFile);
}

bool AppLauncher::launchAppWithArguments(
    const std::string &desktopFile, const std::vector<std::string> &args)
{
    return launchDesktopApp(desktopFile, args);
}

bool AppLauncher::runCommand(const std::string &command)
{
    if (m_keepAppSingleInstance) {
        AppInfo appInfo = m_appInfoGetter.getLatestAppInfoByCmdline(command);
        if (appWindowHasCreated(appInfo)) {
            return activeAppWindow(appInfo);
        }
    }

    return runNewCommandInstance(command);
}

AppLauncher::LaunchError AppLauncher::error() const
{
    return m_launchError;
}

AppLauncher::LaunchErrorMessage AppLauncher::errorMessage() const
{
    switch (m_launchError) {
    case LaunchError::FailedToFindDesktopFile:
        return { m_launchError,
                "FailedToFindDesktopFile", "Failed to find the desktop file."};

    case LaunchError::FailedToCreateDesktopAppInfo:
        return { m_launchError,
                "FailedToCreateDesktopAppInfo", "Failed to create DesktopAppInfo."};

    case LaunchError::FailedToActiveWindow:
        return { m_launchError,
                "FailedToActiveWindow", "Failed to active application window."};

    case LaunchError::FailedToLaunchApp:
        return { m_launchError,
                "FailedToLaunchApp", "Failed to launch the application."};

    case LaunchError::FailedToParseCommand:
        return { m_launchError,
                "FailedToParseCommand", "Failed to create app info from command."};

    case LaunchError::FailedToRunCommand:
        return { m_launchError,
                "FailedToRunCommand", "Failed to run command."};

    default:
        return { m_launchError,
                "NoError", "No error occurred." };
    }
}

bool AppLauncher::launchDesktopApp(
    const std::string &desktopFile, const std::vector<std::string> &args)
{
    initLaunchError();

    if (!misc::file::fileExists(desktopFile)) {
        setLaunchError(LaunchError::FailedToFindDesktopFile);
        return false;
    }

    if (shouldLaunchNewInstance(desktopFile, args)) {
        return doLaunchApp(desktopFile, args);
    }

    return activeAppWindow(desktopFile, args);
}

bool AppLauncher::shouldLaunchNewInstance(
    const std::string &desktopFile, const std::vector<std::string> &args)
{
    if (!m_keepAppSingleInstance) {
        return true;
    }

    if (appinfo_helper::isKmreApp(desktopFile)) {
        return true;
    }

    AppInfo appInfo = m_appInfoGetter.getLatestAppInfoByDesktopFile(desktopFile, args);

    if (appWindowHasCreated(appInfo)) {
        return false;
    }

    if (appWindowIsCreating(appInfo)) {
        return false;
    }

    return true;
}

bool AppLauncher::appWindowHasCreated(const AppInfo &appInfo)
{
    return !appInfo.wids().empty();
}

bool AppLauncher::appWindowIsCreating(const AppInfo &appInfo)
{
    return appInfo.wids().empty() &&
           QDateTime::currentSecsSinceEpoch() - appInfo.launchTimestamp() <= 5;
}

bool AppLauncher::doLaunchApp(
    const std::string &desktopFile, const std::vector<std::string> &args)
{
    GDesktopAppInfo *appInfo = g_desktop_app_info_new_from_filename(desktopFile.c_str());
    if (appInfo == nullptr) {
        setLaunchError(LaunchError::FailedToCreateDesktopAppInfo);
        qWarning() << QString("Create DesktopAppInfo failed from desktop %1.").arg(
            QString::fromStdString(desktopFile));
        return false;
    }

    GList *uris = createUrisFromStringList(args);

    GError *error = nullptr;
    ChildProcessCallbackUserData userData { this, args };
    GAppLaunchContext *appLaunchContext = g_app_launch_context_new();
    g_app_launch_context_unsetenv(appLaunchContext, "QT_QPA_PLATFORM");
    bool launched = g_desktop_app_info_launch_uris_as_manager(
        appInfo, uris, appLaunchContext, G_SPAWN_DEFAULT, nullptr, nullptr, childProcessCallback, &userData, &error);

    if (!launched) {
        setLaunchError(LaunchError::FailedToLaunchApp);
        qWarning() << QString("Failed to launch %1, %2.").arg(
            QString::fromStdString(desktopFile), error->message);
        g_clear_error(&error);
    }

    g_list_free(uris);
    g_object_unref(appInfo);

    return launched;
}

bool AppLauncher::activeAppWindow(
    const std::string &desktopFile, const std::vector<std::string> &args)
{
    AppInfo appInfo = m_appInfoGetter.getLatestAppInfoByDesktopFile(desktopFile, args);
    return activeAppWindow(appInfo);
}

bool AppLauncher::activeAppWindow(const AppInfo &appInfo)
{
    if (appInfo.wids().empty()) {
        setLaunchError(LaunchError::FailedToActiveWindow);
        return false;
    }

    const std::string latestWid = appInfo.wids().back();
    kdk::WindowManager::self()->activateWindow(QVariant::fromValue(QString::fromStdString(latestWid)));
    return true;
}

GList *AppLauncher::createUrisFromStringList(const std::vector<std::string> &args)
{
    GList *uris = nullptr;
    for (const auto &arg : args) {
        uris = g_list_append(uris, const_cast<char *>(arg.c_str()));
    }

    return uris;
}

bool AppLauncher::runNewCommandInstance(const std::string &command)
{
    initLaunchError();

    auto qpaEnv = qgetenv("QT_QPA_PLATFORM");
    if (!qpaEnv.isEmpty()) {
        qunsetenv("QT_QPA_PLATFORM");
    }
    bool ret = QProcess::startDetached(QString::fromStdString(command));
    if (!qpaEnv.isEmpty()) {
        qputenv("QT_QPA_PLATFORM", qpaEnv);
    }

    if (!ret) {
        setLaunchError(LaunchError::FailedToRunCommand);
        qWarning() << QString("Run new commmand '%1' instance failed.").arg(command.c_str());
    }

    return ret;
}

void AppLauncher::initLaunchError()
{
    setLaunchError(AppLauncher::LaunchError::NoError);
}

void AppLauncher::setLaunchError(LaunchError launchError)
{
    m_launchError = std::move(launchError);
}
