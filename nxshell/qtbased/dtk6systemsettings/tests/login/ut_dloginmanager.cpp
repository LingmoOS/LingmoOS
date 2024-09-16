// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "dloginmanager.h"
#include "dloginmanager_p.h"
#include "dloginseat.h"
#include "dloginseat_p.h"
#include "dloginsession.h"
#include "dloginsession_p.h"
#include "dloginuser.h"
#include "dloginuser_p.h"
#include "dloginutils.h"
#include "login1managerinterface.h"
#include "login1managerservice.h"
#include "login1seatinterface.h"
#include "login1seatservice.h"
#include "login1sessioninterface.h"
#include "login1sessionservice.h"
#include "login1userinterface.h"
#include "login1userservice.h"

#include <ddbusinterface.h>

#include <ostream>

DLOGIN_USE_NAMESPACE

class TestDLoginManager : public testing::Test
{
public:
    TestDLoginManager()
        : m_fakeService(new Login1ManagerService)
        , m_dLoginManager(new DLoginManager)
    {
        m_dLoginManager->d_ptr->m_inter->m_interface->setTimeout(INT_MAX);
    }

    virtual ~TestDLoginManager()
    {
        delete m_dLoginManager;
        delete m_fakeService;
    }

protected:
    Login1ManagerService *m_fakeService;
    DLoginManager *m_dLoginManager;
    static const QString Service;
};

const QString TestDLoginManager::Service = QStringLiteral("org.freedesktop.fakelogin1");

TEST_F(TestDLoginManager, propertyKillExcludeUsers)
{
    m_fakeService->m_killExcludeUsers.clear();
    ASSERT_THAT(m_fakeService->m_killExcludeUsers, testing::IsEmpty());
    m_fakeService->m_killExcludeUsers << "User1000";
    ASSERT_THAT(m_fakeService->m_killExcludeUsers, testing::SizeIs(1));
    auto users = m_dLoginManager->killExcludeUsers();
    ASSERT_THAT(users, testing::SizeIs(1));
    EXPECT_EQ(users.front(), "User1000");
    m_fakeService->m_killExcludeUsers << "User1001";
    users = m_dLoginManager->killExcludeUsers();
    ASSERT_THAT(users, testing::SizeIs(2));
    EXPECT_EQ(users[1], "User1001");
}

TEST_F(TestDLoginManager, propertyKillOnlyUsers)
{
    m_fakeService->m_killOnlyUsers.clear();
    ASSERT_THAT(m_fakeService->m_killOnlyUsers, testing::IsEmpty());
    m_fakeService->m_killOnlyUsers << "User1000";
    ASSERT_THAT(m_fakeService->m_killOnlyUsers, testing::SizeIs(1));
    auto users = m_dLoginManager->killOnlyUsers();
    ASSERT_THAT(users, testing::SizeIs(1));
    EXPECT_EQ(users.front(), "User1000");
    m_fakeService->m_killOnlyUsers << "User1001";
    users = m_dLoginManager->killOnlyUsers();
    ASSERT_THAT(users, testing::SizeIs(2));
    EXPECT_EQ(users[1], "User1001");
}

TEST_F(TestDLoginManager, propertyDocked)
{
    m_fakeService->m_docked = false;
    ASSERT_FALSE(m_fakeService->docked());
    EXPECT_FALSE(m_dLoginManager->docked());
    m_fakeService->m_docked = true;
    ASSERT_TRUE(m_fakeService->docked());
    EXPECT_TRUE(m_dLoginManager->docked());
}

TEST_F(TestDLoginManager, propertyIdleHint)
{
    m_fakeService->m_idleHint = false;
    ASSERT_FALSE(m_fakeService->idleHint());
    EXPECT_FALSE(m_dLoginManager->idleHint());
    m_fakeService->m_idleHint = true;
    ASSERT_TRUE(m_fakeService->idleHint());
    EXPECT_TRUE(m_dLoginManager->idleHint());
}

TEST_F(TestDLoginManager, propertyKillUserProcesses)
{
    m_fakeService->m_killUserProcesses = false;
    ASSERT_FALSE(m_fakeService->killUserProcesses());
    EXPECT_FALSE(m_dLoginManager->killUserProcesses());
    m_fakeService->m_killUserProcesses = true;
    ASSERT_TRUE(m_fakeService->killUserProcesses());
    EXPECT_TRUE(m_dLoginManager->killUserProcesses());
}

TEST_F(TestDLoginManager, propertyLidClosed)
{
    m_fakeService->m_lidClosed = false;
    ASSERT_FALSE(m_fakeService->lidClosed());
    EXPECT_FALSE(m_dLoginManager->lidClosed());
    m_fakeService->m_lidClosed = true;
    ASSERT_TRUE(m_fakeService->lidClosed());
    EXPECT_TRUE(m_dLoginManager->lidClosed());
}

TEST_F(TestDLoginManager, propertyPreparingForShutdown)
{
    m_fakeService->m_preparingForShutdown = false;
    ASSERT_FALSE(m_fakeService->preparingForShutdown());
    EXPECT_FALSE(m_dLoginManager->preparingForShutdown());
    m_fakeService->m_preparingForShutdown = true;
    ASSERT_TRUE(m_fakeService->preparingForShutdown());
    EXPECT_TRUE(m_dLoginManager->preparingForShutdown());
}

