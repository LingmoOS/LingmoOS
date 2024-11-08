/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "file-launch-manager.h"
#include "file-utils.h"
#include "file-info.h"
#include "file-info-job.h"
#include <QStandardPaths>
#include "file-launch-action.h"

#include <gio/gdesktopappinfo.h>
#include <QUrl>

#include <QTimer>
#include <QSettings>
#include <QTextCodec>

using namespace Peony;

static bool glib_force_use_portal = false;
static bool init_glib_env = false;

FileLaunchManager::FileLaunchManager(QObject *parent) : QObject(parent)
{

}

FileLaunchAction *FileLaunchManager::getDefaultAction(const QString &uri)
{
    //FIXME: replace BLOCKING api in ui thread.
    auto info = FileInfo::fromUri(uri);
    QString mimeType = info->mimeType();

    if (mimeType.isEmpty()) {
        FileInfoJob job(info);
        job.querySync();
        mimeType = info->mimeType();
    }

    if (info->canExecute() && info->uri().endsWith(".desktop")) {
        QUrl url = uri;
        auto path = url.path();
        bool isMdmApp = false;
        qDebug() << "getDefaultAction uri:"<<uri;
        GDesktopAppInfo *info = g_desktop_app_info_new_from_filename(path.toUtf8().constData());
        //fix bug#130690, set mdm app can not get info, open tips not correct issue
        if (!info) {
            GKeyFile *keyfile = g_key_file_new();
            g_key_file_load_from_file(keyfile, path.toUtf8().data(), G_KEY_FILE_NONE, nullptr);
            g_autofree gchar* execmd = g_key_file_get_string(keyfile, "Desktop Entry", "Exec", nullptr);
            QString cmd = execmd;
            // 通过lingmo-menu的配置文件判断
            QString settingsPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.cache/lingmo-menu/lingmo-menu.ini";
            QSettings settings(settingsPath, QSettings::IniFormat);
            auto g = settings.childGroups();
            auto k = settings.allKeys();
            settings.setIniCodec(QTextCodec::codecForName("utf-8"));
            settings.beginGroup("application");
            bool isExist = settings.contains(execmd);
            bool notDisable = true;
            if (isExist) {
                notDisable = settings.value(execmd).toBool();
            }
            settings.endGroup();

            qDebug() << "getDefaultAction empty info cmd:"<<cmd<<isExist<<notDisable;
            if (isExist && !notDisable) {
                isMdmApp = true;
            }
        }
        FileLaunchAction *action = new FileLaunchAction(uri, G_APP_INFO(info));
        qDebug() << "getDefaultAction isMdmApp:"<<isMdmApp;
        if (isMdmApp) {
            action->setProperty("isMdmApp", isMdmApp);
        }
        g_object_unref(info);
        return action;
    } else {
        GError *error = NULL;
        GAppInfo *info  = NULL;
        bool isMdmApp = false;
        /*
        * g_app_info_get_default_for_type function get wrong default app, so we get the
        * default app info from mimeapps.list, and chose the right default app for mimeType file
        */
        QString mimeAppsListPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
           + "/.config/mimeapps.list";
        GKeyFile *keyfile = g_key_file_new ();
        gboolean ret = g_key_file_load_from_file (keyfile, mimeAppsListPath.toUtf8(), G_KEY_FILE_NONE, &error);
        if (false == ret) {
            qWarning()<<"load mimeapps list error msg"<<error->message;
            info = g_app_info_get_default_for_type(mimeType.toUtf8().constData(), false);
            g_error_free(error);
        } else {
            // 需要匹配应用是否被禁用
            gchar *desktopApp = g_key_file_get_string (keyfile, "Default Applications", mimeType.toUtf8(), &error);
            if (NULL != desktopApp) {
                QString desktopFile = QString("/usr/share/applications/") + desktopApp;
                GKeyFile *desktop_key_file = g_key_file_new();
                if (g_key_file_load_from_file(desktop_key_file, desktopFile.toUtf8().constData(), G_KEY_FILE_NONE, nullptr)) {
                    g_autofree gchar* execmd = g_key_file_get_string(desktop_key_file, "Desktop Entry", "Exec", nullptr);
                    QString cmd = execmd;
                    // 通过lingmo-menu的配置文件判断
                    QString settingsPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.cache/lingmo-menu/lingmo-menu.ini";
                    QSettings settings(settingsPath, QSettings::IniFormat);
                    auto g = settings.childGroups();
                    auto k = settings.allKeys();
                    settings.setIniCodec(QTextCodec::codecForName("utf-8"));
                    settings.beginGroup("application");
                    bool isExist = settings.contains(execmd);
                    bool notDisable = true;
                    if (isExist) {
                        notDisable = settings.value(execmd).toBool();
                    }
                    settings.endGroup();

                    if (isExist && !notDisable) {
                        isMdmApp = true;
                    }
                }
                g_key_file_free(desktop_key_file);

                info = (GAppInfo*)g_desktop_app_info_new(desktopApp);
                g_free (desktopApp);
            } else {
                info = g_app_info_get_default_for_type(mimeType.toUtf8().constData(), false);
                auto execmd = g_app_info_get_commandline(info);
                QString settingsPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.cache/lingmo-menu/lingmo-menu.ini";
                QSettings settings(settingsPath, QSettings::IniFormat);
                auto g = settings.childGroups();
                auto k = settings.allKeys();
                settings.setIniCodec(QTextCodec::codecForName("utf-8"));
                settings.beginGroup("application");
                bool isExist = settings.contains(execmd);
                bool notDisable = true;
                if (isExist) {
                    notDisable = settings.value(execmd).toBool();
                }
                settings.endGroup();

                if (isExist && !notDisable) {
                    isMdmApp = true;
                }
            }
        }

        g_key_file_free (keyfile);

        FileLaunchAction *action = new FileLaunchAction(uri, info);
        action->setProperty("isMdmApp", isMdmApp);
        g_object_unref(info);

        return action;
    }
}

