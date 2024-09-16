// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QDBusConnection>
#include <QSharedPointer>
#include <sys/syscall.h>
#include <QProcess>
#include <thread>
#include <QDBusUnixFileDescriptor>

#include "dbus/applicationmanager1service.h"
#include "dbus/applicationservice.h"
#include "cgroupsidentifier.h"
#include "constant.h"
#include "dbus/instanceadaptor.h"
#include "global.h"

class TestApplicationManager : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        if (!QDBusConnection::sessionBus().isConnected()) {
            GTEST_SKIP() << "skip for now.";
        }
        auto &bus = ApplicationManager1DBus::instance();
        bus.initGlobalServerBus(DBusType::Session);
        bus.setDestBus();
        std::shared_ptr<ApplicationManager1Storage> tmp{nullptr};
        m_am = new ApplicationManager1Service{std::make_unique<CGroupsIdentifier>(), tmp};
        auto ptr = std::make_unique<QFile>(QString{"/usr/share/applications/test-Application.desktop"});
        const auto *appID = "test-Application";
        DesktopFile file{std::move(ptr), appID, 0, 0};
        QSharedPointer<ApplicationService> app = QSharedPointer<ApplicationService>::create(std::move(file), nullptr, tmp);
        QSharedPointer<InstanceService> instance = QSharedPointer<InstanceService>::create(
            InstancePath.path().split('/').last(), ApplicationPath.path(), QString{"/"}, QString{"DDE"});
        app->m_Instances.insert(InstancePath, instance);
        m_am->m_applicationList.insert(appID, app);
        new InstanceAdaptor{instance.data()};
    }

    static void TearDownTestCase() { m_am->deleteLater(); }

    static inline ApplicationManager1Service *m_am{nullptr};
    const static inline QDBusObjectPath ApplicationPath{QString{DDEApplicationManager1ObjectPath} + "/test_2dApplication"};
    const static inline QDBusObjectPath InstancePath{ApplicationPath.path() + "/" + QUuid::createUuid().toString(QUuid::Id128)};
};

TEST_F(TestApplicationManager, identifyService)
{
    if (m_am == nullptr) {
        GTEST_SKIP() << "skip for now...";
    }
    using namespace std::chrono_literals;
    // for service unit
    auto workingDir = QDir::cleanPath(QDir::current().absolutePath() + QDir::separator() + ".." + QDir::separator() + "tools");
    QFile pidFile{workingDir + QDir::separator() + "pid.txt"};
    if (pidFile.exists()) {
        ASSERT_TRUE(pidFile.remove());
    }

    QProcess fakeServiceProc;
    fakeServiceProc.setWorkingDirectory(workingDir);
    auto InstanceId = InstancePath.path().split('/').last();
    fakeServiceProc.start("/usr/bin/systemd-run",
                          {{QString{R"(--unit=app-DDE-test\x2dApplication@%1.service)"}.arg(InstanceId)},
                           {"--user"},
                           {QString{"--working-directory=%1"}.arg(workingDir)},
                           {"--slice=app.slice"},
                           {"./fake-process.sh"}});
    fakeServiceProc.waitForFinished();
    if (fakeServiceProc.exitStatus() != QProcess::ExitStatus::NormalExit) {
        GTEST_SKIP() << "invoke systemd-run failed.";
    }

    std::this_thread::sleep_for(500ms);

    auto success = pidFile.open(QFile::ReadOnly | QFile::Text | QFile::ExistingOnly);
    EXPECT_TRUE(success);

    if (!success) {
        qWarning() << pidFile.errorString();
        fakeServiceProc.terminate();
        ASSERT_TRUE(false);
    }

    bool ok{false};
    auto fakePid = pidFile.readLine().toInt(&ok);
    ASSERT_TRUE(ok);

    auto pidfd = pidfd_open(fakePid, 0);
    ASSERT_TRUE(pidfd > 0) << std::strerror(errno);
    ObjectInterfaceMap instanceInfo;
    QDBusObjectPath path;
    auto appId = m_am->Identify(QDBusUnixFileDescriptor{pidfd}, path, instanceInfo);
    EXPECT_EQ(appId.toStdString(), QString{"test-Application"}.toStdString());

    if (path.path().isEmpty()) {
        GTEST_SKIP_("couldn't find instance and skip.");
    }

    ObjectInterfaceMap map{{QString{InstanceInterface},
                            QVariantMap{{QString{"Application"}, ApplicationPath},
                                        {QString{"SystemdUnitPath"}, QDBusObjectPath{"/"}},
                                        {QString{"Launcher"}, QString{"DDE"}},
                                        {QString{"Orphaned"}, false}}}};
    EXPECT_EQ(instanceInfo, map);

    close(pidfd);

    if (pidFile.exists()) {
        pidFile.remove();
    }

    // for scope unit
    ASSERT_TRUE(QProcess::startDetached("/usr/bin/systemd-run",
                                        {{"--scope"},
                                         {QString{R"(--unit=app-DDE-test\x2dApplication-%1.scope)"}.arg(InstanceId)},
                                         {"--user"},
                                         {QString{"--working-directory=%1"}.arg(workingDir)},
                                         {"--slice=app.slice"},
                                         {"./fake-process.sh"},
                                         {"Scope"}},
                                        workingDir));

    std::this_thread::sleep_for(500ms);

    success = pidFile.open(QFile::ReadOnly | QFile::Text | QFile::ExistingOnly);
    EXPECT_TRUE(success);

    if (!success) {
        qWarning() << pidFile.errorString();
        ASSERT_TRUE(false);
    }

    ok = false;
    fakePid = pidFile.readLine().toInt(&ok);
    ASSERT_TRUE(ok);

    pidfd = pidfd_open(fakePid, 0);
    ASSERT_TRUE(pidfd > 0) << std::strerror(errno);

    instanceInfo.clear();
    path.setPath("/");

    appId = m_am->Identify(QDBusUnixFileDescriptor{pidfd}, path, instanceInfo);
    EXPECT_EQ(appId.toStdString(), QString{"test-Application"}.toStdString());

    if (path.path().isEmpty()) {
        GTEST_SKIP_("couldn't find instance and skip.");
    }

    EXPECT_EQ(instanceInfo, map);

    close(pidfd);

    if (pidFile.exists()) {
        pidFile.remove();
    }
}