TEST_F(TestDLoginManager, propertyPreparingForSleep)
{
    m_fakeService->m_preparingForSleep = false;
    ASSERT_FALSE(m_fakeService->preparingForSleep());
    EXPECT_FALSE(m_dLoginManager->preparingForSleep());
    m_fakeService->m_preparingForSleep = true;
    ASSERT_TRUE(m_fakeService->preparingForSleep());
    EXPECT_TRUE(m_dLoginManager->preparingForSleep());
}

TEST_F(TestDLoginManager, propertyRemoveIPC)
{
    m_fakeService->m_removeIPC = false;
    ASSERT_FALSE(m_fakeService->removeIPC());
    EXPECT_FALSE(m_dLoginManager->removeIPC());
    m_fakeService->m_removeIPC = true;
    ASSERT_TRUE(m_fakeService->removeIPC());
    EXPECT_TRUE(m_dLoginManager->removeIPC());
}

TEST_F(TestDLoginManager, propertyBlockInhibited)
{
    m_fakeService->m_blockInhibited = "test string";
    ASSERT_EQ("test string", m_fakeService->blockInhibited());
    EXPECT_EQ("test string", m_dLoginManager->blockInhibited());
}

TEST_F(TestDLoginManager, propertyDelayInhibited)
{
    m_fakeService->m_delayInhibited = "test string";
    ASSERT_EQ("test string", m_fakeService->delayInhibited());
    EXPECT_EQ("test string", m_dLoginManager->delayInhibited());
}

TEST_F(TestDLoginManager, propertyHandleHibernateKey)
{
    m_fakeService->m_handleHibernateKey = "poweroff";
    ASSERT_EQ("poweroff", m_fakeService->handleHibernateKey());
    EXPECT_EQ(PowerAction::PowerOff, m_dLoginManager->handleHibernateKey());
    m_fakeService->m_handleHibernateKey = "hybrid-sleep";
    ASSERT_EQ("hybrid-sleep", m_fakeService->handleHibernateKey());
    EXPECT_EQ(PowerAction::HybridSleep, m_dLoginManager->handleHibernateKey());
    m_fakeService->m_handleHibernateKey = "";
    ASSERT_EQ("", m_fakeService->handleHibernateKey());
    EXPECT_EQ(PowerAction::Unknown, m_dLoginManager->handleHibernateKey());
}

TEST_F(TestDLoginManager, propertyHandleLidSwitch)
{
    m_fakeService->m_handleLidSwitch = "poweroff";
    ASSERT_EQ("poweroff", m_fakeService->handleLidSwitch());
    EXPECT_EQ(PowerAction::PowerOff, m_dLoginManager->handleLidSwitch());
    m_fakeService->m_handleLidSwitch = "hybrid-sleep";
    ASSERT_EQ("hybrid-sleep", m_fakeService->handleLidSwitch());
    EXPECT_EQ(PowerAction::HybridSleep, m_dLoginManager->handleLidSwitch());
    m_fakeService->m_handleLidSwitch = "";
    ASSERT_EQ("", m_fakeService->handleLidSwitch());
    EXPECT_EQ(PowerAction::Unknown, m_dLoginManager->handleLidSwitch());
}

TEST_F(TestDLoginManager, propertyHandleLidSwitchDocked)
{
    m_fakeService->m_handleLidSwitchDocked = "poweroff";
    ASSERT_EQ("poweroff", m_fakeService->handleLidSwitchDocked());
    EXPECT_EQ(PowerAction::PowerOff, m_dLoginManager->handleLidSwitchDocked());
    m_fakeService->m_handleLidSwitchDocked = "hybrid-sleep";
    ASSERT_EQ("hybrid-sleep", m_fakeService->handleLidSwitchDocked());
    EXPECT_EQ(PowerAction::HybridSleep, m_dLoginManager->handleLidSwitchDocked());
    m_fakeService->m_handleLidSwitchDocked = "";
    ASSERT_EQ("", m_fakeService->handleLidSwitchDocked());
    EXPECT_EQ(PowerAction::Unknown, m_dLoginManager->handleLidSwitchDocked());
}

TEST_F(TestDLoginManager, propertyHandleLidSwitchExternalPower)
{
    m_fakeService->m_handleLidSwitchExternalPower = "poweroff";
    ASSERT_EQ("poweroff", m_fakeService->handleLidSwitchExternalPower());
    EXPECT_EQ(PowerAction::PowerOff, m_dLoginManager->handleLidSwitchExternalPower());
    m_fakeService->m_handleLidSwitchExternalPower = "hybrid-sleep";
    ASSERT_EQ("hybrid-sleep", m_fakeService->handleLidSwitchExternalPower());
    EXPECT_EQ(PowerAction::HybridSleep, m_dLoginManager->handleLidSwitchExternalPower());
    m_fakeService->m_handleLidSwitchExternalPower = "";
    ASSERT_EQ("", m_fakeService->handleLidSwitchExternalPower());
    EXPECT_EQ(PowerAction::Unknown, m_dLoginManager->handleLidSwitchExternalPower());
}

TEST_F(TestDLoginManager, propertyHandlePowerKey)
{
    m_fakeService->m_handlePowerKey = "poweroff";
    ASSERT_EQ("poweroff", m_fakeService->handlePowerKey());
    EXPECT_EQ(PowerAction::PowerOff, m_dLoginManager->handlePowerKey());
    m_fakeService->m_handlePowerKey = "hybrid-sleep";
    ASSERT_EQ("hybrid-sleep", m_fakeService->handlePowerKey());
    EXPECT_EQ(PowerAction::HybridSleep, m_dLoginManager->handlePowerKey());
    m_fakeService->m_handlePowerKey = "";
    ASSERT_EQ("", m_fakeService->handlePowerKey());
    EXPECT_EQ(PowerAction::Unknown, m_dLoginManager->handlePowerKey());
}

