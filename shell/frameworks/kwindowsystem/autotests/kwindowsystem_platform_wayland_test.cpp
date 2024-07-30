/*
    SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <QFileSystemWatcher>
#include <QProcess>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

class TestKWindowsystemPlatformWayland : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testWithHelper();

private:
    std::unique_ptr<QProcess> m_westonProcess;
};

void TestKWindowsystemPlatformWayland::initTestCase()
{
    const QString westonExec = QStandardPaths::findExecutable(QStringLiteral("weston"));

    // start Weston
    m_westonProcess.reset(new QProcess);
    m_westonProcess->setProgram(westonExec);
    m_westonProcess->setArguments(QStringList({QStringLiteral("--socket=kwindowsystem-platform-wayland-0"), QStringLiteral("--backend=headless-backend.so")}));
    m_westonProcess->start();
    if (!m_westonProcess->waitForStarted()) {
        m_westonProcess.reset();
        QSKIP("Weston could not be started");
    }

    // wait for the socket to appear
    QTest::qWait(500);

    QDir runtimeDir(qgetenv("XDG_RUNTIME_DIR"));
    if (runtimeDir.exists(QStringLiteral("kwindowsystem-platform-wayland-0"))) {
        // already there
        return;
    }

    std::unique_ptr<QFileSystemWatcher> socketWatcher(new QFileSystemWatcher(QStringList({runtimeDir.absolutePath()})));
    QSignalSpy socketSpy(socketWatcher.get(), &QFileSystemWatcher::directoryChanged);
    QVERIFY(socketSpy.isValid());

    // limit to max of 10 waits
    for (int i = 0; i < 10; i++) {
        QVERIFY(socketSpy.wait());
        if (runtimeDir.exists(QStringLiteral("kwindowsystem-platform-wayland-0"))) {
            return;
        }
    }
}

void TestKWindowsystemPlatformWayland::cleanupTestCase()
{
    if (!m_westonProcess) {
        return;
    }
    m_westonProcess->terminate();
    QVERIFY(m_westonProcess->waitForFinished());
    m_westonProcess.reset();
}

void TestKWindowsystemPlatformWayland::testWithHelper()
{
    // This test starts a helper binary on platform wayland
    // it executes the actual test and will return 0 on success, and an error value otherwise
    QString processName = QFINDTESTDATA("kwindowsystem_platform_wayland_helper");
    QVERIFY(!processName.isEmpty());

    QProcess helper;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QStringLiteral("WAYLAND_DISPLAY"), QStringLiteral("kwindowsystem-platform-wayland-0"));
    helper.setProgram(processName);
    helper.setProcessEnvironment(env);
    helper.start();
    QVERIFY(helper.waitForFinished());
    QCOMPARE(helper.exitCode(), 0);
}

QTEST_GUILESS_MAIN(TestKWindowsystemPlatformWayland)
#include "kwindowsystem_platform_wayland_test.moc"
