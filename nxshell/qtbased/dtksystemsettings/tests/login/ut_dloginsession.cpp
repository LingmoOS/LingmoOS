// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "dloginsession.h"
#include "dloginsession_p.h"
#include "dloginutils.h"
#include "login1sessioninterface.h"
#include "login1sessionservice.h"
#include "sessionmanagerservice.h"
#include "startmanagerservice.h"

#include <qfile.h>
#include <qprocess.h>

#include <ostream>

DLOGIN_USE_NAMESPACE

class TestDLoginSession : public testing::Test
{
public:
    TestDLoginSession()
        : m_fakeService(new Login1SessionService)
        , m_sessionManagerService(new SessionManagerService)
        , m_startManagerService(new StartManagerService)
        , m_dLoginSession(new DLoginSession("/org/freedesktop/login1/session"))
    {
        m_dLoginSession->d_ptr->m_inter->m_interface->setTimeout(INT_MAX);
        qputenv("XDG_CURRENT_DESKTOP", "Deepin");
    }

    virtual ~TestDLoginSession()
    {
        delete m_dLoginSession;
        delete m_fakeService;
        delete m_sessionManagerService;
        delete m_startManagerService;
    }

protected:
    Login1SessionService *m_fakeService;
    SessionManagerService *m_sessionManagerService;
    StartManagerService *m_startManagerService;
    DLoginSession *m_dLoginSession;
    static const QString Service;
};

const QString TestDLoginSession::Service = QStringLiteral("org.freedesktop.fakelogin1");

TEST_F(TestDLoginSession, propertyActive)
{
    m_fakeService->m_active = false;
    ASSERT_FALSE(m_fakeService->active());
    EXPECT_FALSE(m_dLoginSession->active());
    m_fakeService->m_active = true;
    ASSERT_TRUE(m_fakeService->active());
    EXPECT_TRUE(m_dLoginSession->active());
}

TEST_F(TestDLoginSession, propertyIdleHint)
{
    m_fakeService->m_idleHint = false;
    ASSERT_FALSE(m_fakeService->idleHint());
    EXPECT_FALSE(m_dLoginSession->idleHint());
    m_fakeService->m_idleHint = true;
    ASSERT_TRUE(m_fakeService->idleHint());
    EXPECT_TRUE(m_dLoginSession->idleHint());
}

TEST_F(TestDLoginSession, propertyLocked)
{
    m_sessionManagerService->m_locked = false;
    ASSERT_FALSE(m_sessionManagerService->Locked());
    EXPECT_FALSE(m_dLoginSession->locked());
    m_sessionManagerService->m_locked = true;
    ASSERT_TRUE(m_sessionManagerService->Locked());
    EXPECT_TRUE(m_dLoginSession->locked());
}

TEST_F(TestDLoginSession, propertyRemote)
{
    m_fakeService->m_remote = false;
    ASSERT_FALSE(m_fakeService->remote());
    EXPECT_FALSE(m_dLoginSession->remote());
    m_fakeService->m_remote = true;
    ASSERT_TRUE(m_fakeService->remote());
    EXPECT_TRUE(m_dLoginSession->remote());
}

struct SessionClassParam
{
    QString strClass;
    SessionClass enumClass;
};

class TestSessionClass : public TestDLoginSession,
                         public testing::WithParamInterface<SessionClassParam>
{
public:
    TestSessionClass()
        : TestDLoginSession()
    {
    }

    ~TestSessionClass() override = default;
};

TEST_P(TestSessionClass, sessionClass)
{
    auto params = GetParam();
    m_fakeService->m_sessionClass = params.strClass;
    ASSERT_EQ(params.strClass, m_fakeService->sessionClass());
    EXPECT_EQ(params.enumClass, m_dLoginSession->sessionClass());
}

INSTANTIATE_TEST_SUITE_P(Default,
                        TestSessionClass,
                        testing::Values(SessionClassParam{ "greeter", SessionClass::Greeter },
                                        SessionClassParam{ "lock-screen",
                                                           SessionClass::LockScreen },
                                        SessionClassParam{ "user", SessionClass::User },
                                        SessionClassParam{ "", SessionClass::Unknown }));

TEST_F(TestDLoginSession, propertyDesktop)
{
    m_fakeService->m_desktop = "test string";
    ASSERT_EQ("test string", m_fakeService->desktop());
    EXPECT_EQ("test string", m_dLoginSession->desktop());
}