const QList<FileLaunchAction*> FileLaunchManager::getRecommendActions(const QString &uri)
{
    //FIXME: replace BLOCKING api in ui thread.
    auto info = FileInfo::fromUri(uri);
    QString mimeType = info->mimeType();
    if (mimeType.isEmpty()) {
        FileInfoJob job(info);
        job.querySync();
        mimeType = info->mimeType();
    }
    GList *app_infos = g_app_info_get_recommended_for_type(mimeType.toUtf8().constData());
    GList *l = app_infos;
    QList<FileLaunchAction *> actions;
    while (l) {
        auto app_info = static_cast<GAppInfo*>(l->data);
        actions<<new FileLaunchAction(uri, app_info, true);
        g_object_unref(app_info);
        l = l->next;
    }
    return actions;
}

const QList<FileLaunchAction*> FileLaunchManager::getFallbackActions(const QString &uri)
{
    //FIXME: replace BLOCKING api in ui thread.
    auto info = FileInfo::fromUri(uri);
    QString mimeType = info->mimeType();
    if (mimeType.isEmpty()) {
        FileInfoJob job(info);
        job.querySync();
        mimeType = info->mimeType();
    }
    GList *app_infos = g_app_info_get_fallback_for_type(mimeType.toUtf8().constData());
    GList *l = app_infos;
    QList<FileLaunchAction *> actions;
    while (l) {
        auto app_info = static_cast<GAppInfo*>(l->data);
        actions<<new FileLaunchAction(uri, app_info, true);
        g_object_unref(app_info);
        l = l->next;
    }
    return actions;
}

const QList<FileLaunchAction*> FileLaunchManager::getAllActionsForType(const QString &uri)
{
    //FIXME: replace BLOCKING api in ui thread.
    auto info = FileInfo::fromUri(uri);
    QString mimeType = info->mimeType();
    if (mimeType.isEmpty()) {
        FileInfoJob job(info);
        job.querySync();
        mimeType = info->mimeType();
    }
    GList *app_infos = g_app_info_get_all_for_type(mimeType.toUtf8().constData());
    GList *l = app_infos;
    QList<FileLaunchAction *> actions;
    while (l) {
        auto app_info = static_cast<GAppInfo*>(l->data);
        //only show available applications
        if (g_app_info_should_show(app_info))
           actions<<new FileLaunchAction(uri, app_info, true);
        g_object_unref(app_info);
        l = l->next;
    }
    return actions;
}

const QList<FileLaunchAction*> FileLaunchManager::getAllActions(const QString &uri)
{
    GList *app_infos = g_app_info_get_all();
    GList *l = app_infos;
    QList<FileLaunchAction *> actions;
    while (l) {
        auto app_info = static_cast<GAppInfo*>(l->data);
        //only show available applications
        if (g_app_info_should_show(app_info))
           actions<<new FileLaunchAction(uri, app_info, true);
        g_object_unref(app_info);
        l = l->next;
    }
    return actions;
}

void FileLaunchManager::openSync(const QString &uri, bool forceWithArg, bool skipDialog)
{
    QString tmp = uri;
    auto targetUri = FileUtils::getTargetUri(uri);
    if (targetUri.isNull()) {
        tmp = targetUri;
    }
    auto action = getDefaultAction(tmp);
    action->lauchFileSync(forceWithArg, skipDialog);
    action->deleteLater();
}

void FileLaunchManager::openAsync(const QString &uri, bool forceWithArg, bool skipDialog)
{
    QString tmp = uri;
    //FIXME: replace BLOCKING api in ui thread.
    auto targetUri = FileUtils::getTargetUri(uri);
    if (!targetUri.isNull()) {
        tmp = targetUri;
        qDebug()<<"open async"<<targetUri;
    }
    auto action = getDefaultAction(tmp);
    action->lauchFileAsync(forceWithArg, skipDialog);
    action->deleteLater();
}

void FileLaunchManager::openAsync(const QStringList &files, bool forceWithArg, bool skipDialog)
{
    QStringList targets;
    for (auto uri : files) {
        auto target = FileUtils::getTargetUri(uri);
        if (!target.isEmpty()) {
            targets.append(target);
        } else {
            targets.append(uri);
        }
    }
    auto action = getDefaultAction(targets.first());
    action->lauchFilesAsync(targets, forceWithArg, skipDialog);
    action->deleteLater();
}

bool FileLaunchManager::isGlibForceUsePortal()
{
    if (!init_glib_env) {
        auto env = qgetenv("GLIB_FORCE_USE_PORTAL");
        glib_force_use_portal = !env.isEmpty() && env[0] == '1';
        init_glib_env = true;
    }
    return glib_force_use_portal;
}

void FileLaunchManager::setDefaultLauchAction(const QString &uri, FileLaunchAction *action)
{
    //FIXME: replace BLOCKING api in ui thread.
    auto info = FileInfo::fromUri(uri);
    if (info->mimeType().isEmpty()) {
        FileInfoJob job(info);
        job.querySync();
    }

    GError *err = nullptr;
    bool ret = g_app_info_set_as_default_for_type(action->gAppInfo(),
                                       info->mimeType().toUtf8(),
                                       &err);
    if (false == ret) {
        qDebug()<<"set default app failed, err code" <<err->code <<"err msg"<< err->message;
        g_error_free(err);
    }
}