TEST_F(TestDLoginManager, propertyHandleSuspendKey)
{
    m_fakeService->m_handleSuspendKey = "poweroff";
    ASSERT_EQ("poweroff", m_fakeService->handleSuspendKey());
    EXPECT_EQ(PowerAction::PowerOff, m_dLoginManager->handleSuspendKey());
    m_fakeService->m_handleSuspendKey = "hybrid-sleep";
    ASSERT_EQ("hybrid-sleep", m_fakeService->handleSuspendKey());
    EXPECT_EQ(PowerAction::HybridSleep, m_dLoginManager->handleSuspendKey());
    m_fakeService->m_handleSuspendKey = "";
    ASSERT_EQ("", m_fakeService->handleSuspendKey());
    EXPECT_EQ(PowerAction::Unknown, m_dLoginManager->handleSuspendKey());
}

TEST_F(TestDLoginManager, propertyIdleAction)
{
    m_fakeService->m_idleAction = "poweroff";
    ASSERT_EQ("poweroff", m_fakeService->idleAction());
    EXPECT_EQ(PowerAction::PowerOff, m_dLoginManager->idleAction());
    m_fakeService->m_idleAction = "hybrid-sleep";
    ASSERT_EQ("hybrid-sleep", m_fakeService->idleAction());
    EXPECT_EQ(PowerAction::HybridSleep, m_dLoginManager->idleAction());
    m_fakeService->m_idleAction = "";
    ASSERT_EQ("", m_fakeService->idleAction());
    EXPECT_EQ(PowerAction::Unknown, m_dLoginManager->idleAction());
}

TEST_F(TestDLoginManager, propertyScheduledShutdown)
{
    m_fakeService->m_scheduledShutdown = { .type = "poweroff", .usec = 1000 };
    ASSERT_EQ("poweroff", m_fakeService->scheduledShutdown().type);
    ASSERT_EQ(1000, m_fakeService->scheduledShutdown().usec);
    EXPECT_EQ(ShutdownType::PowerOff, m_dLoginManager->scheduledShutdown().type);
    EXPECT_EQ(1, m_dLoginManager->scheduledShutdown().time.toMSecsSinceEpoch());
    m_fakeService->m_scheduledShutdown = { .type = "", .usec = 2000 };
    ASSERT_EQ("", m_fakeService->scheduledShutdown().type);
    ASSERT_EQ(2000, m_fakeService->scheduledShutdown().usec);
    EXPECT_EQ(ShutdownType::Unknown, m_dLoginManager->scheduledShutdown().type);
    EXPECT_EQ(2, m_dLoginManager->scheduledShutdown().time.toMSecsSinceEpoch());
}

TEST_F(TestDLoginManager, propertyNAutoVTs)
{
    m_fakeService->m_nAutoVTs = 1024;
    ASSERT_EQ(1024, m_fakeService->nAutoVTs());
    EXPECT_EQ(1024, m_dLoginManager->nAutoVTs());
    m_fakeService->m_nAutoVTs = 2048;
    ASSERT_EQ(2048, m_fakeService->nAutoVTs());
    EXPECT_EQ(2048, m_dLoginManager->nAutoVTs());
}

TEST_F(TestDLoginManager, propertyHoldoffTimeoutUSec)
{
    m_fakeService->m_holdoffTimeoutUSec = 1024;
    ASSERT_EQ(1024, m_fakeService->holdoffTimeoutUSec());
    EXPECT_EQ(1024, m_dLoginManager->holdoffTimeoutUSec());
    m_fakeService->m_holdoffTimeoutUSec = 2048;
    ASSERT_EQ(2048, m_fakeService->holdoffTimeoutUSec());
    EXPECT_EQ(2048, m_dLoginManager->holdoffTimeoutUSec());
}

TEST_F(TestDLoginManager, propertyIdleActionUSec)
{
    m_fakeService->m_idleActionUSec = 1024;
    ASSERT_EQ(1024, m_fakeService->idleActionUSec());
    EXPECT_EQ(1024, m_dLoginManager->idleActionUSec());
    m_fakeService->m_idleActionUSec = 2048;
    ASSERT_EQ(2048, m_fakeService->idleActionUSec());
    EXPECT_EQ(2048, m_dLoginManager->idleActionUSec());
}

TEST_F(TestDLoginManager, propertyIdleSinceHint)
{
    m_fakeService->m_idleSinceHint = 1000;
    ASSERT_EQ(1000, m_fakeService->idleSinceHint());
    EXPECT_EQ(1, m_dLoginManager->idleSinceHint().toMSecsSinceEpoch());
}

TEST_F(TestDLoginManager, propertyIdleSinceHintMonotonic)
{
    m_fakeService->m_idleSinceHintMonotonic = 1000;
    ASSERT_EQ(1000, m_fakeService->idleSinceHintMonotonic());
    EXPECT_EQ(1000, m_dLoginManager->idleSinceHintMonotonic());
}