TEST_F(TestDLoginSession, propertyDisplay)
{
    m_fakeService->m_display = "test string";
    ASSERT_EQ("test string", m_fakeService->display());
    EXPECT_EQ("test string", m_dLoginSession->display());
}

TEST_F(TestDLoginSession, propertyId)
{
    m_fakeService->m_id = "test string";
    ASSERT_EQ("test string", m_fakeService->id());
    EXPECT_EQ("test string", m_dLoginSession->id());
}

TEST_F(TestDLoginSession, propertyName)
{
    m_fakeService->m_name = "test string";
    ASSERT_EQ("test string", m_fakeService->name());
    EXPECT_EQ("test string", m_dLoginSession->name());
}

TEST_F(TestDLoginSession, propertyRemoteHost)
{
    m_fakeService->m_remoteHost = "test string";
    ASSERT_EQ("test string", m_fakeService->remoteHost());
    EXPECT_EQ("test string", m_dLoginSession->remoteHost());
}

TEST_F(TestDLoginSession, propertyRemoteUser)
{
    m_fakeService->m_remoteUser = "test string";
    ASSERT_EQ("test string", m_fakeService->remoteUser());
    EXPECT_EQ("test string", m_dLoginSession->remoteUser());
}

TEST_F(TestDLoginSession, propertyScope)
{
    m_fakeService->m_scope = "test string";
    ASSERT_EQ("test string", m_fakeService->scope());
    EXPECT_EQ("test string", m_dLoginSession->scope());
}

TEST_F(TestDLoginSession, propertyService)
{
    m_fakeService->m_applicationService = "test string";
    ASSERT_EQ("test string", m_fakeService->service());
    EXPECT_EQ("test string", m_dLoginSession->service());
}

struct SessionStateParam
{
    QString strState;
    SessionState enumState;
};

class TestSessionState : public TestDLoginSession,
                         public testing::WithParamInterface<SessionStateParam>
{
public:
    TestSessionState()
        : TestDLoginSession()
    {
    }

    ~TestSessionState() override = default;
};

TEST_P(TestSessionState, state)
{
    auto params = GetParam();
    m_fakeService->m_state = params.strState;
    ASSERT_EQ(params.strState, m_fakeService->state());
    EXPECT_EQ(params.enumState, m_dLoginSession->state());
}

INSTANTIATE_TEST_SUITE_P(Default,
                        TestSessionState,
                        testing::Values(SessionStateParam{ "active", SessionState::Active },
                                        SessionStateParam{ "closing", SessionState::Closing },
                                        SessionStateParam{ "online", SessionState::Online },
                                        SessionStateParam{ "", SessionState::Unknown }));

struct SessionTypeParam
{
    QString strType;
    SessionType enumType;
};

class TestSessionType : public TestDLoginSession,
                        public testing::WithParamInterface<SessionTypeParam>
{
public:
    TestSessionType()
        : TestDLoginSession()
    {
    }

    ~TestSessionType() override = default;
};

TEST_P(TestSessionType, type)
{
    auto params = GetParam();
    m_fakeService->m_type = params.strType;
    ASSERT_EQ(params.strType, m_fakeService->type());
    EXPECT_EQ(params.enumType, m_dLoginSession->type());
}

INSTANTIATE_TEST_SUITE_P(Default,
                        TestSessionType,
                        testing::Values(SessionTypeParam{ "x11", SessionType::X11 },
                                        SessionTypeParam{ "wayland", SessionType::Wayland },
                                        SessionTypeParam{ "tty", SessionType::TTY },
                                        SessionTypeParam{ "mir", SessionType::Mir },
                                        SessionTypeParam{ "unspecified",
                                                          SessionType::Unspecified }));

TEST_F(TestDLoginSession, propertyTTY)
{
    m_fakeService->m_TTY = "test string";
    ASSERT_EQ("test string", m_fakeService->TTY());
    EXPECT_EQ("test string", m_dLoginSession->TTY());
}

TEST_F(TestDLoginSession, seat)
{
    m_fakeService->m_seat.seatId = "seat0";
    m_fakeService->m_seat.path = QDBusObjectPath("/org/freedesktop/login1/seat");
    ASSERT_EQ("seat0", m_fakeService->seat().seatId);
    EXPECT_EQ("seat0", m_dLoginSession->seat());
}

