/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2019, Tianjin LINGMO Information Technology Co., Ltd.
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

#include "file-launch-action.h"
#include <gio/gdesktopappinfo.h>

#include "file-info.h"
#include "file-info-job.h"
#include "file-utils.h"
#include "file-operation-utils.h"
#include "audio-play-manager.h"
#include "global-settings.h"

#include <QMessageBox>
#include <QPushButton>
#include <QDir>
#include <QUrl>
#include <QFile>
#include <QProcess>
#include <recent-vfs-manager.h>
#include <QApplication>

#ifdef LINGMO_COMMON
#include <lingmosdk/lingmo-com4cxx.h>
#endif

#include <QDebug>
#include <QtX11Extras/QX11Info>
#include <kstartupinfo.h>

using namespace Peony;

static void launchAsyncCallback(GAppInfo *source_object,
                                GAsyncResult *res,
                                gpointer user_data)
{
    g_app_info_launch_uris_finish (source_object, res, nullptr);
    KStartupInfoId* startInfoId = static_cast<KStartupInfoId*>(user_data);
    if (!startInfoId)
        return;
    delete startInfoId;
}

static void contextLaunchedCallback(GAppLaunchContext *context,
                                    GAppInfo          *info,
                                    GVariant          *platform_data,
                                    gpointer           user_data)
{
    KStartupInfoId* startInfoId = static_cast<KStartupInfoId*>(user_data);
    if (!startInfoId)
        return;
    gint pid = -1;
    g_variant_lookup (platform_data, "pid", "i", &pid);
    KStartupInfoData data;
    data.addPid(pid);
#ifdef KSTARTUPINFO_HAS_SET_ICON_GEOMETRY
    data.setIconGeometry(QRect(0, 0, 1, 1));  // ugly
#endif
    KStartupInfo::sendChange(*startInfoId, data);
    KStartupInfo::resetStartupEnv();
}

static GAppLaunchContext *getAppContext(FileLaunchAction *action = nullptr)
{
    auto context = g_app_launch_context_new();
    auto env_list = g_listenv();
    auto p = env_list;
    while (*p) {
        g_app_launch_context_setenv(context, *p, g_getenv(*p));
        p++;
    }
    g_strfreev(env_list);
    if (action) {
        auto app_info = action->gAppInfo();
        if (!G_IS_DESKTOP_APP_INFO (app_info))
            return context;
        auto desktop_app_info = G_DESKTOP_APP_INFO (app_info);
        auto desktop_filename = g_desktop_app_info_get_filename(desktop_app_info);
        g_app_launch_context_setenv(context, "APPLICATION_ID", desktop_filename);
        //修改 bug#205475 双屏扩展模式下，打开副屏上的.sh ，窗口显示在副屏上，注释 DESKTOP_STARTUP_ID 就可以显示在主屏
        //g_app_launch_context_setenv(context, "DESKTOP_STARTUP_ID", g_app_info_get_name(app_info));
        g_app_launch_context_setenv(context, "TIMESTAMP",  QString::number(QX11Info::getTimestamp()).toUtf8().constData());
    }
    return context;
}

FileLaunchAction::FileLaunchAction(const QString &uri, GAppInfo *app_info, bool forceWithArg, QObject *parent) : QAction(parent)
{
    if(uri.startsWith("recent:///"))
        m_uri = FileUtils::getTargetUri(uri);
    else
        m_uri = uri;

    m_app_info = static_cast<GAppInfo*>(g_object_ref(app_info));
    m_force_with_arg = forceWithArg;

    if (!isValid())
        return;

    GIcon *icon = g_app_info_get_icon(m_app_info);
    auto iconName = FileUtils::getIconStringFromGIcon(icon);
    if (iconName.startsWith("/")) {
        m_icon.addFile(iconName);
    } else {
        m_icon = QIcon::fromTheme(iconName);
    }
    setIcon(m_icon);
    m_info_name = g_app_info_get_name(m_app_info);
    setText(m_info_name);
    m_info_display_name = g_app_info_get_display_name(m_app_info);

    connect(this, &QAction::triggered, [=]() {
        this->lauchFileAsync(m_force_with_arg, true);
    });
}

FileLaunchAction::~FileLaunchAction()
{
    if (m_app_info)
        g_object_unref(m_app_info);
}

const QString FileLaunchAction::getUri()
{
    return m_uri;
}