TEST_F(TestDLoginManager, propertyInhibitDelayMaxUSec)
{
    m_fakeService->m_inhibitDelayMaxUSec = 1024;
    ASSERT_EQ(1024, m_fakeService->inhibitDelayMaxUSec());
    EXPECT_EQ(1024, m_dLoginManager->inhibitDelayMaxUSec());
    m_fakeService->m_inhibitDelayMaxUSec = 2048;
    ASSERT_EQ(2048, m_fakeService->inhibitDelayMaxUSec());
    EXPECT_EQ(2048, m_dLoginManager->inhibitDelayMaxUSec());
}

TEST_F(TestDLoginManager, propertyInhibitorsMax)
{
    m_fakeService->m_inhibitorsMax = 1024;
    ASSERT_EQ(1024, m_fakeService->inhibitorsMax());
    EXPECT_EQ(1024, m_dLoginManager->inhibitorsMax());
    m_fakeService->m_inhibitorsMax = 2048;
    ASSERT_EQ(2048, m_fakeService->inhibitorsMax());
    EXPECT_EQ(2048, m_dLoginManager->inhibitorsMax());
}

TEST_F(TestDLoginManager, propertyNCurrentInhibitors)
{
    m_fakeService->m_nCurrentInhibitors = 1024;
    ASSERT_EQ(1024, m_fakeService->nCurrentInhibitors());
    EXPECT_EQ(1024, m_dLoginManager->nCurrentInhibitors());
    m_fakeService->m_nCurrentInhibitors = 2048;
    ASSERT_EQ(2048, m_fakeService->nCurrentInhibitors());
    EXPECT_EQ(2048, m_dLoginManager->nCurrentInhibitors());
}

TEST_F(TestDLoginManager, propertyNCurrentSessions)
{
    m_fakeService->m_nCurrentSessions = 1024;
    ASSERT_EQ(1024, m_fakeService->nCurrentSessions());
    EXPECT_EQ(1024, m_dLoginManager->nCurrentSessions());
    m_fakeService->m_nCurrentSessions = 2048;
    ASSERT_EQ(2048, m_fakeService->nCurrentSessions());
    EXPECT_EQ(2048, m_dLoginManager->nCurrentSessions());
}

TEST_F(TestDLoginManager, propertyRuntimeDirectoryInodesMax)
{
    m_fakeService->m_runtimeDirectoryInodesMax = 1024;
    ASSERT_EQ(1024, m_fakeService->runtimeDirectoryInodesMax());
    EXPECT_EQ(1024, m_dLoginManager->runtimeDirectoryInodesMax());
    m_fakeService->m_runtimeDirectoryInodesMax = 2048;
    ASSERT_EQ(2048, m_fakeService->runtimeDirectoryInodesMax());
    EXPECT_EQ(2048, m_dLoginManager->runtimeDirectoryInodesMax());
}

TEST_F(TestDLoginManager, propertyRuntimeDirectorySize)
{
    m_fakeService->m_runtimeDirectorySize = 1024;
    ASSERT_EQ(1024, m_fakeService->runtimeDirectorySize());
    EXPECT_EQ(1024, m_dLoginManager->runtimeDirectorySize());
    m_fakeService->m_runtimeDirectorySize = 2048;
    ASSERT_EQ(2048, m_fakeService->runtimeDirectorySize());
    EXPECT_EQ(2048, m_dLoginManager->runtimeDirectorySize());
}

TEST_F(TestDLoginManager, propertySessionsMax)
{
    m_fakeService->m_sessionsMax = 1024;
    ASSERT_EQ(1024, m_fakeService->sessionsMax());
    EXPECT_EQ(1024, m_dLoginManager->sessionsMax());
    m_fakeService->m_sessionsMax = 2048;
    ASSERT_EQ(2048, m_fakeService->sessionsMax());
    EXPECT_EQ(2048, m_dLoginManager->sessionsMax());
}

TEST_F(TestDLoginManager, propertyUserStopDelayUSec)
{
    m_fakeService->m_userStopDelayUSec = 1024;
    ASSERT_EQ(1024, m_fakeService->userStopDelayUSec());
    EXPECT_EQ(1024, m_dLoginManager->userStopDelayUSec());
    m_fakeService->m_userStopDelayUSec = 2048;
    ASSERT_EQ(2048, m_fakeService->userStopDelayUSec());
    EXPECT_EQ(2048, m_dLoginManager->userStopDelayUSec());
}

TEST_F(TestDLoginManager, activateSession)
{
    m_fakeService->m_sessionId = "session0";
    m_dLoginManager->activateSession("session1");
    EXPECT_EQ("session1", m_fakeService->m_sessionId);
}

TEST_F(TestDLoginManager, activateSessionOnSeat)
{
    m_fakeService->m_sessionId = "session0";
    m_fakeService->m_seatId = "seat0";
    m_dLoginManager->activateSessionOnSeat("session1", "seat1");
    EXPECT_EQ("session1", m_fakeService->m_sessionId);
    EXPECT_EQ("seat1", m_fakeService->m_seatId);
}

struct ExecuteQuery
{
    QString query;
    ExecuteStatus result;

    friend std::ostream &operator<<(std::ostream &os, const ExecuteQuery &executeQuery)
    {
        os << "{query: \"" << executeQuery.query.toStdString() << "\", "
           << "result: " << static_cast<int>(executeQuery.result) << "}";
        return os;
    }
};

class TestExecuteQuery : public TestDLoginManager, public testing::WithParamInterface<ExecuteQuery>
{
public:
    TestExecuteQuery()
        : TestDLoginManager()
    {
    }

    ~TestExecuteQuery() override = default;
};