TEST_F(TestDLoginSession, user)
{
    m_fakeService->m_user.userId = 1000;
    m_fakeService->m_user.path = QDBusObjectPath("/org/freedesktop/login1/user");
    ASSERT_EQ(1000, m_fakeService->user().userId);
    EXPECT_EQ(1000, m_dLoginSession->user());
}

TEST_F(TestDLoginSession, propertyAudit)
{
    m_fakeService->m_audit = 1024;
    ASSERT_EQ(1024, m_fakeService->audit());
    EXPECT_EQ(1024, m_dLoginSession->audit());
    m_fakeService->m_audit = 2048;
    ASSERT_EQ(2048, m_fakeService->audit());
    EXPECT_EQ(2048, m_dLoginSession->audit());
}

TEST_F(TestDLoginSession, propertyLeader)
{
    m_fakeService->m_leader = 1024;
    ASSERT_EQ(1024, m_fakeService->leader());
    EXPECT_EQ(1024, m_dLoginSession->leader());
    m_fakeService->m_leader = 2048;
    ASSERT_EQ(2048, m_fakeService->leader());
    EXPECT_EQ(2048, m_dLoginSession->leader());
}

TEST_F(TestDLoginSession, propertyVTNr)
{
    m_fakeService->m_VTNr = 1024;
    ASSERT_EQ(1024, m_fakeService->VTNr());
    EXPECT_EQ(1024, m_dLoginSession->VTNr());
    m_fakeService->m_VTNr = 2048;
    ASSERT_EQ(2048, m_fakeService->VTNr());
    EXPECT_EQ(2048, m_dLoginSession->VTNr());
}

TEST_F(TestDLoginSession, propertyIdleSinceHint)
{
    m_fakeService->m_idleSinceHint = 1000;
    ASSERT_EQ(1000, m_fakeService->idleSinceHint());
    EXPECT_EQ(1, m_dLoginSession->idleSinceHint().toMSecsSinceEpoch());
}

TEST_F(TestDLoginSession, propertyIdleSinceHintMonotonic)
{
    m_fakeService->m_idleSinceHintMonotonic = 1024;
    ASSERT_EQ(1024, m_fakeService->idleSinceHintMonotonic());
    EXPECT_EQ(1024, m_dLoginSession->idleSinceHintMonotonic());
    m_fakeService->m_idleSinceHintMonotonic = 2048;
    ASSERT_EQ(2048, m_fakeService->idleSinceHintMonotonic());
    EXPECT_EQ(2048, m_dLoginSession->idleSinceHintMonotonic());
}

TEST_F(TestDLoginSession, propertyCreatedTime)
{
    m_fakeService->m_timestamp = 1000;
    ASSERT_EQ(1000, m_fakeService->timestamp());
    EXPECT_EQ(1, m_dLoginSession->createdTime().toMSecsSinceEpoch());
}

TEST_F(TestDLoginSession, propertyCreatedTimeMonotonic)
{
    m_fakeService->m_timestampMonotonic = 1024;
    ASSERT_EQ(1024, m_fakeService->timestampMonotonic());
    EXPECT_EQ(1024, m_dLoginSession->createdTimeMonotonic());
    m_fakeService->m_timestampMonotonic = 2048;
    ASSERT_EQ(2048, m_fakeService->timestampMonotonic());
    EXPECT_EQ(2048, m_dLoginSession->createdTimeMonotonic());
}

TEST_F(TestDLoginSession, activate)
{
    m_fakeService->m_active = false;
    m_dLoginSession->activate();
    EXPECT_TRUE(m_fakeService->m_active);
}

TEST_F(TestDLoginSession, kill)
{
    m_fakeService->m_who = "";
    m_fakeService->m_signalNumber = -1;
    m_dLoginSession->kill(SessionRole::All, 8);
    EXPECT_EQ("all", m_fakeService->m_who);
    EXPECT_EQ(8, m_fakeService->m_signalNumber);
}

TEST_F(TestDLoginSession, lock)
{
    m_fakeService->m_lockedHint = false;
    m_dLoginSession->lock();
    EXPECT_TRUE(m_fakeService->m_lockedHint);
}