bool FileLaunchAction::isDesktopFileAction()
{
    //FIXME: replace BLOCKING api in ui thread.
    auto info = FileInfo::fromUri(m_uri);
    if (info->isEmptyInfo()) {
        FileInfoJob j(info);
        j.querySync();
    }
    return info->isDesktopFile();
}

const QString FileLaunchAction::getAppInfoName()
{
    return m_info_name;
}

const QString FileLaunchAction::getAppInfoDisplayName()
{
    return m_info_display_name;
}

bool FileLaunchAction::isExcuteableFile(QString fileType)
{
    if (m_executable_type.contains(fileType))
        return true;

    return false;
}

void FileLaunchAction::lauchFileSync(bool forceWithArg, bool skipDialog)
{
    m_force_with_arg = forceWithArg;

    //FIXME: replace BLOCKING api in ui thread.
    auto fileInfo = FileInfo::fromUri(m_uri);
    if (fileInfo->isEmptyInfo()) {
        FileInfoJob j(fileInfo);
        j.querySync();
    }

    bool readable = fileInfo->canRead();
    if (!readable) {
        QMessageBox::critical(0, tr("No Permission"), tr("File is not readable. Please check if file has read permisson."));
        return;
    }

    bool executable = fileInfo->canExecute();
    bool isAppImage = fileInfo->type() == "application/vnd.appimage";
    bool isExecutable = isExcuteableFile(fileInfo->type());
    if (isAppImage) {
        if (executable) {
            QUrl url = m_uri;
            auto path = url.path();

            QProcess p;
            p.setProgram(path);
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
            p.startDetached();
#else
            p.startDetached(path);
#endif
            return;
        }
    }

    if (executable && !isDesktopFileAction() && !skipDialog && isExecutable) {
        QMessageBox msg;
        auto defaultAction = msg.addButton("By Default App", QMessageBox::ButtonRole::ActionRole);
        auto exec = msg.addButton(tr("Execute Directly"), QMessageBox::ButtonRole::ActionRole);
        auto execTerm = msg.addButton(tr("Execute in Terminal"), QMessageBox::ButtonRole::ActionRole);
        msg.addButton(QMessageBox::Cancel);

        msg.setText(tr("Detected launching an executable file %1, you want?").arg(fileInfo->displayName()));
        msg.exec();
        auto button = msg.clickedButton();
        if (button == exec) {
            execFile();
            return;
        } else if (button == execTerm) {
            execFileInterm();
            return;
        } else if (button == defaultAction) {
            //skip
        } else {
            return;
        }
    }

    //fix bug#213466, remote file open not same with local issue
    //fix issue https://gitee.com/lingmo/explor/issues/I9QXWX, default open new file issue
    if (fileInfo->type() == "application/octet-stream" || fileInfo->type() == "application/x-zerosize") {
        GAppInfo *text_info = g_app_info_get_default_for_type("text/plain", false);
        GList *l = nullptr;
        char *uri = g_strdup(m_uri.toUtf8().constData());
        l = g_list_prepend(l, uri);
#if GLIB_CHECK_VERSION(2, 60, 0)
        g_app_info_launch_uris_async(text_info, l,
                                     nullptr, nullptr,
                                     nullptr, nullptr);
#else
        g_app_info_launch_uris(text_info, l, nullptr, nullptr);
#endif
        g_list_free_full(l, g_free);
        g_object_unref(text_info);
        return;
    }

    if (!isValid()) {
        Peony::AudioPlayManager::getInstance()->playWarningAudio();
        QMessageBox::critical(nullptr, tr("Open Failed"), tr("Can not open %1, file not exist, is it deleted?").arg(m_uri));
        return;
    }

    if (isDesktopFileAction() && !forceWithArg) {
        g_app_info_launch(m_app_info, nullptr, nullptr, nullptr);
    } else {
        GList *l = nullptr;
        char *uri = g_strdup(m_uri.toUtf8().constData());
        l = g_list_prepend(l, uri);
        g_app_info_launch_uris(m_app_info,
                               l,
                               nullptr,
                               nullptr);
        g_list_free_full(l, g_free);
    }

    return;
}

