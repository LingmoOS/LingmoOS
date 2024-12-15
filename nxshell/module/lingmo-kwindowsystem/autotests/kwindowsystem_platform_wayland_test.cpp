/*
 *   Copyright 2016 Martin Gräßlin <mgraesslin@kde.org>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <QtTest>
#include <QProcess>

class TestKWindowsystemPlatformWayland : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testWithHelper();

private:
    QScopedPointer<QProcess> m_westonProcess;
};

void TestKWindowsystemPlatformWayland::initTestCase()
{
    // start Weston
    m_westonProcess.reset(new QProcess);
    m_westonProcess->setProgram(QStringLiteral("weston"));
    m_westonProcess->setArguments(QStringList({QStringLiteral("--socket=kwindowsystem-platform-wayland-0"),
                                               QStringLiteral("--backend=headless-backend.so")}));
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

    QScopedPointer<QFileSystemWatcher> socketWatcher(new QFileSystemWatcher(QStringList({runtimeDir.absolutePath()})));
    QSignalSpy socketSpy(socketWatcher.data(), &QFileSystemWatcher::directoryChanged);
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
    if (m_westonProcess.isNull()) {
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

    QScopedPointer<QProcess> helper(new QProcess);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QStringLiteral("WAYLAND_DISPLAY"), QStringLiteral("kwindowsystem-platform-wayland-0"));
    helper->setProgram(processName);
    helper->setProcessEnvironment(env);
    helper->start();
    QVERIFY(helper->waitForFinished());
    QCOMPARE(helper->exitCode(), 0);
}

QTEST_GUILESS_MAIN(TestKWindowsystemPlatformWayland)
#include "kwindowsystem_platform_wayland_test.moc"