TEST_P(TestExecuteQuery, canHalt)
{
    auto params = GetParam();
    m_fakeService->m_canHalt = params.query;
    ASSERT_EQ(params.query, m_fakeService->CanHalt());
    EXPECT_EQ(m_dLoginManager->canHalt(), params.result);
}

TEST_P(TestExecuteQuery, canHibernate)
{
    auto params = GetParam();
    m_fakeService->m_canHibernate = params.query;
    ASSERT_EQ(params.query, m_fakeService->CanHibernate());
    EXPECT_EQ(m_dLoginManager->canHibernate(), params.result);
}

TEST_P(TestExecuteQuery, canHybridSleep)
{
    auto params = GetParam();
    m_fakeService->m_canHybridSleep = params.query;
    ASSERT_EQ(params.query, m_fakeService->CanHybridSleep());
    EXPECT_EQ(m_dLoginManager->canHybridSleep(), params.result);
}

TEST_P(TestExecuteQuery, canPowerOff)
{
    auto params = GetParam();
    m_fakeService->m_canPowerOff = params.query;
    ASSERT_EQ(params.query, m_fakeService->CanPowerOff());
    EXPECT_EQ(m_dLoginManager->canPowerOff(), params.result);
}

TEST_P(TestExecuteQuery, canReboot)
{
    auto params = GetParam();
    m_fakeService->m_canReboot = params.query;
    ASSERT_EQ(params.query, m_fakeService->CanReboot());
    EXPECT_EQ(m_dLoginManager->canReboot(), params.result);
}

TEST_P(TestExecuteQuery, canSuspend)
{
    auto params = GetParam();
    m_fakeService->m_canSuspend = params.query;
    ASSERT_EQ(params.query, m_fakeService->CanSuspend());
    EXPECT_EQ(m_dLoginManager->canSuspend(), params.result);
}

TEST_P(TestExecuteQuery, canSuspendThenHibernate)
{
    auto params = GetParam();
    m_fakeService->m_canSuspendThenHibernate = params.query;
    ASSERT_EQ(params.query, m_fakeService->CanSuspendThenHibernate());
    EXPECT_EQ(m_dLoginManager->canSuspendThenHibernate(), params.result);
}

INSTANTIATE_TEST_SUITE_P(Default,
                        TestExecuteQuery,
                        testing::Values(ExecuteQuery{ "yes", ExecuteStatus::Yes },
                                        ExecuteQuery{ "no", ExecuteStatus::No },
                                        ExecuteQuery{ "na", ExecuteStatus::Na },
                                        ExecuteQuery{ "challenge", ExecuteStatus::Challenge },
                                        ExecuteQuery{ "", ExecuteStatus::Unknown },
                                        ExecuteQuery{ "fsadjfkasjhfo", ExecuteStatus::Unknown }));

TEST_F(TestDLoginManager, cancelScheduledShutdown)
{
    ASSERT_TRUE(m_dLoginManager->cancelScheduledShutdown().hasValue());
    m_fakeService->m_cancelScheduledShutdown = false;
    EXPECT_EQ(false, m_dLoginManager->cancelScheduledShutdown().value());
    m_fakeService->m_cancelScheduledShutdown = true;
    EXPECT_EQ(true, m_dLoginManager->cancelScheduledShutdown().value());
}

TEST_F(TestDLoginManager, findSeatById)
{
    QString testPath = "/org/freedesktop/login1/seat0";
    Login1SeatService seat(Service, testPath);
    m_fakeService->m_seatPath = QDBusObjectPath(testPath);
    m_fakeService->m_seatId = "seat1";
    auto dSeat = m_dLoginManager->findSeatById("seat0");
    ASSERT_TRUE(dSeat.hasValue());
    auto vSeat = dSeat.value();
    EXPECT_EQ("seat0", m_fakeService->m_seatId);
    ASSERT_NE(vSeat->d_ptr, nullptr);
    ASSERT_NE(vSeat->d_ptr->m_inter, nullptr);
    EXPECT_EQ(testPath, vSeat->d_ptr->m_inter->m_path);
}

TEST_F(TestDLoginManager, findSessionById)
{
    QString testPath = QStringLiteral("/org/freedesktop/login1/session0");
    Login1SessionService session(Service, testPath);
    m_fakeService->m_sessionPath = QDBusObjectPath(testPath);
    m_fakeService->m_sessionId = "session1";
    auto dSession = m_dLoginManager->findSessionById("session0");
    ASSERT_TRUE(dSession.hasValue());
    auto vSession = dSession.value();
    EXPECT_EQ("session0", m_fakeService->m_sessionId);
    ASSERT_NE(vSession->d_ptr, nullptr);
    ASSERT_NE(vSession->d_ptr->m_inter, nullptr);
    EXPECT_EQ(testPath, vSession->d_ptr->m_inter->m_path);
}

TEST_F(TestDLoginManager, findSessionByPID)
{
    QString testPath = QStringLiteral("/org/freedesktop/login1/session0");
    Login1SessionService session(Service, testPath);
    m_fakeService->m_sessionPath = QDBusObjectPath(testPath);
    m_fakeService->m_PID = 1024;
    auto dSession = m_dLoginManager->findSessionByPID(2048);
    ASSERT_TRUE(dSession.hasValue());
    auto vSession = dSession.value();
    EXPECT_EQ(2048, m_fakeService->m_PID);
    ASSERT_NE(vSession->d_ptr, nullptr);
    ASSERT_NE(vSession->d_ptr->m_inter, nullptr);
    EXPECT_EQ(testPath, vSession->d_ptr->m_inter->m_path);
}

