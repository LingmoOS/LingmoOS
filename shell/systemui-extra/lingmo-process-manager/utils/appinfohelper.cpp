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

#include "appinfohelper.h"
#include <QCryptographicHash>
#include <QFile>
#include <QStandardPaths>
#include <QDebug>
#include <kdesktopfile.h>
#include <kconfiggroup.h>
#include <fstream>
#include <windowmanager/windowmanager.h>
#include <gio/gdesktopappinfo.h>
#include "misc.h"

namespace {

const char *xdg_auto_start_path = "/etc/xdg/autostart";

std::vector<std::string> searchAppDesktopFiles(const std::string &searchStr)
{
    std::vector<std::string> appDesktopFiles;

    char ***results = g_desktop_app_info_search (searchStr.c_str());
    char ***groups, **desktopNames;

    for (groups = results; *groups; ++ groups) {
        for (desktopNames = *groups; *desktopNames; ++ desktopNames) {
            if (!g_utf8_validate (*desktopNames, -1, nullptr)) {
                **desktopNames = '\0';
            }
            appDesktopFiles.push_back(*desktopNames);
        }
    }

    for (int i = 0; results[i]; i++) {
        g_strfreev (results[i]);
    }
    g_free (results);

    return appDesktopFiles;
}

}

bool appinfo_helper::isFromSameProcess(const std::string &wid1, const std::string &wid2)
{
    return getPidByWid(wid1) == getPidByWid(wid2);
}

bool appinfo_helper::areAllTheWindowMinimize(const std::vector<std::string> &wids)
{
    for (const auto &wid : wids) {
        auto windowInfo = kdk::WindowManager::getwindowInfo(QVariant::fromValue(QString::fromStdString(wid)));
        if (windowInfo.isValid() && !windowInfo.isMinimized()) {
            return false;
        }
    }
    return true;
}

bool appinfo_helper::hasActiveWindow(const std::vector<std::string> &wids)
{
    const std::string activeWindow = kdk::WindowManager::currentActiveWindow().toString().toStdString();
    auto it = std::find(wids.cbegin(), wids.cend(), activeWindow);
    return it != wids.cend();
}

int appinfo_helper::getPidByWid(const std::string &wid)
{
    return kdk::WindowManager::getPid(QVariant::fromValue(QString::fromStdString(wid)));
}

bool appinfo_helper::isSameDesktopFile(const std::string &desktopFile1, const std::string &desktopFile2)
{
    if ((desktopFile1 == desktopFile2) && !desktopFile1.empty()) {
        return true;
    }

    return hasSameDesktopName(desktopFile1, desktopFile2) &&
           hasSameMD5Hash(desktopFile1, desktopFile2);
}

bool appinfo_helper::hasSameDesktopName(const std::string &desktopFile1, const std::string &desktopFile2)
{
    auto desktopName1 = misc::file::getFileNameFromPath(desktopFile1);
    auto desktopName2 = misc::file::getFileNameFromPath(desktopFile2);

    return (desktopName1 == desktopName2) && !desktopName1.empty();
}

bool appinfo_helper::hasSameMD5Hash(const std::string &desktopFile1, const std::string &desktopFile2)
{
    auto file1MD5Hash = misc::file::fileMD5Hash(desktopFile1);
    auto file2MD5Hash = misc::file::fileMD5Hash(desktopFile2);

    return (file1MD5Hash == file2MD5Hash) && !file1MD5Hash.isEmpty();
}

bool appinfo_helper::isKmreApp(const std::string &desktopFile)
{
    if (!misc::file::fileExists(desktopFile)) {
        return false;
    }

    GDesktopAppInfo *appInfo = g_desktop_app_info_new_from_filename(desktopFile.c_str());
    if (appInfo == nullptr) {
        return false;
    }

    QString categories = g_desktop_app_info_get_categories(appInfo);
    return categories.contains("Android") || categories.contains("Apk");
}

std::string appinfo_helper::generateDesktopFileId(const std::string &desktopFile)
{
    QCryptographicHash hash(QCryptographicHash::Md5);
    QFile file(QString::fromStdString(desktopFile));
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << QString("Generate id failed, the desktop file '%1' dose not exist.").arg(QString::fromStdString(desktopFile));
        return std::string();
    }
    hash.addData(file.readAll());
    return hash.result().toHex().toStdString();
}

std::string appinfo_helper::findDesktopFileFromCmdline(const std::string &cmdline)
{
    auto cmdlineList = misc::string::split(cmdline, ' ');
    const std::string execName = cmdlineList.empty() ? cmdline : cmdlineList.front();

    auto appDesktopFiles = searchAppDesktopFiles(execName);
    if (appDesktopFiles.empty()) {
        return std::string();
    }

    auto it = std::find_if(appDesktopFiles.cbegin(), appDesktopFiles.cend(),
                           [execName](const std::string &desktopFile) {
        return desktopFile == execName + ".desktop";
    });

    if (it != appDesktopFiles.cend()) {
        auto desktopFile = QStandardPaths::locate(
            QStandardPaths::ApplicationsLocation, QString::fromStdString(*it));
        return desktopFile.toStdString();
    }

    return appDesktopFiles.front();
}

std::string appinfo_helper::getDesktopFileName(const std::string &desktopFile)
{
    auto filePathElements = misc::string::split(desktopFile, '/');
    if (filePathElements.empty()) {
        qWarning() << QString("Get the desktop file's ('%1') path elements failed.")
                         .arg(QString::fromStdString(desktopFile));
        return std::string();
    }

    return filePathElements.back();
}

bool appinfo_helper::isTopApp(
    const std::string &desktopFile, const std::vector<std::string> &defaultTopApps)
{
    for (const auto &topAppDesktopFile : defaultTopApps) {
        if (appinfo_helper::isSameDesktopFile(topAppDesktopFile, desktopFile)) {
            return true;
        }
    }

    return false;
}

bool appinfo_helper::isSessionApp(
    const std::string &desktopFile, const std::vector<std::string> &availableFiles)
{
    // only show in autostart path
    if (desktopFile.find_first_of(xdg_auto_start_path) != 0 ||
        availableFiles.size() > 1) {
        return false;
    }

    KDesktopFile kDesktopFile(QString::fromStdString(desktopFile));
    KConfigGroup desktopGroup = kDesktopFile.desktopGroup();

    if (desktopGroup.readEntry("OnlyShowIn").contains("LINGMO") ||
        desktopGroup.readEntry("X-LINGMO-Autostart-Phase").contains("WindowManager") ||
        desktopGroup.readEntry("X-LINGMO-Autostart-Phase").contains("Initialization")) {
        return true;
    }

    return false;
}

std::string appinfo_helper::getCmdlineFromDesktopFile(
    const std::string &desktopFile, const std::vector<std::string> &args)
{
    GDesktopAppInfo *appInfo = g_desktop_app_info_new_from_filename(desktopFile.c_str());
    if (appInfo == nullptr) {
        return std::string();
    }

    QString exec = g_desktop_app_info_get_locale_string(appInfo, "Exec");
    exec.remove(QRegExp("\\s%\\w"));

    exec += ' ';
    for (const auto &arg : args) {
        exec += QString::fromStdString(arg);
    }

    return exec.toStdString();
}
