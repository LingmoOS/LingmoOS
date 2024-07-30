/*
 * SPDX-FileCopyrightText: 2014 Daniel Vratil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#include <QDBusConnection>
#include <QGuiApplication>

#include "backendloader.h"
#include "kscreen_backendLauncher_debug.h"
#include "log.h"

int main(int argc, char **argv)
{
    KScreen::Log::instance();
    QGuiApplication::setDesktopSettingsAware(false);
    QCoreApplication::setAttribute(Qt::AA_DisableSessionManager);
    QGuiApplication app(argc, argv);

    if (!QDBusConnection::sessionBus().registerService(QStringLiteral("org.kde.KScreen"))) {
        qCWarning(KSCREEN_BACKEND_LAUNCHER) << "Cannot register org.kde.KScreen service. Another launcher already running?";
        return -1;
    }

    BackendLoader *loader = new BackendLoader;
    if (!loader->init()) {
        return -2;
    }

    const int ret = app.exec();

    // Make sure the backend is destroyed and unloaded before we return (i.e.
    // as long as QApplication object and it's XCB connection still exist
    delete loader;

    return ret;
}
