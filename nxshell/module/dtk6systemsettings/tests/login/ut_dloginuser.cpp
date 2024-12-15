// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "dloginuser.h"
#include "dloginuser_p.h"
#include "login1userinterface.h"
#include "login1userservice.h"

DLOGIN_USE_NAMESPACE

class TestDLoginUser : public testing::Test
{
public:
    TestDLoginUser()
        : m_fakeService(new Login1UserService)
        , m_dLoginUser(new DLoginUser("/org/freedesktop/login1/user"))
    {
        m_dLoginUser->d_ptr->m_inter->m_interface->setTimeout(INT_MAX);
    }

    virtual ~TestDLoginUser()
    {
        delete m_dLoginUser;
        delete m_fakeService;
    }

protected:
    Login1UserService *m_fakeService;
    DLoginUser *m_dLoginUser;
    static const QString Service;
};

const QString TestDLoginUser::Service = QStringLiteral("org.freedesktop.fakelogin1");

TEST_F(TestDLoginUser, propertySessions)
{
    m_fakeService->m_sessions = {
        { "session0", QDBusObjectPath("/org/freedesktop/login1/session0") },
        { "session1", QDBusObjectPath("/org/freedesktop/login1/session0") }
    };
    auto sessionList = m_dLoginUser->sessions();
    ASSERT_THAT(sessionList, testing::SizeIs(2));
    EXPECT_EQ("session0", sessionList[0]);
    EXPECT_EQ("session1", sessionList[1]);
}

TEST_F(TestDLoginUser, propertyIdleHint)
{
    m_fakeService->m_idleHint = false;
    ASSERT_FALSE(m_fakeService->idleHint());
    EXPECT_FALSE(m_dLoginUser->idleHint());
    m_fakeService->m_idleHint = true;
    ASSERT_TRUE(m_fakeService->idleHint());
    EXPECT_TRUE(m_dLoginUser->idleHint());
}

TEST_F(TestDLoginUser, propertyLinger)
{
    m_fakeService->m_linger = false;
    ASSERT_FALSE(m_fakeService->linger());
    EXPECT_FALSE(m_dLoginUser->linger());
    m_fakeService->m_linger = true;
    ASSERT_TRUE(m_fakeService->linger());
    EXPECT_TRUE(m_dLoginUser->linger());
}

TEST_F(TestDLoginUser, propertyName)
{
    m_fakeService->m_name = "test string";
    ASSERT_EQ("test string", m_fakeService->name());
    EXPECT_EQ("test string", m_dLoginUser->name());
}

TEST_F(TestDLoginUser, propertyRuntimePath)
{
    m_fakeService->m_runtimePath = "test string";
    ASSERT_EQ("test string", m_fakeService->runtimePath());
    EXPECT_EQ("test string", m_dLoginUser->runtimePath());
}

TEST_F(TestDLoginUser, propertyService)
{
    m_fakeService->m_applicationService = "test string";
    ASSERT_EQ("test string", m_fakeService->service());
    EXPECT_EQ("test string", m_dLoginUser->service());
}

TEST_F(TestDLoginUser, propertySlice)
{
    m_fakeService->m_slice = "test string";
    ASSERT_EQ("test string", m_fakeService->slice());
    EXPECT_EQ("test string", m_dLoginUser->slice());
}

struct UserStateParam
{
    QString strState;
    UserState enumState;
};

class TestUserState : public TestDLoginUser, public testing::WithParamInterface<UserStateParam>
{
public:
    TestUserState()
        : TestDLoginUser()
    {
    }

    ~TestUserState() override = default;
};

TEST_P(TestUserState, propertyState)
{
    auto params = GetParam();
    m_fakeService->m_state = params.strState;
    ASSERT_EQ(params.strState, m_fakeService->state());
    EXPECT_EQ(params.enumState, m_dLoginUser->state());
}

INSTANTIATE_TEST_SUITE_P(Default,
                        TestUserState,
                        testing::Values(UserStateParam{ "active", UserState::Active },
                                        UserStateParam{ "closing", UserState::Closing },
                                        UserStateParam{ "lingering", UserState::Lingering },
                                        UserStateParam{ "offline", UserState::Offline },
                                        UserStateParam{ "online", UserState::Online },
                                        UserStateParam{ "", UserState::Unknown }));

TEST_F(TestDLoginUser, propertyDisplay)
{
    m_fakeService->m_display.sessionId = "session0";
    m_fakeService->m_display.path = QDBusObjectPath("/org/freedesktop/login1/session0");
    ASSERT_EQ("session0", m_fakeService->display().sessionId);
    EXPECT_EQ("session0", m_dLoginUser->display());
}

TEST_F(TestDLoginUser, propertyGID)
{
    m_fakeService->m_GID = 1024;
    ASSERT_EQ(1024, m_fakeService->GID());
    EXPECT_EQ(1024, m_dLoginUser->GID());
    m_fakeService->m_GID = 2048;
    ASSERT_EQ(2048, m_fakeService->GID());
    EXPECT_EQ(2048, m_dLoginUser->GID());
}

TEST_F(TestDLoginUser, propertyUID)
{
    m_fakeService->m_UID = 1024;
    ASSERT_EQ(1024, m_fakeService->UID());
    EXPECT_EQ(1024, m_dLoginUser->UID());
    m_fakeService->m_UID = 2048;
    ASSERT_EQ(2048, m_fakeService->UID());
    EXPECT_EQ(2048, m_dLoginUser->UID());
}

TEST_F(TestDLoginUser, propertyIdleSinceHint)
{
    m_fakeService->m_idleSinceHint = 1000;
    ASSERT_EQ(1000, m_fakeService->idleSinceHint());
    EXPECT_EQ(1, m_dLoginUser->idleSinceHint().toMSecsSinceEpoch());
}

TEST_F(TestDLoginUser, propertyIdleSinceHintMonotonic)
{
    m_fakeService->m_idleSinceHintMonotonic = 1024;
    ASSERT_EQ(1024, m_fakeService->idleSinceHintMonotonic());
    EXPECT_EQ(1024, m_dLoginUser->idleSinceHintMonotonic());
    m_fakeService->m_idleSinceHintMonotonic = 2048;
    ASSERT_EQ(2048, m_fakeService->idleSinceHintMonotonic());
    EXPECT_EQ(2048, m_dLoginUser->idleSinceHintMonotonic());
}

TEST_F(TestDLoginUser, propertyLoginTime)
{
    m_fakeService->m_timestamp = 1000;
    ASSERT_EQ(1000, m_fakeService->timestamp());
    EXPECT_EQ(1, m_dLoginUser->loginTime().toMSecsSinceEpoch());
}

TEST_F(TestDLoginUser, propertyLoginTimeMonotonic)
{
    m_fakeService->m_timestampMonotonic = 1024;
    ASSERT_EQ(1024, m_fakeService->timestampMonotonic());
    EXPECT_EQ(1024, m_dLoginUser->loginTimeMonotonic());
    m_fakeService->m_timestampMonotonic = 2048;
    ASSERT_EQ(2048, m_fakeService->timestampMonotonic());
    EXPECT_EQ(2048, m_dLoginUser->loginTimeMonotonic());
}

TEST_F(TestDLoginUser, kill)
{
    m_fakeService->m_signalNumber = -1;
    m_dLoginUser->kill(8);
    EXPECT_EQ(8, m_fakeService->m_signalNumber);
}

TEST_F(TestDLoginUser, terminate)
{
    m_fakeService->m_terminated = false;
    m_dLoginUser->terminate();
    EXPECT_TRUE(m_fakeService->m_terminated);
}
