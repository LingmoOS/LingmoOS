// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_dbuslogin1manager.h"
#include "dbuslogin1manager.h"

UT_DBusLogin1Manager::UT_DBusLogin1Manager()
{
}

void UT_DBusLogin1Manager::SetUp()
{
    m_loginManager = new DBusLogin1Manager(
        "org.freedesktop.logintest",
        "/org/freedesktop/logintest",
        QDBusConnection::systemBus(), this);
}

void UT_DBusLogin1Manager::TearDown()
{
    delete m_loginManager;
    m_loginManager = nullptr;
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_staticInterfaceName_001)
{
    m_loginManager->staticInterfaceName();
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_blockInhibited_001)
{
    EXPECT_TRUE(m_loginManager->blockInhibited().isEmpty());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_controlGroupHierarchy_001)
{
    EXPECT_TRUE(m_loginManager->controlGroupHierarchy().isEmpty());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_controllers_001)
{
    EXPECT_TRUE(m_loginManager->controllers().isEmpty());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_delayInhibited_001)
{
    EXPECT_TRUE(m_loginManager->delayInhibited().isEmpty());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_handleHibernateKey_001)
{
    EXPECT_TRUE(m_loginManager->handleHibernateKey().isEmpty());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_handleLidSwitch_001)
{
    EXPECT_TRUE(m_loginManager->handleLidSwitch().isEmpty());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_handlePowerKey_001)
{
    EXPECT_TRUE(m_loginManager->handlePowerKey().isEmpty());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_handleSuspendKey_001)
{
    EXPECT_TRUE(m_loginManager->handleSuspendKey().isEmpty());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_idleAction_001)
{
    EXPECT_TRUE(m_loginManager->idleAction().isEmpty());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_idleActionUSec_001)
{
    EXPECT_EQ(0, m_loginManager->idleActionUSec());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_idleHint_001)
{
    EXPECT_FALSE(m_loginManager->idleHint());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_idleSinceHint_001)
{
    EXPECT_FALSE(m_loginManager->idleSinceHint());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_idleSinceHintMonotonic_001)
{
    EXPECT_FALSE(m_loginManager->idleSinceHintMonotonic());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_inhibitDelayMaxUSec_001)
{
    EXPECT_FALSE(m_loginManager->inhibitDelayMaxUSec());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_killExcludeUsers_001)
{
    EXPECT_TRUE(m_loginManager->killExcludeUsers().isEmpty());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_killOnlyUsers_001)
{
    EXPECT_TRUE(m_loginManager->killOnlyUsers().isEmpty());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_killUserProcesses_001)
{
    EXPECT_FALSE(m_loginManager->killUserProcesses());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_nAutoVTs_001)
{
    EXPECT_FALSE(m_loginManager->nAutoVTs());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_preparingForShutdown_001)
{
    EXPECT_FALSE(m_loginManager->preparingForShutdown());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_preparingForSleep_001)
{
    EXPECT_FALSE(m_loginManager->preparingForSleep());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_resetControllers_001)
{
    EXPECT_TRUE(m_loginManager->resetControllers().isEmpty());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_ActivateSession_001)
{
    EXPECT_FALSE(m_loginManager->ActivateSession("").isValid());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_ActivateSessionOnSeat_001)
{
    EXPECT_FALSE(m_loginManager->ActivateSessionOnSeat("", "").isValid());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_AttachDevice_001)
{
    EXPECT_FALSE(m_loginManager->AttachDevice("", "", false).isValid());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_CanHibernate_001)
{
    EXPECT_FALSE(m_loginManager->CanHibernate().isValid());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_CanHybridSleep_001)
{
    EXPECT_FALSE(m_loginManager->CanHybridSleep().isValid());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_CanPowerOff_001)
{
    EXPECT_FALSE(m_loginManager->CanPowerOff().isValid());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_CanReboot_001)
{
    EXPECT_FALSE(m_loginManager->CanReboot().isValid());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_CanSuspend_001)
{
    EXPECT_FALSE(m_loginManager->CanSuspend().isValid());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_FlushDevices_001)
{
    EXPECT_FALSE(m_loginManager->FlushDevices(false).isValid());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_GetSeat_001)
{
    EXPECT_FALSE(m_loginManager->GetSeat("").isValid());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_GetSession_001)
{
    EXPECT_FALSE(m_loginManager->GetSession("").isValid());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_GetSessionByPID_001)
{
    EXPECT_FALSE(m_loginManager->GetSessionByPID(0).isValid());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_GetUser_001)
{
    EXPECT_FALSE(m_loginManager->GetUser(0).isValid());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_Hibernate_001)
{
    EXPECT_FALSE(m_loginManager->Hibernate(false).isValid());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_HybridSleep_001)
{
    EXPECT_FALSE(m_loginManager->HybridSleep(false).isValid());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_Inhibit_001)
{
    EXPECT_FALSE(m_loginManager->Inhibit("", "", "", "").isValid());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_KillSession_001)
{
    EXPECT_FALSE(m_loginManager->KillSession("", "", "").isValid());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_KillUser_001)
{
    EXPECT_FALSE(m_loginManager->KillUser(0, "").isValid());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_ListInhibitors_001)
{
    EXPECT_FALSE(m_loginManager->ListInhibitors().isValid());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_ListSeats_001)
{
    EXPECT_FALSE(m_loginManager->ListSeats().isValid());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_ListSessions_001)
{
    EXPECT_FALSE(m_loginManager->ListSessions().isValid());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_ListUsers_001)
{
    EXPECT_FALSE(m_loginManager->ListUsers().isValid());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_LockSession_001)
{
    EXPECT_FALSE(m_loginManager->LockSession("").isValid());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_LockSessions_001)
{
    EXPECT_FALSE(m_loginManager->LockSessions().isValid());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_PowerOff_001)
{
    EXPECT_FALSE(m_loginManager->PowerOff(false).isValid());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_Reboot_001)
{
    EXPECT_FALSE(m_loginManager->Reboot(false).isValid());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_ReleaseSession_001)
{
    EXPECT_FALSE(m_loginManager->ReleaseSession("").isValid());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_SetUserLinger_001)
{
    EXPECT_FALSE(m_loginManager->SetUserLinger(0, false, false).isValid());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_Suspend_001)
{
    EXPECT_FALSE(m_loginManager->Suspend(false).isValid());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_TerminateSeat_001)
{
    EXPECT_FALSE(m_loginManager->TerminateSeat("").isValid());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_TerminateSession_001)
{
    EXPECT_FALSE(m_loginManager->TerminateSession("").isValid());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_TerminateUser_001)
{
    EXPECT_FALSE(m_loginManager->TerminateUser(0).isValid());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_UnlockSession_001)
{
    EXPECT_FALSE(m_loginManager->UnlockSession("").isValid());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager_UnlockSessions_001)
{
    EXPECT_FALSE(m_loginManager->UnlockSessions().isValid());
}

TEST_F(UT_DBusLogin1Manager, DBusLogin1Manager___propertyChanged___001)
{
    QDBusMessage message;
    QList<QVariant> param;

    param << "org.freedesktop.login1.Manager"
          << "test"
          << "PrepareForShutdown";

    message.setArguments(param);

    m_loginManager->__propertyChanged__(message);
}
