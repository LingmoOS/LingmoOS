/*
 * Peony-Qt
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "explor-desktop-application.h"
#include "waylandoutputmanager.h"

#include <QDBusMessage>
#include <QDBusConnection>
#include <lingmo-log4qt.h>
#include <stdio.h>
#include <stdlib.h>
#include <QTime>
#include <QFile>
#include <QThread>

#include <QStandardPaths>
#include <QProcess>

#include "xdg-portal-helper.h"
#ifdef LINGMO_SDK_KABASE
#include <lingmosdk/applications/kabase/log.hpp>
#endif

void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    QByteArray currentTime = QTime::currentTime().toString().toLocal8Bit();

    QString logFilePath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/explor-qt-desktop.log";

    bool showDebug = true;
    if (!QFile::exists(logFilePath)) {
        showDebug = false;
    }
    FILE *log_file = nullptr;

    if (showDebug) {
        log_file = fopen(logFilePath.toLocal8Bit().constData(), "a+");
    }

    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";
    switch (type) {
    case QtDebugMsg:
        if (!log_file) {
            break;
        }
        fprintf(log_file, "Debug: %s: %s (%s:%u, %s)\n", currentTime.constData(), localMsg.constData(), file, context.line, function);
        break;
    case QtInfoMsg:
        fprintf(log_file? log_file: stdout, "Info: %s: %s (%s:%u, %s)\n", currentTime.constData(), localMsg.constData(), file, context.line, function);
        break;
    case QtWarningMsg:
        fprintf(log_file? log_file: stderr, "Warning: %s: %s (%s:%u, %s)\n", currentTime.constData(), localMsg.constData(), file, context.line, function);
        break;
    case QtCriticalMsg:
        fprintf(log_file? log_file: stderr, "Critical: %s: %s (%s:%u, %s)\n", currentTime.constData(), localMsg.constData(), file, context.line, function);
        break;
    case QtFatalMsg:
        fprintf(log_file? log_file: stderr, "Fatal: %s: %s (%s:%u, %s)\n", currentTime.constData(), localMsg.constData(), file, context.line, function);
        break;
    }

    if (log_file)
        fclose(log_file);
}

int main(int argc, char *argv[])
{
    //qputenv("QT_QPA_PLATFORM", "wayland");
    PeonyDesktopApplication::explor_desktop_start_time = QDateTime::currentMSecsSinceEpoch();

    QString xdgUserDirsUri = "file://" + QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.config/user-dirs.dirs";
    if (!QFile::exists(xdgUserDirsUri)) {
        if (QFile::exists("/usr/bin/xdg-user-dirs-update")) {
            QProcess p;
            p.setProgram("/usr/bin/xdg-user-dirs-update");
            p.start();
            p.waitForFinished();
        }
    }

    Peony::XdgPortalHelper::getInstance()->tryUnusePortal();
//    initLingmoUILog4qt("explor-desktop");
//    qInstallMessageHandler(messageOutput);
#ifdef LINGMO_SDK_KABASE
    qInstallMessageHandler(kdk::kabase::Log::logOutput);
#else
    initLingmoUILog4qt("explor");
#endif
    qDebug() << "desktop start time in main:" <<PeonyDesktopApplication::explor_desktop_start_time;

    QGuiApplication::setFallbackSessionManagementEnabled(true);
    QGuiApplication::setQuitOnLastWindowClosed(false);
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    QString id = "explor-qt-desktop" + qgetenv("DISPLAY");
    PeonyDesktopApplication a(argc, argv, id);
    Peony::XdgPortalHelper::getInstance()->tryResetPortal();
    if (a.isRunning())
        return 0;

//    WaylandOutputManager waylandOutputManager;
//    QThread waylandThread;
//    waylandOutputManager.moveToThread(&waylandThread);
//    QObject::connect(&waylandThread, &QThread::started, &waylandOutputManager, &WaylandOutputManager::run);
//    QObject::connect(&a, &PeonyDesktopApplication::requestSetLINGMOOutputEnable, &waylandOutputManager, &WaylandOutputManager::setLINGMOOutputEnable);
//    waylandThread.start();

//    QDBusMessage message = QDBusMessage::createMethodCall("org.gnome.SessionManager",
//                                                          "/org/gnome/SessionManager",
//                                                          "org.gnome.SessionManager",
//                                                          "startupfinished");
//    QList<QVariant> args;
//    args.append("explor-qt-desktop");
//    args.append("startupfinished");
//    message.setArguments(args);
//    QDBusConnection::sessionBus().send(message);

    return a.exec();
}

