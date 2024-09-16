// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sessionbasemodel.h"
#include "userinfo.h"

#include <QSignalSpy>

#include <gtest/gtest.h>

class UT_SessionBaseModel : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    SessionBaseModel *m_sessionBaseModel;
};

void UT_SessionBaseModel::SetUp()
{
    m_sessionBaseModel = new SessionBaseModel();
}

void UT_SessionBaseModel::TearDown()
{
    delete m_sessionBaseModel;
}

TEST_F(UT_SessionBaseModel, init)
{
    std::shared_ptr<NativeUser> nativeUser(new NativeUser("/org/deepin/dde/Accounts1/User"+QString::number((getuid()))));
    ASSERT_TRUE(m_sessionBaseModel);
    m_sessionBaseModel->updateCurrentUser(nativeUser);
    EXPECT_EQ(m_sessionBaseModel->currentUser(), nativeUser);

    SessionBaseModel::PowerAction poweraction = m_sessionBaseModel->powerAction();
    m_sessionBaseModel->onPowerActionChanged(SessionBaseModel::PowerAction::RequireNormal);
    EXPECT_EQ(m_sessionBaseModel->powerAction(), poweraction);

    SessionBaseModel::PowerAction poweractiontwo = m_sessionBaseModel->powerAction();
    m_sessionBaseModel->onPowerActionChanged(SessionBaseModel::PowerAction::RequireShutdown);
    EXPECT_EQ(m_sessionBaseModel->powerAction(), poweractiontwo);

    SessionBaseModel::PowerAction poweractionthe = m_sessionBaseModel->powerAction();
    m_sessionBaseModel->onPowerActionChanged(SessionBaseModel::PowerAction::RequireRestart);
    EXPECT_EQ(m_sessionBaseModel->powerAction(), poweractionthe);

    SessionBaseModel::PowerAction poweractionfor = m_sessionBaseModel->powerAction();
    m_sessionBaseModel->onPowerActionChanged(SessionBaseModel::PowerAction::RequireSuspend);
    EXPECT_EQ(m_sessionBaseModel->powerAction(), poweractionfor);

    SessionBaseModel::PowerAction poweractionfiv = m_sessionBaseModel->powerAction();
    m_sessionBaseModel->onPowerActionChanged(SessionBaseModel::PowerAction::RequireHibernate);
    EXPECT_EQ(m_sessionBaseModel->powerAction(), poweractionfiv);

    m_sessionBaseModel->setPowerAction(SessionBaseModel::PowerAction::None);

    SessionBaseModel::ModeStatus currentmode = m_sessionBaseModel->currentModeState();
    m_sessionBaseModel->onStatusChanged(SessionBaseModel::ModeStatus::PasswordMode);
    EXPECT_EQ(m_sessionBaseModel->currentModeState(), currentmode);

    SessionBaseModel::ModeStatus currentmodetwo = m_sessionBaseModel->currentModeState();
    m_sessionBaseModel->onStatusChanged(SessionBaseModel::ModeStatus::ConfirmPasswordMode);
    EXPECT_EQ(m_sessionBaseModel->currentModeState(), currentmodetwo);

    SessionBaseModel::ModeStatus currentmodethr = m_sessionBaseModel->currentModeState();
    m_sessionBaseModel->onStatusChanged(SessionBaseModel::ModeStatus::UserMode);
    EXPECT_EQ(m_sessionBaseModel->currentModeState(), currentmodethr);

    SessionBaseModel::ModeStatus currentmodefiv = m_sessionBaseModel->currentModeState();
    m_sessionBaseModel->onStatusChanged(SessionBaseModel::ModeStatus::PowerMode);
    EXPECT_EQ(m_sessionBaseModel->currentModeState(), currentmodefiv);

    SessionBaseModel::ModeStatus currentmodesix = m_sessionBaseModel->currentModeState();
    m_sessionBaseModel->onStatusChanged(SessionBaseModel::ModeStatus::ShutDownMode);
    EXPECT_EQ(m_sessionBaseModel->currentModeState(), currentmodesix);

    m_sessionBaseModel->setCurrentModeState(SessionBaseModel::ModeStatus::NoStatus);

    bool hasSwap = m_sessionBaseModel->hasSwap();
    m_sessionBaseModel->onHasSwapChanged(!hasSwap);
    m_sessionBaseModel->setHasSwap(!hasSwap);
    EXPECT_EQ(m_sessionBaseModel->hasSwap(), !hasSwap);

    bool isshow = m_sessionBaseModel->visible();
    m_sessionBaseModel->visibleChanged(!isshow);
    m_sessionBaseModel->setVisible(!isshow);
    EXPECT_EQ(m_sessionBaseModel->visible(), !isshow);

    bool sleep = m_sessionBaseModel->canSleep();
    m_sessionBaseModel->canSleepChanged(!sleep);
    m_sessionBaseModel->setCanSleep(!sleep);
    EXPECT_EQ(m_sessionBaseModel->canSleep(), !sleep);

    bool UserSwitchButton = m_sessionBaseModel->allowShowUserSwitchButton();
    m_sessionBaseModel->setAllowShowUserSwitchButton(!UserSwitchButton);
    m_sessionBaseModel->allowShowUserSwitchButtonChanged(!UserSwitchButton);
    EXPECT_EQ(m_sessionBaseModel->allowShowUserSwitchButton(), !UserSwitchButton);

    bool alwaysUserSwitchButton = m_sessionBaseModel->alwaysShowUserSwitchButton();
    m_sessionBaseModel->setAlwaysShowUserSwitchButton(!alwaysUserSwitchButton);
    EXPECT_EQ(m_sessionBaseModel->alwaysShowUserSwitchButton(), !alwaysUserSwitchButton);

    bool isServer = m_sessionBaseModel->isServerModel();
    m_sessionBaseModel->setIsServerModel(!isServer);
    EXPECT_EQ(m_sessionBaseModel->isServerModel(), !isServer);

    bool abortConfirm = m_sessionBaseModel->abortConfirm();
    m_sessionBaseModel->abortConfirmChanged(!abortConfirm);
    m_sessionBaseModel->setAbortConfirm(!abortConfirm);
    EXPECT_EQ(m_sessionBaseModel->abortConfirm(), !abortConfirm);

    bool isBlack = m_sessionBaseModel->isBlackMode();
    m_sessionBaseModel->setIsBlackMode(!isBlack);
    EXPECT_EQ(m_sessionBaseModel->isBlackMode(), !isBlack);

    bool isHibernate = m_sessionBaseModel->isHibernateMode();
    m_sessionBaseModel->setIsHibernateModel(!isHibernate);
    m_sessionBaseModel->HibernateModeChanged(!isHibernate);
    EXPECT_EQ(m_sessionBaseModel->isHibernateMode(), !isHibernate);

    int listSize = m_sessionBaseModel->userListSize();
    m_sessionBaseModel->onUserListSizeChanged(++listSize);
    m_sessionBaseModel->setUserListSize(++listSize);
    EXPECT_EQ(m_sessionBaseModel->userListSize(), listSize);

    m_sessionBaseModel->setSessionKey("AAAAAA");
    m_sessionBaseModel->setHasVirtualKB(true);
    m_sessionBaseModel->setHasVirtualKB(false);
}
