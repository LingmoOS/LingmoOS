// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "deepinauthframework.h"
#include "lockworker.h"

#include <gtest/gtest.h>

class UT_LockWorker : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    DeepinAuthFramework *m_authFramework;
    SessionBaseModel *m_model;
    LockWorker *m_worker;
};

void UT_LockWorker::SetUp()
{
    m_model = new SessionBaseModel();
    std::shared_ptr<User> user_ptr(new User);
    m_model->updateCurrentUser(user_ptr);

    m_worker = new LockWorker(m_model);
}

void UT_LockWorker::TearDown()
{
    delete m_worker;
    delete m_model;
}

TEST_F(UT_LockWorker, worker)
{
    m_worker->isLocked();
    m_worker->enableZoneDetected(false);
}

TEST_F(UT_LockWorker, auth)
{
    m_worker->createAuthentication("uos");
    m_worker->startAuthentication("uos", 0);
    m_worker->sendTokenToAuth("uos", 0, "123");
    m_worker->endAuthentication("uos", 0);
    m_worker->destroyAuthentication("uos");
    m_worker->switchToUser(m_model->currentUser());
    m_worker->setLocked(false);
    m_worker->restartResetSessionTimer();
    m_worker->handleServiceEvent(0, 0, "", "");
}

TEST_F(UT_LockWorker, connection)
{
//    m_worker->doPowerAction(SessionBaseModel::PowerAction::RequireSuspend);
//    m_worker->doPowerAction(SessionBaseModel::PowerAction::RequireHibernate);
//    m_worker->doPowerAction(SessionBaseModel::PowerAction::RequireRestart);
//    m_worker->doPowerAction(SessionBaseModel::PowerAction::RequireShutdown);
//    m_worker->doPowerAction(SessionBaseModel::PowerAction::RequireLock);
//    m_worker->doPowerAction(SessionBaseModel::PowerAction::RequireLogout);
//    m_worker->doPowerAction(SessionBaseModel::PowerAction::RequireSwitchSystem);
//    m_worker->doPowerAction(SessionBaseModel::PowerAction::RequireSwitchUser);
}