TEST_F(TestDLoginSession, setIdleHint)
{
    m_fakeService->m_idleHint = false;
    m_dLoginSession->setIdleHint(true);
    EXPECT_TRUE(m_fakeService->m_idleHint);
}

TEST_P(TestSessionType, setType)
{
    auto params = GetParam();
    m_fakeService->m_type = "";
    m_dLoginSession->setType(params.enumType);
    EXPECT_EQ(params.strType, m_fakeService->m_type);
}

TEST_F(TestDLoginSession, terminate)
{
    m_fakeService->m_terminated = false;
    m_dLoginSession->terminate();
    EXPECT_TRUE(m_fakeService->m_terminated);
}

struct AutostartParam
{
    QString homeDir = "";
    QString xdgConfigHome = "";
    QString xdgConfigDirs = "";
    QString xdgCurrentDesktop = "";
    QStringList desktopFiles = {};
    QString userAutostartDirResult = "";
    QStringList systemAutostartDirResult = {};
    QStringList autostartAppsResult = {};
    QString judgeAutostartDesktopFile = "";
    bool judgeAutostartResult = false;
    QString isAutostartFileName = "";
    bool isAutostartResult = false;

    friend std::ostream &operator<<(std::ostream &os, const AutostartParam &param)
    {
        os << "homeDir: " << param.homeDir << " xdgConfigHome: " << param.xdgConfigHome
           << " xdgConfigDirs: " << param.xdgConfigDirs
           << " xdgCurrentDesktop: " << param.xdgCurrentDesktop
           << " desktopFiles: " << param.desktopFiles
           << " userAutostartDirResult: " << param.userAutostartDirResult
           << " systemAutostartDirResult: " << param.systemAutostartDirResult
           << " autostartAppsResult: " << param.autostartAppsResult;
        return os;
    }
};

class EnvGuard
{
public:
    EnvGuard()
        : m_oldHome(qgetenv("HOME"))
        , m_oldXdgConfigHome(qgetenv("XDG_CONFIG_HOME"))
        , m_oldXdgConfigDirs(qgetenv("XDG_CONFIG_DIRS"))
        , m_oldXdgCurrentDesktop(qgetenv("XDG_CURRENT_DESKTOP"))
    {
    }

    ~EnvGuard()
    {
        qputenv("HOME", m_oldHome.toUtf8());
        qputenv("XDG_CONFIG_HOME", m_oldXdgConfigHome.toUtf8());
        qputenv("XDG_CONFIG_DIRS", m_oldXdgConfigDirs.toUtf8());
        qputenv("XDG_CURRENT_DESKTOP", m_oldXdgCurrentDesktop.toUtf8());
    }

private:
    const QString m_oldHome;
    const QString m_oldXdgConfigHome;
    const QString m_oldXdgConfigDirs;
    const QString m_oldXdgCurrentDesktop;
};

class TestAutostart : public TestDLoginSession, public testing::WithParamInterface<AutostartParam>
{
protected:
    TestAutostart()
        : TestDLoginSession()
    {
        Prefix = qApp->applicationDirPath() + "/autostart-test";
        QDir dir;
        dir.mkpath(Prefix);
    }

    ~TestAutostart() override
    {
        QDir dir(Prefix);
        dir.removeRecursively();
    }

    void setHomeDir(const QString &homeDir)
    {
        if (homeDir.isEmpty()) {
            qputenv("HOME", "");
        } else {
            if (!homeDir.startsWith("/")) {
                FAIL();
            }
            qputenv("HOME", catPath(homeDir).toUtf8());
        }
    }

    void setXdgConfigHome(const QString &xdgConfigHome)
    {
        if (xdgConfigHome.isEmpty()) {
            qputenv("XDG_CONFIG_HOME", "");
        } else {
            if (!xdgConfigHome.startsWith("/")) {
                qputenv("XDG_CONFIG_HOME", xdgConfigHome.toUtf8());
            } else {
                qputenv("XDG_CONFIG_HOME", catPath(xdgConfigHome).toUtf8());
            }
        }
    }