TEST_F(TestDLoginManager, findUserById)
{
    QString testPath = QStringLiteral("/org/freedesktop/login1/user1000");
    Login1UserService user(Service, testPath);
    m_fakeService->m_userPath = QDBusObjectPath(testPath);
    m_fakeService->m_UID = 1024;
    auto dUser = m_dLoginManager->findUserById(2048);
    ASSERT_TRUE(dUser.hasValue());
    auto vUser = dUser.value();
    EXPECT_EQ(2048, m_fakeService->m_UID);
    ASSERT_NE(vUser->d_ptr, nullptr);
    ASSERT_NE(vUser->d_ptr->m_inter, nullptr);
    EXPECT_EQ(testPath, vUser->d_ptr->m_inter->m_path);
}

TEST_F(TestDLoginManager, findUserByPID)
{
    QString testPath = QStringLiteral("/org/freedesktop/login1/user1000");
    Login1UserService user(Service, testPath);
    m_fakeService->m_userPath = QDBusObjectPath(testPath);
    m_fakeService->m_PID = 1024;
    auto dUser = m_dLoginManager->findUserByPID(2048);
    ASSERT_TRUE(dUser.hasValue());
    auto vUser = dUser.value();
    EXPECT_EQ(2048, m_fakeService->m_PID);
    ASSERT_NE(vUser->d_ptr, nullptr);
    ASSERT_NE(vUser->d_ptr->m_inter, nullptr);
    EXPECT_EQ(testPath, vUser->d_ptr->m_inter->m_path);
}

TEST_F(TestDLoginManager, halt)
{
    m_fakeService->m_haltInteractive = false;
    m_dLoginManager->halt(true);
    EXPECT_TRUE(m_fakeService->m_haltInteractive);
}

TEST_F(TestDLoginManager, hibernate)
{
    m_fakeService->m_hibernateInteractive = false;
    m_dLoginManager->hibernate(true);
    EXPECT_TRUE(m_fakeService->m_hibernateInteractive);
}

TEST_F(TestDLoginManager, hybridSleep)
{
    m_fakeService->m_hybridSleepInteractive = false;
    m_dLoginManager->hybridSleep(true);
    EXPECT_TRUE(m_fakeService->m_hybridSleepInteractive);
}

TEST_F(TestDLoginManager, powerOff)
{
    m_fakeService->m_powerOffInteractive = false;
    m_dLoginManager->powerOff(true);
    EXPECT_TRUE(m_fakeService->m_powerOffInteractive);
}

TEST_F(TestDLoginManager, reboot)
{
    m_fakeService->m_rebootInteractive = false;
    m_dLoginManager->reboot(true);
    EXPECT_TRUE(m_fakeService->m_rebootInteractive);
}

TEST_F(TestDLoginManager, suspend)
{
    m_fakeService->m_suspendInteractive = false;
    m_dLoginManager->suspend(true);
    EXPECT_TRUE(m_fakeService->m_suspendInteractive);
}

TEST_F(TestDLoginManager, suspendThenHibernate)
{
    m_fakeService->m_suspendThenHibernateInteractive = false;
    m_dLoginManager->suspendThenHibernate(true);
    EXPECT_TRUE(m_fakeService->m_suspendThenHibernateInteractive);
}

struct InhibitParam
{
    int what;
    QString strWhat;
    QString who;
    QString why;
    InhibitMode mode;
    QString strMode;
    int fd;

    friend std::ostream &operator<<(std::ostream &os, const InhibitParam &param)
    {
        os << "{what: " << param.what << " strWhat: " << param.strWhat << " who: " << param.who
           << " why: " << param.why << " mode: " << static_cast<int>(param.mode)
           << " strMode: " << param.strMode << " fd: " << param.fd << "}";
        return os;
    }
};

class TestInhibit : public TestDLoginManager, public testing::WithParamInterface<InhibitParam>
{
public:
    TestInhibit()
        : TestDLoginManager()
    {
    }

    ~TestInhibit() override = default;
};

TEST_P(TestInhibit, inhibit)
{
    auto params = GetParam();
    auto &dbusInhibitor = m_fakeService->m_inhibitor;
    dbusInhibitor.what = "";
    dbusInhibitor.who = "";
    dbusInhibitor.why = "";
    dbusInhibitor.mode = "";
    m_fakeService->m_inhibitFileDescriptor.giveFileDescriptor(params.fd);
    auto eFd = m_dLoginManager->inhibit(params.what, params.who, params.why, params.mode);
    ASSERT_TRUE(eFd.hasValue());
    auto fd = eFd.value();
    EXPECT_EQ(fd, params.fd);
    QStringList expected = params.strWhat.split(":");
    QStringList dbusWhat = dbusInhibitor.what.split(":");
    foreach (const auto &behavior, expected) {
        if (dbusWhat.contains(behavior)) {
            dbusWhat.removeAll(behavior);
        } else {
            FAIL();
        }
    }
    EXPECT_THAT(dbusWhat, testing::IsEmpty());
    EXPECT_EQ(params.who, dbusInhibitor.who);
    EXPECT_EQ(params.why, dbusInhibitor.why);
    EXPECT_EQ(params.strMode, dbusInhibitor.mode);
}

