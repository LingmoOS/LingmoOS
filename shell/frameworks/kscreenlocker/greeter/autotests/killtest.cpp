/*
SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/
// own
#include <config-kscreenlocker.h>
// KF
#include <KLibexec>
// Qt
#include <QProcess>
#include <QSignalSpy>
#include <QTest>
// system
#include <signal.h>

Q_DECLARE_METATYPE(QProcess::ExitStatus)

class KillTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testKill_data();
    void testKill();
    void testImmediateKill_data();
    void testImmediateKill();
};

void KillTest::testKill_data()
{
    QTest::addColumn<int>("signal");
    QTest::addColumn<bool>("expectedQuit");
    QTest::addColumn<QProcess::ExitStatus>("exitStatus");

    QTest::newRow("SIGHUP") << SIGHUP << true << QProcess::CrashExit;
    QTest::newRow("SIGINT") << SIGINT << true << QProcess::CrashExit;
    QTest::newRow("SIGQUIT") << SIGQUIT << true << QProcess::CrashExit;
    QTest::newRow("SIGILL") << SIGILL << true << QProcess::CrashExit;
    QTest::newRow("SIGTRAP") << SIGTRAP << true << QProcess::CrashExit;
    QTest::newRow("SIGABRT") << SIGABRT << true << QProcess::CrashExit;
    QTest::newRow("SIGIOT") << SIGIOT << true << QProcess::CrashExit;
    QTest::newRow("SIGBUS") << SIGBUS << true << QProcess::CrashExit;
    QTest::newRow("SIGFPE") << SIGFPE << true << QProcess::CrashExit;
    QTest::newRow("SIGKILL") << SIGKILL << true << QProcess::CrashExit;
    QTest::newRow("SIGUSR1") << SIGUSR1 << false << QProcess::CrashExit;
    QTest::newRow("SIGSEGV") << SIGSEGV << true << QProcess::CrashExit;
    QTest::newRow("SIGUSR2") << SIGUSR2 << true << QProcess::CrashExit;
    QTest::newRow("SIGPIPE") << SIGPIPE << true << QProcess::CrashExit;
    QTest::newRow("SIGALRM") << SIGALRM << true << QProcess::CrashExit;
    QTest::newRow("SIGTERM") << SIGTERM << true << QProcess::NormalExit;
    // ignore
    // QTest::newRow("SIGCHLD")   << SIGCHLD;
    // QTest::newRow("SIGCONT")   << SIGCONT;
    // QTest::newRow("SIGSTOP")   << SIGSTOP;
    // QTest::newRow("SIGTSTP")   << SIGTSTP;
    // QTest::newRow("SIGTTIN")   << SIGTTIN;
    // QTest::newRow("SIGTTOU")   << SIGTTOU;
    // QTest::newRow("SIGURG")    << SIGURG;
    // QTest::newRow("SIGXCPU")   << SIGXCPU;
    // QTest::newRow("SIGXFSZ")   << SIGXFSZ;
    // QTest::newRow("SIGVTALRM") << SIGVTALRM;
    // QTest::newRow("SIGPROF")   << SIGPROF;
    // QTest::newRow("SIGWINCH")  << SIGWINCH;
    // QTest::newRow("SIGIO")     << SIGIO;
    // QTest::newRow("SIGPWR")    << SIGPWR;
    QTest::newRow("SIGSYS") << SIGSYS << true << QProcess::CrashExit;
#ifdef Q_OS_LINUX
#ifdef SIGUNUSED
    QTest::newRow("SIGUNUSED") << SIGUNUSED << true << QProcess::CrashExit;
#endif
#ifdef SIGSTKFLT
    QTest::newRow("SIGSTKFLT") << SIGSTKFLT << true << QProcess::CrashExit;
#endif
#endif
}

void KillTest::testKill()
{
    QProcess greeter(this);
    QSignalSpy spy(&greeter, &QProcess::readyReadStandardOutput);

    greeter.setReadChannel(QProcess::StandardOutput);
    greeter.start(KLibexec::path(KSCREENLOCKER_GREET_BIN_REL), QStringList({QStringLiteral("--testing")}));

    // Wait for the "locked at" message as an indication that the greeter is ready
    spy.wait(5000);
    QCOMPARE(spy.count(), 1);

    QVERIFY(greeter.readAllStandardOutput().contains(QByteArrayLiteral("Locked at")));

    // now kill
    QFETCH(int, signal);
    kill(greeter.processId(), signal);

    QFETCH(bool, expectedQuit);
    QCOMPARE(greeter.waitForFinished(3000), expectedQuit);
    if (greeter.state() == QProcess::Running) {
        greeter.terminate();
        QVERIFY(greeter.waitForFinished());
    } else {
        QFETCH(QProcess::ExitStatus, exitStatus);
        QCOMPARE(greeter.exitStatus(), exitStatus);

        if (greeter.exitStatus() == QProcess::NormalExit) {
            // exit code is only valid for NormalExit
            QCOMPARE(greeter.exitCode(), 1);
        }
    }
}

void KillTest::testImmediateKill_data()
{
    QTest::addColumn<int>("signal");

    QTest::newRow("SIGHUP") << SIGHUP;
    QTest::newRow("SIGINT") << SIGINT;
    QTest::newRow("SIGQUIT") << SIGQUIT;
    QTest::newRow("SIGILL") << SIGILL;
    QTest::newRow("SIGTRAP") << SIGTRAP;
    QTest::newRow("SIGABRT") << SIGABRT;
    QTest::newRow("SIGIOT") << SIGIOT;
    QTest::newRow("SIGBUS") << SIGBUS;
    QTest::newRow("SIGFPE") << SIGFPE;
    QTest::newRow("SIGKILL") << SIGKILL;
    QTest::newRow("SIGUSR1") << SIGUSR1;
    QTest::newRow("SIGSEGV") << SIGSEGV;
    QTest::newRow("SIGUSR2") << SIGUSR2;
    QTest::newRow("SIGPIPE") << SIGPIPE;
    QTest::newRow("SIGALRM") << SIGALRM;
    QTest::newRow("SIGTERM") << SIGTERM;
    // ignore
    // QTest::newRow("SIGCHLD")   << SIGCHLD;
    // QTest::newRow("SIGCONT")   << SIGCONT;
    // QTest::newRow("SIGSTOP")   << SIGSTOP;
    // QTest::newRow("SIGTSTP")   << SIGTSTP;
    // QTest::newRow("SIGTTIN")   << SIGTTIN;
    // QTest::newRow("SIGTTOU")   << SIGTTOU;
    // QTest::newRow("SIGURG")    << SIGURG;
    // QTest::newRow("SIGXCPU")   << SIGXCPU;
    // QTest::newRow("SIGXFSZ")   << SIGXFSZ;
    // QTest::newRow("SIGVTALRM") << SIGVTALRM;
    // QTest::newRow("SIGPROF")   << SIGPROF;
    // QTest::newRow("SIGWINCH")  << SIGWINCH;
    // QTest::newRow("SIGIO")     << SIGIO;
    // QTest::newRow("SIGPWR")    << SIGPWR;
    QTest::newRow("SIGSYS") << SIGSYS;
#ifdef Q_OS_LINUX
#ifdef SIGSTKFLT
    QTest::newRow("SIGSTKFLT") << SIGSTKFLT;
#endif
#ifdef SIGUNUSED
    QTest::newRow("SIGUNUSED") << SIGUNUSED;
#endif
#endif
}

void KillTest::testImmediateKill()
{
    // this test ensures that the greeter indicates crashexit when a signal is sent to the greeter
    // before it had time to properly setup
    QProcess greeter(this);
    greeter.start(KLibexec::path(KSCREENLOCKER_GREET_BIN_REL), QStringList({QStringLiteral("--testing")}));
    QVERIFY(greeter.waitForStarted());

    // now kill
    QFETCH(int, signal);
    kill(greeter.processId(), signal);

    QVERIFY(greeter.waitForFinished());
    QCOMPARE(greeter.exitStatus(), QProcess::CrashExit);
}

QTEST_MAIN(KillTest)
#include "killtest.moc"