    void setXdgConfigDirs(const QString &xdgConfigDirs)
    {
        if (xdgConfigDirs.isEmpty()) {
            qputenv("XDG_CONFIG_DIRS", "");
        } else {
            QStringList configDirs = xdgConfigDirs.split(":");
            for (int i = 0; i < configDirs.size(); i++) {
                if (!configDirs[i].isEmpty() && configDirs[i].startsWith("/")) {
                    configDirs[i] = catPath(configDirs[i]);
                }
            }
            QString configDirVar = configDirs.join(":");
            qputenv("XDG_CONFIG_DIRS", configDirVar.toUtf8());
        }
    }

    static QString catPath(const QString &path)
    {
        if (path.isEmpty()) {
            return path;
        } else {
            return Prefix + path;
        }
    }

    static QString trimPath(const QString &path)
    {
        QString result = path;
        if (result.startsWith(Prefix)) {
            result.replace(Prefix, "");
        }
        return result;
    }

    void presetEnv(const QString &name, const QString &value)
    {
        qputenv(name.toStdString().c_str(), value.toUtf8());
    }

    EnvGuard m_guard;
    static QString Prefix;
};

QString TestAutostart::Prefix = "";

TEST_P(TestAutostart, getUserAutostartDir)
{
    auto params = GetParam();
    setHomeDir(params.homeDir);
    setXdgConfigHome(params.xdgConfigHome);
    QString userAutostartDir = m_dLoginSession->d_ptr->getUserAutostartDir();
    EXPECT_EQ(params.userAutostartDirResult, trimPath(userAutostartDir));
}

TEST_P(TestAutostart, getSystemAutostartDir)
{
    auto params = GetParam();
    setXdgConfigDirs(params.xdgConfigDirs);
    QStringList systemAutostartDir = m_dLoginSession->d_ptr->getSystemAutostartDirs();
    foreach (const auto &autostartDir, systemAutostartDir) {
        if (params.systemAutostartDirResult.contains(trimPath(autostartDir))) {
            params.systemAutostartDirResult.removeAll(trimPath(autostartDir));
        } else {
            FAIL();
        }
    }
    EXPECT_THAT(params.systemAutostartDirResult, testing::IsEmpty());
}

TEST_P(TestAutostart, getAutostartApps)
{
    auto params = GetParam();
    setHomeDir(params.homeDir);
    setXdgConfigHome(params.xdgConfigHome);
    setXdgConfigDirs(params.xdgConfigDirs);
    QString userAutostartDir = m_dLoginSession->d_ptr->getUserAutostartDir();
    QDir dir;
    dir.mkpath(userAutostartDir);
    foreach (const auto &desktopFile, params.desktopFiles) {
        QFileInfo fileInfo(":/" + desktopFile);
        QFile::copy(fileInfo.absoluteFilePath(), userAutostartDir + "/" + fileInfo.fileName());
    }
    QStringList autostartApps = m_dLoginSession->d_ptr->getAutostartApps(userAutostartDir);
    foreach (const auto app, params.autostartAppsResult) {
        if (autostartApps.contains(catPath(app))) {
            autostartApps.removeOne(catPath(app));
        } else {
            FAIL();
        }
    }
    if (params.systemAutostartDirResult.size() == 1
        && params.systemAutostartDirResult[0].startsWith("/etc/xdg")) {
        return;
    }
    EXPECT_THAT(autostartApps, testing::IsEmpty());
    auto eAutostartApps = m_dLoginSession->autostartList();
    ASSERT_TRUE(eAutostartApps.hasValue());
    autostartApps = eAutostartApps.value();
    foreach (const auto app, params.autostartAppsResult) {
        if (autostartApps.contains(catPath(app))) {
            autostartApps.removeOne(catPath(app));
        } else {
            FAIL();
        }
    }
    EXPECT_THAT(autostartApps, testing::IsEmpty());
    QStringList systemAutostartDirs = m_dLoginSession->d_ptr->getSystemAutostartDirs();
    if (!systemAutostartDirs.isEmpty()) {
        QString first = systemAutostartDirs.first();
        dir.mkdir(first);
        foreach (const auto &desktopFile, params.desktopFiles) {
            QFileInfo fileInfo(":/" + desktopFile);
            QFile::copy(fileInfo.absoluteFilePath(), first + "/" + fileInfo.fileName());
        }
    }
    eAutostartApps = m_dLoginSession->autostartList();
    ASSERT_TRUE(eAutostartApps.hasValue());
    autostartApps = eAutostartApps.value();
    foreach (const auto app, params.autostartAppsResult) {
        if (autostartApps.contains(catPath(app))) {
            autostartApps.removeOne(catPath(app));
        } else {
            FAIL();
        }
    }
    EXPECT_THAT(autostartApps, testing::IsEmpty());
}