void FileLaunchAction::lauchFileAsync(bool forceWithArg, bool skipDialog)
{
    //FIXME: replace BLOCKING api in ui thread.
    auto fileInfo = FileInfo::fromUri(m_uri);
    if (fileInfo->isEmptyInfo()) {
        FileInfoJob j(fileInfo);
        j.querySync();
    }

    bool executable = fileInfo->canExecute();
    bool isAppImage = fileInfo->type() == "application/vnd.appimage";
    bool isExecutable = isExcuteableFile(fileInfo->type());
    qDebug() <<"executable:" <<executable <<isAppImage <<fileInfo->type();

    QUrl url = m_uri;
    if (isAppImage) {
        if (executable) {
            QUrl url = m_uri;
            auto path = url.path();

            QProcess p;
            p.setProgram(path);
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
            p.startDetached();
#else
            p.startDetached(path);
#endif
            return;
        }
    }

    if (executable && !isDesktopFileAction() && !skipDialog && isExecutable) {
        QMessageBox msg;
        auto defaultAction = msg.addButton(tr("By Default App"), QMessageBox::ButtonRole::ActionRole);
        auto exec = msg.addButton(tr("Execute Directly"), QMessageBox::ButtonRole::ActionRole);
        auto execTerm = msg.addButton(tr("Execute in Terminal"), QMessageBox::ButtonRole::ActionRole);
        msg.addButton(QMessageBox::Cancel);

        msg.setWindowTitle(tr("Launch Options"));
        msg.setText(tr("Detected launching an executable file %1, you want?").arg(fileInfo->displayName()));
        msg.exec();
        auto button = msg.clickedButton();
        if (button == exec) {
            execFile();
            return;
        } else if (button == execTerm) {
            execFileInterm();
            return;
        } else if (button == defaultAction) {
            //skip
        } else {
            return;
        }
    }

    //fix bug#213466, remote file open not same with local issue
    //fix issue https://gitee.com/lingmo/explor/issues/I9QXWX, default open new file issue
    if (fileInfo->type() == "application/octet-stream" || fileInfo->type() == "application/x-zerosize") {
        GAppInfo *text_info = g_app_info_get_default_for_type("text/plain", false);
        GList *l = nullptr;
        char *uri = g_strdup(m_uri.toUtf8().constData());
        l = g_list_prepend(l, uri);
#if GLIB_CHECK_VERSION(2, 60, 0)
        g_app_info_launch_uris_async(text_info, l,
                                     nullptr, nullptr,
                                     nullptr, nullptr);
#else
        g_app_info_launch_uris(text_info, l, nullptr, nullptr);
#endif
        g_list_free_full(l, g_free);
        g_object_unref(text_info);
        return;
    }

    if (!isValid()) {
        Peony::AudioPlayManager::getInstance()->playWarningAudio();
        bool isReadable = fileInfo->canRead();
        if (!isReadable)
        {
            if (fileInfo->isSymbolLink())
            {
                auto result = QMessageBox::question(nullptr, tr("Open Link failed"),
                                      tr("File not exist, do you want to delete the link file?"),
                                                    QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                if (result == QMessageBox::Yes) {
                    qDebug() << "Delete unused symbollink.";
                    QStringList selections;
                    selections.push_back(m_uri);
                    FileOperationUtils::trash(selections, true);
                }
            }
            else
                QMessageBox::critical(nullptr, tr("Open Failed"),
                                  tr("Can not open %1, Please confirm you have the right authority.").arg(url.toDisplayString()));
        }
        else if (fileInfo->isDesktopFile() && GlobalSettings::getInstance()->getProjectName() != V10_SP1_EDU)
        {
            auto result = QMessageBox::question(nullptr, tr("Open App failed"),
                                  tr("The linked app is changed or uninstalled, so it can not work correctly. \n"
                                     "Do you want to delete the link file?"),
                                                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            if (result == QMessageBox::Yes) {
                qDebug() << "Delete unused desktop file";
                QStringList selections;
                selections.push_back(m_uri);
                FileOperationUtils::trash(selections, true);
            }
        } else {
            QUrl url = m_uri;
            if(!QFile(url.path()).exists())
            {
                QMessageBox::warning(nullptr,
                                     tr("Error"),
                                     tr("File original path not exist, are you deleted or moved it?"));
                return;
            }

            auto result = QMessageBox::question(nullptr, tr("Error"), tr("Can not get a default application for opening %1, do you want open it with text format?").arg(m_uri), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            if (result == QMessageBox::Yes) {
                GAppInfo *text_info = g_app_info_get_default_for_type("text/plain", false);
                GList *l = nullptr;
                char *uri = g_strdup(m_uri.toUtf8().constData());
                l = g_list_prepend(l, uri);
#if GLIB_CHECK_VERSION(2, 60, 0)
                g_app_info_launch_uris_async(text_info, l,
                                             nullptr, nullptr,
                                             nullptr, nullptr);
#else
                g_app_info_launch_uris(text_info, l, nullptr, nullptr);
#endif
                g_list_free_full(l, g_free);
                g_object_unref(text_info);
            }
        }
        return;
    }

    if (isDesktopFileAction() && !forceWithArg) {
#if GLIB_CHECK_VERSION(2, 60, 0)
        g_app_info_launch_uris_async(m_app_info, nullptr,
                                     nullptr, nullptr,
                                     nullptr, nullptr);
#else
        g_app_info_launch_uris(m_app_info, nullptr, nullptr, nullptr);
#endif
    } else {
        GList *l = nullptr;
        char *uri = g_strdup(m_uri.toUtf8().constData());
        l = g_list_prepend(l, uri);

#if GLIB_CHECK_VERSION(2, 60, 0)
        // send startup info to kwindowsystem
        quint32 timeStamp = QX11Info::isPlatformX11() ? QX11Info::appUserTime() : 0;
        KStartupInfoId* startInfoId = new KStartupInfoId();
        startInfoId->initId(KStartupInfo::createNewStartupIdForTimestamp(timeStamp));
        startInfoId->setupStartupEnv();
        KStartupInfoData data;
        data.setHostname();
        float scale = qApp->devicePixelRatio();
        QRect rect = fileInfo.get()->property("iconGeometry").toRect();
        rect.moveTo(rect.x()/* * scale*/, rect.y()/* * scale*/);
#ifdef KSTARTUPINFO_HAS_SET_ICON_GEOMETRY
        if (rect.isValid()) {
            data.setIconGeometry(rect);
            qDebug() << "KStartupInfoData iconGeometry:" << m_uri <<rect ;
        }
#endif
        data.setLaunchedBy(getpid());

        KStartupInfo::sendStartup(*startInfoId, data);

        auto context = getAppContext(this);
        g_signal_connect(context, "launched", G_CALLBACK(contextLaunchedCallback), startInfoId);
        g_app_info_launch_uris_async(m_app_info, l,
                                     context, nullptr,
                                     GAsyncReadyCallback(launchAsyncCallback), startInfoId);

        g_object_unref(context);
#else
        g_app_info_launch_uris(m_app_info, l, nullptr, nullptr);
#endif
        RecentVFSManager::getInstance()->insert(fileInfo.get()->uri(), fileInfo.get()->mimeType(), fileInfo.get()->displayName(), g_app_info_get_name(m_app_info));
        g_list_free_full(l, g_free);
    }
}

void FileLaunchAction::lauchFilesAsync(const QStringList files, bool forceWithArg, bool skipDialog)
{
    m_force_with_arg = forceWithArg;
    if(files.isEmpty())
        return;

    //FIXME: replace BLOCKING api in ui thread.
    auto fileInfo = FileInfo::fromUri(m_uri);
    if (fileInfo->isEmptyInfo()) {
        FileInfoJob j(fileInfo);
        j.querySync();
    }

    bool executable = fileInfo->canExecute();
    bool isAppImage = fileInfo->type() == "application/vnd.appimage";
    bool isExecutable = isExcuteableFile(fileInfo->type());
    if (isAppImage) {
        if (executable) {
            QProcess p;
            for (auto uri:files) {
                auto path = ((QUrl) uri).path();
                QProcess p;
                p.setProgram(path);
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
                p.startDetached();
#else
                p.startDetached(path);
#endif
            }
            return;
        }
    }

    if (executable && !isDesktopFileAction() && !skipDialog && isExecutable) {
        QMessageBox msg;
        auto defaultAction = msg.addButton(tr("By Default App"), QMessageBox::ButtonRole::ActionRole);
        auto exec = msg.addButton(tr("Execute Directly"), QMessageBox::ButtonRole::ActionRole);
        auto execTerm = msg.addButton(tr("Execute in Terminal"), QMessageBox::ButtonRole::ActionRole);
        msg.addButton(QMessageBox::Cancel);

        msg.setWindowTitle(tr("Launch Options"));
        msg.setText(tr("Detected launching an executable file %1, you want?").arg(fileInfo->displayName()));
        msg.exec();
        auto button = msg.clickedButton();
        if (button == exec) {
            execFile();
            return;
        } else if (button == execTerm) {
            execFileInterm();
            return;
        } else if (button == defaultAction) {
            //skip
        } else {
            return;
        }
    }

    if (!isValid()) {
        Peony::AudioPlayManager::getInstance()->playWarningAudio();
        bool isReadable = fileInfo->canRead();
        if (!isReadable)
        {
            if (fileInfo->isSymbolLink())
            {
                auto result = QMessageBox::question(nullptr, tr("Open Link failed"),
                                      tr("File not exist, do you want to delete the link file?"),
                                                    QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                if (result == QMessageBox::Yes) {
                    qDebug() << "Delete unused symbollink.";
                    QStringList selections;
                    selections.push_back(m_uri);
                    FileOperationUtils::trash(selections, true);
                }
            }
            else
                QMessageBox::critical(nullptr, tr("Open Failed"),
                                  tr("Can not open %1, Please confirm you have the right authority.").arg(m_uri),
                                      QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes );
        }
        else if (fileInfo->isDesktopFile() && GlobalSettings::getInstance()->getProjectName() != V10_SP1_EDU)
        {
            auto result = QMessageBox::question(nullptr, tr("Open App failed"),
                                  tr("The linked app is changed or uninstalled, so it can not work correctly. \n"
                                     "Do you want to delete the link file?"),
                                                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            if (result == QMessageBox::Yes) {
                qDebug() << "Delete unused desktop file";
                QStringList selections;
                selections.push_back(m_uri);
                FileOperationUtils::trash(selections, true);
            }
        }
        else {
            auto result = QMessageBox::question(nullptr, tr("Error"), tr("Can not get a default application for opening %1, do you want open it with text format?").arg(m_uri),
                                                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            if (result == QMessageBox::Yes) {
                GAppInfo *text_info = g_app_info_get_default_for_type("text/plain", false);
                GList *l = nullptr;
                for (auto uri : files) {
                    l = g_list_prepend(l, g_strdup(uri.toUtf8().constData()));
                }
#if GLIB_CHECK_VERSION(2, 60, 0)
                g_app_info_launch_uris_async(text_info, l,
                                             nullptr, nullptr,
                                             nullptr, nullptr);
#else
                g_app_info_launch_uris(text_info, l, nullptr, nullptr);
#endif
                g_list_free_full(l, g_free);
                g_object_unref(text_info);
            }
        }
        return;
    }

    if (isDesktopFileAction() && !forceWithArg) {
#if GLIB_CHECK_VERSION(2, 60, 0)
        g_app_info_launch_uris_async(m_app_info, nullptr,
                                     nullptr, nullptr,
                                     nullptr, nullptr);
#else
        g_app_info_launch_uris(m_app_info, nullptr, nullptr, nullptr);
#endif
    } else {
        GList *l = nullptr;
        for (auto uri : files) {
            l = g_list_prepend(l, g_strdup(uri.toUtf8().constData()));
        }
#if GLIB_CHECK_VERSION(2, 60, 0)
        g_app_info_launch_uris_async(m_app_info, l,
                                     nullptr, nullptr,
                                     nullptr, nullptr);
#else
        g_app_info_launch_uris(m_app_info, l, nullptr, nullptr);
#endif
        g_list_free_full(l, g_free);
    }

    return;
}

bool FileLaunchAction::isValid()
{
    return G_IS_APP_INFO(m_app_info);
}

void FileLaunchAction::execFile()
{
    QUrl url = m_uri;
    char *quote = g_shell_quote(url.path().toUtf8());
    QString newDir = m_uri.section('/',0,m_uri.count('/')-1);
    GAppInfo *app_info = g_app_info_create_from_commandline(quote, nullptr, G_APP_INFO_CREATE_NONE, nullptr);
    QDir::setCurrent(QUrl(newDir).path());
    g_app_info_launch(app_info, nullptr, nullptr, nullptr);
    QDir::setCurrent(QDir::homePath());
    g_object_unref(app_info);
    g_free(quote);
}

void FileLaunchAction::execFileInterm()
{
    QUrl url = m_uri;
    char *quote = g_shell_quote(url.path().toUtf8());
    QString newDir = m_uri.section('/',0,m_uri.count('/')-1);
    GAppInfo *app_info = g_app_info_create_from_commandline(quote, nullptr, G_APP_INFO_CREATE_NEEDS_TERMINAL, nullptr);
    QDir::setCurrent(QUrl(newDir).path());
    g_app_info_launch(app_info, nullptr, nullptr, nullptr);
    QDir::setCurrent(QDir::homePath());
    g_object_unref(app_info);
    g_free(quote);
}