INSTANTIATE_TEST_SUITE_P(Default,
                        TestInhibit,
                        testing::Values(InhibitParam{ InhibitBehavior::HandleHibernateKey,
                                                      "handle-hibernate-key",
                                                      "uos",
                                                      "test",
                                                      InhibitMode::Block,
                                                      "block",
                                                      3 },
                                        InhibitParam{ InhibitBehavior::HandleHibernateKey
                                                              | InhibitBehavior::Shutdown,
                                                      "handle-hibernate-key:shutdown",
                                                      "uos",
                                                      "test",
                                                      InhibitMode::Block,
                                                      "block",
                                                      3 },
                                        InhibitParam{ InhibitBehavior::HandleHibernateKey
                                                              | InhibitBehavior::Shutdown
                                                              | InhibitBehavior::Sleep,
                                                      "handle-hibernate-key:shutdown:sleep",
                                                      "uos",
                                                      "test",
                                                      InhibitMode::Block,
                                                      "block",
                                                      3 },
                                        InhibitParam{ InhibitBehavior::HandleHibernateKey,
                                                      "handle-hibernate-key",
                                                      "uos",
                                                      "test",
                                                      InhibitMode::Delay,
                                                      "delay",
                                                      3 }));

TEST_F(TestDLoginManager, killSession)
{
    m_fakeService->m_sessionId = "session1";
    m_fakeService->m_sessionRole = "";
    m_fakeService->m_signalNumber = -1;
    m_dLoginManager->killSession("session0", SessionRole::All, 8);
    EXPECT_EQ("session0", m_fakeService->m_sessionId);
    EXPECT_EQ("all", m_fakeService->m_sessionRole);
    EXPECT_EQ(8, m_fakeService->m_signalNumber);
    m_dLoginManager->killSession("session0", SessionRole::Leader, 8);
    EXPECT_EQ("leader", m_fakeService->m_sessionRole);
    m_dLoginManager->killSession("session0", SessionRole::Unknown, 8);
    EXPECT_EQ("", m_fakeService->m_sessionRole);
}

TEST_F(TestDLoginManager, killUser)
{
    m_fakeService->m_UID = 1024;
    m_fakeService->m_signalNumber = -1;
    m_dLoginManager->killUser(2048, 8);
    EXPECT_EQ(2048, m_fakeService->m_UID);
    EXPECT_EQ(8, m_fakeService->m_signalNumber);
}

TEST_F(TestDLoginManager, listInhibitors)
{
    m_fakeService->m_inhibitors = {
        DBusInhibitor{ "sleep:shutdown", "test", "test", "block", 1000, 1 },
        DBusInhibitor{ "handle-hibernate-key:handle-power-key", "uos", "test", "delay", 1001, 2 }
    };
    auto eInhibitors = m_dLoginManager->listInhibitors();
    ASSERT_TRUE(eInhibitors.hasValue());
    auto inhibitors = eInhibitors.value();
    ASSERT_THAT(inhibitors, testing::SizeIs(2));
    EXPECT_EQ(InhibitBehavior::Sleep | InhibitBehavior::Shutdown, inhibitors[0].what);
    EXPECT_EQ("test", inhibitors[0].who);
    EXPECT_EQ("test", inhibitors[0].why);
    EXPECT_EQ(InhibitMode::Block, inhibitors[0].mode);
    EXPECT_EQ(1000, inhibitors[0].UID);
    EXPECT_EQ(1, inhibitors[0].PID);
}

TEST_F(TestDLoginManager, listSeats)
{
    auto &dbusSeats = m_fakeService->m_seats;
    dbusSeats = { { "seat0", QDBusObjectPath("/org/freedesktop/login1/seat0") },
                  { "seat1", QDBusObjectPath("/org/freedesktop/login1/seat1") },
                  { "seat2", QDBusObjectPath("/org/freedesktop/login1/seat2") } };

    auto eSeats = m_dLoginManager->listSeats();
    ASSERT_TRUE(eSeats.hasValue());
    auto seats = eSeats.value();
    ASSERT_THAT(seats, testing::SizeIs(3));
    EXPECT_EQ("seat0", seats[0]);
    EXPECT_EQ("seat1", seats[1]);
    EXPECT_EQ("seat2", seats[2]);
}

TEST_F(TestDLoginManager, listSessions)
{
    auto &dbusSessions = m_fakeService->m_sessions;
    dbusSessions = {
        { "session0", 1000, "test0", "seat0", QDBusObjectPath("/org/freedesktop/login1/session0") },
        { "session1", 1001, "test1", "seat1", QDBusObjectPath("/org/freedesktop/login1/session1") },
        { "session2", 1002, "test2", "seat2", QDBusObjectPath("/org/freedesktop/login1/session2") }
    };
    auto eSessions = m_dLoginManager->listSessions();
    ASSERT_TRUE(eSessions.hasValue());
    auto sessions = eSessions.value();
    ASSERT_THAT(sessions, testing::SizeIs(3));
    EXPECT_EQ("session0", sessions[0]);
    EXPECT_EQ("session1", sessions[1]);
    EXPECT_EQ("session2", sessions[2]);
}

TEST_F(TestDLoginManager, listUsers)
{
    auto &dbusUsers = m_fakeService->m_users;
    dbusUsers = { { 1000, "test0", QDBusObjectPath("/org/freedesktop/login1/user1000") },
                  { 1001, "test1", QDBusObjectPath("/org/freedesktop/login1/user1001") },
                  { 1002, "test2", QDBusObjectPath("/org/freedesktop/login1/user1002") } };
    auto eUsers = m_dLoginManager->listUsers();
    ASSERT_TRUE(eUsers.hasValue());
    auto users = eUsers.value();
    ASSERT_THAT(users, testing::SizeIs(3));
    EXPECT_EQ(1000, users[0]);
    EXPECT_EQ(1001, users[1]);
    EXPECT_EQ(1002, users[2]);
}