TEST_P(TestAutostart, judgeAutostart)
{
    auto params = GetParam();
    setHomeDir(params.homeDir);
    setXdgConfigHome(params.xdgConfigHome);
    setXdgConfigDirs(params.xdgConfigDirs);
    QFileInfo fileInfo(":/" + params.judgeAutostartDesktopFile);
    bool result = m_dLoginSession->d_ptr->judgeAutostart(fileInfo.absoluteFilePath());
    EXPECT_EQ(params.judgeAutostartResult, result);
}

TEST_P(TestAutostart, isAutostart)
{
    auto params = GetParam();
    setHomeDir(params.homeDir);
    setXdgConfigHome(params.xdgConfigHome);
    setXdgConfigDirs(params.xdgConfigDirs);
    QString userAutostartDir = m_dLoginSession->d_ptr->getUserAutostartDir();
    QDir dir;
    dir.mkpath(userAutostartDir);
    foreach (const auto &desktopFile, params.desktopFiles) {
        QFileInfo fileInfo(":/" + desktopFile);
        QFile::copy(fileInfo.absoluteFilePath(), userAutostartDir + "/" + fileInfo.fileName());
    }
    QString fileName;
    if (params.isAutostartFileName.startsWith("/")) {
        fileName = catPath(params.isAutostartFileName);
    } else {
        fileName = params.isAutostartFileName;
    }
    auto eResult = m_dLoginSession->isAutostart(fileName);
    ASSERT_TRUE(eResult.hasValue());
    bool result = eResult.value();
    EXPECT_EQ(params.isAutostartResult, result);
}

