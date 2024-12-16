// Copyright (C) 2023 Dingyuan Zhang <lxz@mkacg.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <systemd/sd-daemon.h>

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusServiceWatcher>
#include <QDBusUnixFileDescriptor>
#include <QDebug>
#include <QFile>
#include <QtEnvironmentVariables>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("TreeLand socket helper");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption typeOption(QStringList() << "t"
                                                << "type",
                                  "xwayland",
                                  "wayland");
    parser.addOption(typeOption);

    parser.process(app);

    const QString &type = parser.value(typeOption);

    if (sd_listen_fds(0) > 1) {
        fprintf(stderr, "Too many file descriptors received.\n");
        exit(1);
    }

    QDBusUnixFileDescriptor unixFileDescriptor(SD_LISTEN_FDS_START);

    auto active = [unixFileDescriptor, type](const QDBusConnection &connection) {
        auto activateFd = [unixFileDescriptor, type, connection] {
            QDBusInterface updateFd("org.lingmo.Compositor1",
                                    "/org/lingmo/Compositor1",
                                    "org.lingmo.Compositor1",
                                    connection);

            if (updateFd.isValid()) {
                if (type == "wayland") {
                    updateFd.call("ActivateWayland", QVariant::fromValue(unixFileDescriptor));
                } else if (type == "xwayland") {
                    QDBusReply<QString> reply = updateFd.call("XWaylandName");
                    if (reply.isValid()) {
                        const QString &xwaylandName = reply.value();

                        QDBusInterface systemd("org.freedesktop.systemd1",
                                               "/org/freedesktop/systemd1",
                                               "org.freedesktop.systemd1.Manager",
                                               QDBusConnection::sessionBus());
                        systemd.call("SetEnvironment",
                                     QStringList() << QString("DISPLAY=%1").arg(xwaylandName));
                    }
                }
            }
        };

        QDBusServiceWatcher *compositorWatcher =
            new QDBusServiceWatcher("org.lingmo.Compositor1",
                                    connection,
                                    QDBusServiceWatcher::WatchForRegistration);

        QObject::connect(compositorWatcher,
                         &QDBusServiceWatcher::serviceRegistered,
                         compositorWatcher,
                         activateFd);

        activateFd();
    };

    active(QDBusConnection::sessionBus());
    active(QDBusConnection::systemBus());

    sd_notify(0, "READY=1");

    return app.exec();
}
