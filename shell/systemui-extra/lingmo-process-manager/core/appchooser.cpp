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

#include "appchooser.h"
#include <QMimeDatabase>
#include <QMimeType>
#include <QStandardPaths>
#include <QDebug>
#include <gio/gdesktopappinfo.h>
#include <gio/gappinfo.h>

namespace {

void eachAppInfoCallback(gpointer data, gpointer userData)
{
    std::vector<std::string> *dstAppDesktopNameList =
        static_cast<std::vector<std::string> *>(userData);

    GAppInfo *appInfo = static_cast<GAppInfo *>(data);

    if (!G_IS_APP_INFO(appInfo) || dstAppDesktopNameList == nullptr) {
        return;
    }

    const std::string appDesktopId = g_app_info_get_id(appInfo);
    QString localDesktopFile =
        QStandardPaths::locate(QStandardPaths::ApplicationsLocation, QString::fromStdString(appDesktopId));

    dstAppDesktopNameList->emplace_back(localDesktopFile.toStdString());
}

}

std::string AppChooser::getDefaultAppForUrl(const std::string &url)
{
    GAppInfo *appInfo = nullptr;
    char *uriScheme = g_uri_parse_scheme(url.c_str());
    if (uriScheme && uriScheme[0] != '\0') {
        appInfo = g_app_info_get_default_for_uri_scheme(uriScheme);
    }
    g_free(uriScheme);

    if (!appInfo) {
        GFile *file = g_file_new_for_uri(url.c_str());
        appInfo = g_file_query_default_handler (file, nullptr, nullptr);
        g_object_unref(file);
    }

    if (appInfo) {
        QString appId = g_app_info_get_id(appInfo);
        g_object_unref (appInfo);
        QString desktopFile = QStandardPaths::locate(QStandardPaths::ApplicationsLocation, appId);
        return desktopFile.toStdString();
    }

    return std::string();
}

std::vector<std::string> AppChooser::getAvailableAppListForFile(const std::string &fileName)
{
    const std::string contentType = getMimeContentTypeFromFile(fileName);
    GList *appList = g_app_info_get_all_for_type(contentType.c_str());

    if (appList == nullptr) {
        return {};
    }

    std::vector<std::string> availableAppList;
    g_list_foreach(appList, eachAppInfoCallback, static_cast<gpointer>(&availableAppList));

    g_list_free_full (appList, g_object_unref);

    return availableAppList;
}

std::string AppChooser::getMimeContentTypeFromFile(const std::string &fileName)
{
    QMimeDatabase mimeDatabase;
    QMimeType mimeType = mimeDatabase.mimeTypeForFile(QString::fromStdString(fileName));
    return mimeType.name().toStdString();
}