INSTANTIATE_TEST_SUITE_P(
        Default,
        TestAutostart,
        testing::Values(
                AutostartParam{ .homeDir = "/test",
                                .xdgConfigHome = "",
                                .xdgConfigDirs = "/system/config1:/system/config2",
                                .desktopFiles = { "test-only-show-in-false.desktop",
                                                  "test-only-show-in-true.desktop",
                                                  "test-not-show-in-true.desktop",
                                                  "test-not-show-in-false.desktop",
                                                  "test.desktop",
                                                  "test-hidden.desktop" },
                                .userAutostartDirResult = "/test/.config/autostart",
                                .systemAutostartDirResult = { "/system/config1/autostart",
                                                              "/system/config2/autostart" },
                                .autostartAppsResult = { "/test/.config/autostart/test.desktop",
                                                         "/test/.config/autostart/"
                                                         "test-only-show-in-true.desktop",
                                                         "/test/.config/autostart/"
                                                         "test-not-show-in-true.desktop" },
                                .judgeAutostartDesktopFile = "test-not-show-in-false.desktop",
                                .judgeAutostartResult = false,
                                .isAutostartFileName = "test.desktop",
                                .isAutostartResult = true },
                AutostartParam{ .homeDir = "/test",
                                .xdgConfigHome = "config",
                                .xdgConfigDirs = "",
                                .desktopFiles = { "test-only-show-in-false.desktop",
                                                  "test-only-show-in-true.desktop",
                                                  "test-not-show-in-true.desktop",
                                                  "test-not-show-in-false.desktop",
                                                  "test.desktop",
                                                  "test-hidden.desktop" },
                                .userAutostartDirResult = "/test/.config/autostart",
                                .systemAutostartDirResult = { "/etc/xdg/autostart" },
                                .autostartAppsResult = { "/test/.config/autostart/test.desktop",
                                                         "/test/.config/autostart/"
                                                         "test-only-show-in-true.desktop",
                                                         "/test/.config/autostart/"
                                                         "test-not-show-in-true.desktop" },
                                .judgeAutostartDesktopFile = "test-not-show-in-true.desktop",
                                .judgeAutostartResult = true,
                                .isAutostartFileName = "test-hidden.desktop",
                                .isAutostartResult = false },
                AutostartParam{
                        .homeDir = "/test",
                        .xdgConfigHome = "/config",
                        .xdgConfigDirs = "/system/config1::/system/config2",
                        .desktopFiles = { "test-only-show-in-false.desktop",
                                          "test-only-show-in-true.desktop",
                                          "test-not-show-in-true.desktop",
                                          "test-not-show-in-false.desktop",
                                          "test.desktop",
                                          "test-hidden.desktop" },
                        .userAutostartDirResult = "/config/autostart",
                        .systemAutostartDirResult = { "/system/config1/autostart",
                                                      "/system/config2/autostart" },
                        .autostartAppsResult = { "/config/autostart/test.desktop",
                                                 "/config/autostart/test-only-show-in-true.desktop",
                                                 "/config/autostart/"
                                                 "test-not-show-in-true.desktop" },
                        .judgeAutostartDesktopFile = "test-only-show-in-true.desktop",
                        .judgeAutostartResult = true,
                        .isAutostartFileName = "test-only-show-in-false.desktop",
                        .isAutostartResult = false },
                AutostartParam{
                        .homeDir = "/test",
                        .xdgConfigHome = "/config",
                        .xdgConfigDirs = "/system/config1:config3:/system/config2",
                        .desktopFiles = { "test-only-show-in-false.desktop",
                                          "test-only-show-in-true.desktop",
                                          "test-not-show-in-true.desktop",
                                          "test-not-show-in-false.desktop",
                                          "test.desktop",
                                          "test-hidden.desktop" },
                        .userAutostartDirResult = "/config/autostart",
                        .systemAutostartDirResult = { "/system/config1/autostart",
                                                      "/system/config2/autostart" },
                        .autostartAppsResult = { "/config/autostart/test.desktop",
                                                 "/config/autostart/test-only-show-in-true.desktop",
                                                 "/config/autostart/"
                                                 "test-not-show-in-true.desktop" },
                        .judgeAutostartDesktopFile = "test-only-show-in-false.desktop",
                        .judgeAutostartResult = false,
                        .isAutostartFileName = "test-only-show-in-true.desktop",
                        .isAutostartResult = true },
                AutostartParam{
                        .homeDir = "/test",
                        .xdgConfigHome = "/config",
                        .xdgConfigDirs = "/system/config1:config3:/system/config2",
                        .desktopFiles = { "test-only-show-in-false.desktop",
                                          "test-only-show-in-true.desktop",
                                          "test-not-show-in-true.desktop",
                                          "test-not-show-in-false.desktop",
                                          "test.desktop",
                                          "test-hidden.desktop" },
                        .userAutostartDirResult = "/config/autostart",
                        .systemAutostartDirResult = { "/system/config1/autostart",
                                                      "/system/config2/autostart" },
                        .autostartAppsResult = { "/config/autostart/test.desktop",
                                                 "/config/autostart/test-only-show-in-true.desktop",
                                                 "/config/autostart/"
                                                 "test-not-show-in-true.desktop" },
                        .judgeAutostartDesktopFile = "test-only-show-in-false.desktop",
                        .judgeAutostartResult = false,
                        .isAutostartFileName = "test-not-show-in-true.desktop",
                        .isAutostartResult = true },
                AutostartParam{
                        .homeDir = "/test",
                        .xdgConfigHome = "/config",
                        .xdgConfigDirs = "/system/config1:config3:/system/config2",
                        .desktopFiles = { "test-only-show-in-false.desktop",
                                          "test-only-show-in-true.desktop",
                                          "test-not-show-in-true.desktop",
                                          "test-not-show-in-false.desktop",
                                          "test.desktop",
                                          "test-hidden.desktop" },
                        .userAutostartDirResult = "/config/autostart",
                        .systemAutostartDirResult = { "/system/config1/autostart",
                                                      "/system/config2/autostart" },
                        .autostartAppsResult = { "/config/autostart/test.desktop",
                                                 "/config/autostart/test-only-show-in-true.desktop",
                                                 "/config/autostart/"
                                                 "test-not-show-in-true.desktop" },
                        .judgeAutostartDesktopFile = "test-only-show-in-false.desktop",
                        .judgeAutostartResult = false,
                        .isAutostartFileName = "test-not-show-in-false.desktop",
                        .isAutostartResult = false },
                AutostartParam{
                        .homeDir = "/test",
                        .xdgConfigHome = "/config",
                        .xdgConfigDirs = "/system/config1:config3:/system/config2",
                        .desktopFiles = { "test-only-show-in-false.desktop",
                                          "test-only-show-in-true.desktop",
                                          "test-not-show-in-true.desktop",
                                          "test-not-show-in-false.desktop",
                                          "test.desktop",
                                          "test-hidden.desktop" },
                        .userAutostartDirResult = "/config/autostart",
                        .systemAutostartDirResult = { "/system/config1/autostart",
                                                      "/system/config2/autostart" },
                        .autostartAppsResult = { "/config/autostart/test.desktop",
                                                 "/config/autostart/test-only-show-in-true.desktop",
                                                 "/config/autostart/"
                                                 "test-not-show-in-true.desktop" },
                        .judgeAutostartDesktopFile = "test-only-show-in-false.desktop",
                        .judgeAutostartResult = false,
                        .isAutostartFileName = "test/test.desktop",
                        .isAutostartResult = false },
                AutostartParam{
                        .homeDir = "/test",
                        .xdgConfigHome = "/config",
                        .xdgConfigDirs = "/system/config1:config3:/system/config2",
                        .desktopFiles = { "test-only-show-in-false.desktop",
                                          "test-only-show-in-true.desktop",
                                          "test-not-show-in-true.desktop",
                                          "test-not-show-in-false.desktop",
                                          "test.desktop",
                                          "test-hidden.desktop" },
                        .userAutostartDirResult = "/config/autostart",
                        .systemAutostartDirResult = { "/system/config1/autostart",
                                                      "/system/config2/autostart" },
                        .autostartAppsResult = { "/config/autostart/test.desktop",
                                                 "/config/autostart/test-only-show-in-true.desktop",
                                                 "/config/autostart/"
                                                 "test-not-show-in-true.desktop" },
                        .judgeAutostartDesktopFile = "test-only-show-in-false.desktop",
                        .judgeAutostartResult = false,
                        .isAutostartFileName = "/test/test.desktop",
                        .isAutostartResult = false },
                AutostartParam{
                        .homeDir = "/test",
                        .xdgConfigHome = "/config",
                        .xdgConfigDirs = "/system/config1:config3:/system/config2",
                        .desktopFiles = { "test-only-show-in-false.desktop",
                                          "test-only-show-in-true.desktop",
                                          "test-not-show-in-true.desktop",
                                          "test-not-show-in-false.desktop",
                                          "test.desktop",
                                          "test-hidden.desktop" },
                        .userAutostartDirResult = "/config/autostart",
                        .systemAutostartDirResult = { "/system/config1/autostart",
                                                      "/system/config2/autostart" },
                        .autostartAppsResult = { "/config/autostart/test.desktop",
                                                 "/config/autostart/test-only-show-in-true.desktop",
                                                 "/config/autostart/"
                                                 "test-not-show-in-true.desktop" },
                        .judgeAutostartDesktopFile = "test-only-show-in-false.desktop",
                        .judgeAutostartResult = false,
                        .isAutostartFileName = "/test/.config/autostart/test.desktop",
                        .isAutostartResult = false }));

TEST_F(TestDLoginSession, addAutostart)
{
    m_startManagerService->m_fileName = "";
    m_startManagerService->m_success = false;
    auto eResult = m_dLoginSession->addAutostart("test");
    ASSERT_TRUE(eResult.hasValue());
    bool result = eResult.value();
    EXPECT_FALSE(result);
    EXPECT_EQ("test", m_startManagerService->m_fileName);
    m_startManagerService->m_success = true;
    eResult = m_dLoginSession->addAutostart("test");
    ASSERT_TRUE(eResult.hasValue());
    result = eResult.value();
    EXPECT_TRUE(result);
}

TEST_F(TestDLoginSession, removeAutostart)
{
    m_startManagerService->m_fileName = "";
    m_startManagerService->m_success = false;
    auto eResult = m_dLoginSession->removeAutostart("test");
    ASSERT_TRUE(eResult.hasValue());
    bool result = eResult.value();
    EXPECT_FALSE(result);
    EXPECT_EQ("test", m_startManagerService->m_fileName);
    m_startManagerService->m_success = true;
    eResult = m_dLoginSession->removeAutostart("test");
    ASSERT_TRUE(eResult.hasValue());
    result = eResult.value();
    EXPECT_TRUE(result);
}
