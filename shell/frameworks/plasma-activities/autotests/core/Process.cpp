/*
    SPDX-FileCopyrightText: 2013 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "Process.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDebug>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QString>
#include <QTemporaryDir>
#include <QTest>

#include <signal.h>
#include <sys/types.h>

#include "common/dbus/common.h"

namespace Process
{
QProcess *Modifier::s_process = nullptr;
QTemporaryDir *Modifier::s_tempDir = nullptr;
QString nulluuid = QStringLiteral("00000000-0000-0000-0000-000000000000");

Modifier::Modifier(Action action)
    : Test()
    , m_action(action)
{
    if (!s_process) {
        s_process = new QProcess();
        s_tempDir = new QTemporaryDir();

        if (!s_tempDir->isValid()) {
            qFatal("Can not create a temporary dir");
        }

        qDebug() << "Running KAMD in " << s_tempDir->path();
        s_process->setProcessChannelMode(QProcess::ForwardedChannels);
    }
}

void Modifier::initTestCase()
{
    const auto state = s_process->state();

    if (state != QProcess::NotRunning && m_action == Start) {
        qFatal("Already running");
    }

    switch (m_action) {
    case Start: {
        qDebug() << "Starting...";

        QRegularExpression nonxdg(QStringLiteral("^[^X][^D][^G].*$"));

        auto env = QProcessEnvironment::systemEnvironment().toStringList().filter(nonxdg)
            << QStringLiteral("XDG_DATA_HOME=") + s_tempDir->path() + QStringLiteral("/")
            << QStringLiteral("XDG_CONFIG_HOME=") + s_tempDir->path() + QStringLiteral("/")
            << QStringLiteral("XDG_CACHE_HOME=") + s_tempDir->path() + QStringLiteral("/");

        // qDebug() << env;

        s_process->setEnvironment(env);
        const QString exec = QStandardPaths::findExecutable(QStringLiteral("kactivitymanagerd"));
        QVERIFY(!exec.isEmpty());
        s_process->start(exec, QStringList());
        s_process->waitForStarted();

        break;
    }

    case Stop:
    case Kill:
    case Crash: {
        qDebug() << "Stopping...";

        const auto dbus = QDBusConnection::sessionBus().interface();
        const auto kamd = KAMD_DBUS_SERVICE;

        if (!dbus->isServiceRegistered(kamd)) {
            break;
        }

        uint pid = dbus->servicePid(kamd);
        // clang-format off
            ::kill(pid,
                    m_action == Stop ? SIGQUIT :
                    m_action == Kill ? SIGKILL :
                    /* else */         SIGSEGV
                    );
        // clang-format on

        while (Test::isActivityManagerRunning()) {
            QCoreApplication::processEvents();
        }

        if (s_process->state() == QProcess::Running) {
            s_process->terminate();
            s_process->waitForFinished();
        }

        break;
    }
    }
}

void Modifier::testProcess()
{
    const auto state = s_process->state();

    switch (m_action) {
    case Start:
        QCOMPARE(state, QProcess::Running);
        break;

    case Stop:
        QCOMPARE(state, QProcess::NotRunning);
        break;

    case Kill:
        QCOMPARE(state, QProcess::NotRunning);
        break;

    case Crash:
        break;
    }
}

void Modifier::cleanupTestCase()
{
    Q_EMIT testFinished();
}

Modifier *exec(Action action)
{
    return new Modifier(action);
}

} // namespace Process

#include "moc_Process.cpp"