TEST_F(TestDLoginManager, lockSession)
{
    m_fakeService->m_sessionId = "session1";
    m_dLoginManager->lockSession("session0");
    EXPECT_EQ("session0", m_fakeService->m_sessionId);
}

struct ScheduleShutdownParam
{
    ShutdownType type;
    QString strType;
    quint64 usec;
};

class TestScheduleShutdown : public TestDLoginManager,
                             public testing::WithParamInterface<ScheduleShutdownParam>
{
    TestScheduleShutdown()
        : TestDLoginManager()
    {
    }

    ~TestScheduleShutdown() override = default;
};

TEST_P(TestScheduleShutdown, scheduleShutdown)
{
    auto params = GetParam();
    m_fakeService->m_scheduledShutdown.type = "";
    m_fakeService->m_scheduledShutdown.usec = 0;
    m_dLoginManager->scheduleShutdown(params.type, QDateTime::fromMSecsSinceEpoch(params.usec));
    EXPECT_EQ(params.strType, m_fakeService->m_scheduledShutdown.type);
    EXPECT_EQ(params.usec * 1000, m_fakeService->m_scheduledShutdown.usec);
}

INSTANTIATE_TEST_SUITE_P(
        Default,
        TestScheduleShutdown,
        testing::Values(ScheduleShutdownParam{ ShutdownType::Halt, "halt", 1000 },
                        ScheduleShutdownParam{ ShutdownType::DryHalt, "dry-halt", 1000 },
                        ScheduleShutdownParam{ ShutdownType::Reboot, "reboot", 1000 },
                        ScheduleShutdownParam{ ShutdownType::DryReboot, "dry-reboot", 1000 },
                        ScheduleShutdownParam{ ShutdownType::PowerOff, "poweroff", 1000 },
                        ScheduleShutdownParam{ ShutdownType::DryPowerOff, "dry-poweroff", 1000 },
                        ScheduleShutdownParam{ ShutdownType::Unknown, "", 1000 }));

TEST_F(TestDLoginManager, setUserLinger)
{
    m_fakeService->m_UID = 1024;
    m_fakeService->m_userLinger = false;
    m_fakeService->m_userLingerInteractive = false;
    m_dLoginManager->setUserLinger(2048, true, true);
    EXPECT_EQ(2048, m_fakeService->m_UID);
    EXPECT_TRUE(m_fakeService->m_userLinger);
    EXPECT_TRUE(m_fakeService->m_userLingerInteractive);
}

TEST_F(TestDLoginManager, terminateSession)
{
    m_fakeService->m_sessionId = "session1";
    m_dLoginManager->terminateSession("session0");
    EXPECT_EQ("session0", m_fakeService->m_sessionId);
}

TEST_F(TestDLoginManager, terminateUser)
{
    m_fakeService->m_UID = 1024;
    m_dLoginManager->terminateUser(2048);
    EXPECT_EQ(2048, m_fakeService->m_UID);
}

TEST_F(TestDLoginManager, logout)
{
    QString testPath = QStringLiteral("/org/freedesktop/login1/session/self");
    Login1SessionService session(Service, testPath);
    session.m_terminated = false;
    m_fakeService->m_sessionPath = QDBusObjectPath(testPath);
    m_dLoginManager->logout();
    EXPECT_TRUE(session.m_terminated);
}

TEST_F(TestDLoginManager, currentSeat)
{
    QString testPath = QStringLiteral("/org/freedesktop/login1/seat/self");
    Login1SeatService seat(Service, testPath);
    m_fakeService->m_seatPath = QDBusObjectPath(testPath);
    auto dSeat = m_dLoginManager->currentSeat();
    ASSERT_TRUE(dSeat.hasValue());
    auto vSeat = dSeat.value();
    ASSERT_NE(vSeat->d_ptr, nullptr);
    ASSERT_NE(vSeat->d_ptr->m_inter, nullptr);
    EXPECT_EQ(testPath, vSeat->d_ptr->m_inter->m_path);
}

TEST_F(TestDLoginManager, currentSession)
{
    QString testPath = QStringLiteral("/org/freedesktop/login1/session/self");
    Login1SessionService session(Service, testPath);
    m_fakeService->m_sessionPath = QDBusObjectPath(testPath);
    auto dSession = m_dLoginManager->currentSession();
    ASSERT_TRUE(dSession.hasValue());
    auto vSession = dSession.value();
    ASSERT_NE(vSession->d_ptr, nullptr);
    ASSERT_NE(vSession->d_ptr->m_inter, nullptr);
    EXPECT_EQ(testPath, vSession->d_ptr->m_inter->m_path);
}

TEST_F(TestDLoginManager, currentUser)
{
    QString testPath = QStringLiteral("/org/freedesktop/login1/user/self");
    Login1UserService user(Service, testPath);
    m_fakeService->m_userPath = QDBusObjectPath(testPath);
    auto dUser = m_dLoginManager->currentUser();
    ASSERT_TRUE(dUser.hasValue());
    auto vUser = dUser.value();
    ASSERT_NE(vUser->d_ptr, nullptr);
    ASSERT_NE(vUser->d_ptr->m_inter, nullptr);
    EXPECT_EQ(testPath, vUser->d_ptr->m_inter->m_path);
}
