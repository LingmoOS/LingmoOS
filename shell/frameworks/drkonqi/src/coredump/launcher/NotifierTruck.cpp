/*
    SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
    SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>
*/

#include "NotifierTruck.h"

#include <chrono>

#include <QCoreApplication>
#include <QFile>
#include <QProcess>
#include <QTimer>

#include <KNotification>
#include <KTerminalLauncherJob>

#include "../coredump.h"

using namespace std::chrono_literals;

bool NotifyTruck::handle(const Coredump &dump)
{
    if (!dump.m_rawData.value(dump.keyPickup()).isEmpty()) {
        // Pickups are currently not supported for notify handling. The problem is that we don't know if we already
        // notified on a crash or not because we have no persistent storage. To fix this we'd probably need to
        // rejigger things substantially and insert a pickup key into the journal that the processor can then look for.
        // Except then the processor needs to hold on to dumps until the entire journal is processed. All a bit awkward.
        return false;
    }

    auto notification = new KNotification(QStringLiteral("applicationcrash"));

    // immediate exit signal. This gets disconnected should `activated` arrive first (in that case we
    // want to wait for the terminal app to start and not exit on further notification signals)
    QObject::connect(notification, &KNotification::closed, this, [this, notification] {
        notification->disconnect(this);
        qApp->exit(0);
    });

    if (!QFile::exists(dump.filename)) {
        notification->setTitle(QStringLiteral("The warpcore has gone missing"));
        notification->setText(QStringLiteral("%1 [%2] crashed but has no core file").arg(dump.exe, QString::number(dump.pid)));
    } else {
        notification->setTitle(QStringLiteral("He's dead, Jim"));
        notification->setText(QStringLiteral("%1 [%2]").arg(dump.exe, QString::number(dump.pid)));
        auto gdbAction = notification->addAction(QStringLiteral("gdb"));

        const auto pid = dump.pid;

        connect(gdbAction, &KNotificationAction::activated, notification, [pid, this, notification]() {
            notification->disconnect(this);
            auto job = new KTerminalLauncherJob(QStringLiteral("coredumpctl gdb %1").arg(QString::number(pid)), this);
            job->setProcessEnvironment(QProcessEnvironment::systemEnvironment());
            connect(job, &KJob::result, this, [job] {
                if (job->error() != KJob::NoError) {
                    qWarning() << job->errorText();
                }
                qApp->exit(0);
            });
            job->start();

            // Just in case the launcher job bugs out also add a timer.
            auto startTimeout = new QTimer(this);
            startTimeout->setInterval(16s);
            connect(startTimeout, &QTimer::timeout, this, [] {
                qApp->exit(0);
            });
            startTimeout->start();
        });
    }

    notification->setFlags(KNotification::DefaultEvent | KNotification::SkipGrouping);
    notification->sendEvent();

    // KNotification internally depends on an eventloop to communicate over dbus. We therefore start the
    // eventloop here. ::handle() is expected to be blocking so this has no adverse effects.
    // The eventloop is either exited when the notification gets closed or when the debugger has started.
    qApp->exec();
    return true;
}

#include "moc_NotifierTruck.cpp"
