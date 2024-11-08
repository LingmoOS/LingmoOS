/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include "app-launcher.h"
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDebug>
#include <QDBusPendingReply>
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <mutex>
#include <qt5xdg/XdgDesktopFile>

static LingmoUIQuick::AppLauncher *g_appLauncher = nullptr;
static std::once_flag onceFlag;
static const QString LINGMO_APP_MANAGER_NAME("com.lingmo.ProcessManager");
static const QString LINGMO_APP_MANAGER_PATH("/com/lingmo/ProcessManager/AppLauncher");
static const QString LINGMO_APP_MANAGER_INTERFACE("com.lingmo.ProcessManager.AppLauncher");

namespace LingmoUIQuick {

class AppLauncherPrivate
{
};

AppLauncher *AppLauncher::instance()
{
    std::call_once(onceFlag, [ & ] {
        g_appLauncher = new AppLauncher();
    });
    return g_appLauncher;
}

AppLauncher::AppLauncher(QObject *parent) : QObject(parent), d(new AppLauncherPrivate)
{
}

AppLauncher::~AppLauncher()
{
    if(d) {
        delete d;
        d = nullptr;
    }
    g_appLauncher = nullptr;
}

void AppLauncher::launchApp(const QString &desktopFile)
{
    if (desktopFile.isEmpty()) {
        return;
    }
    QDBusMessage message = QDBusMessage::createMethodCall(LINGMO_APP_MANAGER_NAME, LINGMO_APP_MANAGER_PATH, LINGMO_APP_MANAGER_INTERFACE, "LaunchApp");
    message << desktopFile;
    auto watcher = new QDBusPendingCallWatcher(QDBusPendingCall(QDBusConnection::sessionBus().asyncCall(message)), this);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this, [desktopFile] (QDBusPendingCallWatcher *self) {
        if (self->isError()) {
            qWarning() << "Fail to call " << LINGMO_APP_MANAGER_INTERFACE << self->error();
            XdgDesktopFile xdf;
            xdf.load(desktopFile);
            if(!xdf.startDetached()) {
                qWarning() << "Fail to Launch" << desktopFile;
            }
        }
        self->deleteLater();
    });
}

void AppLauncher::launchAppWithArguments(const QString &desktopFile, const QStringList &args)
{
    if (desktopFile.isEmpty()) {
        return;
    }
    QDBusMessage message = QDBusMessage::createMethodCall(LINGMO_APP_MANAGER_NAME, LINGMO_APP_MANAGER_PATH, LINGMO_APP_MANAGER_INTERFACE, "LaunchAppWithArguments");
    message << desktopFile << args;
    auto watcher = new QDBusPendingCallWatcher(QDBusPendingCall(QDBusConnection::sessionBus().asyncCall(message)), this);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this, [desktopFile, args] (QDBusPendingCallWatcher *self) {
        if (self->isError()) {
            qWarning() << "Fail to call " << LINGMO_APP_MANAGER_INTERFACE << self->error();
            XdgDesktopFile xdf;
            xdf.load(desktopFile);
            if(!xdf.startDetached(args)) {
                qWarning() << "Fail to Launch" << desktopFile << "with args " << args;
            }
        }
        self->deleteLater();
    });
}

void AppLauncher::openUri(const QString &uri, const QString &parentWindow)
{
    if (uri.isEmpty()) {
        return;
    }
    //这个接口调用了org.freedesktop.portal.OpenURI，当uri为本地文件时会调用后端org.freedesktop.impl.portal.AppChooser，
    // 会显示explorer的应用选择窗
    if(!QDesktopServices::openUrl(QUrl(uri))) {
        qWarning() << "Fail to open" << uri;
    }
}

void AppLauncher::runCommand(const QString &cmd)
{
    if (cmd.isEmpty()) {
        return;
    }

    QDBusMessage message = QDBusMessage::createMethodCall(LINGMO_APP_MANAGER_NAME, LINGMO_APP_MANAGER_PATH, LINGMO_APP_MANAGER_INTERFACE, "RunCommand");
    message << cmd;
    auto watcher = new QDBusPendingCallWatcher(QDBusPendingCall(QDBusConnection::sessionBus().asyncCall(message)), this);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this, [cmd] (QDBusPendingCallWatcher *self) {
        if (self->isError()) {
            qWarning() << "Fail to call " << LINGMO_APP_MANAGER_INTERFACE << self->error();
            QProcess::startDetached(cmd, {});
        }
        self->deleteLater();
    });
}
} // LingmoUIQuick
