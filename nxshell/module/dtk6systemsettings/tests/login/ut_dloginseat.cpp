// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "dloginseat.h"
#include "dloginseat_p.h"
#include "login1seatinterface.h"
#include "login1seatservice.h"

DLOGIN_USE_NAMESPACE

class TestDLoginSeat : public testing::Test
{
public:
    TestDLoginSeat()
        : m_fakeService(new Login1SeatService)
        , m_dLoginSeat(new DLoginSeat("/org/freedesktop/login1/seat"))
    {
        m_dLoginSeat->d_ptr->m_inter->m_interface->setTimeout(INT_MAX);
    }

    virtual ~TestDLoginSeat()
    {
        delete m_dLoginSeat;
        delete m_fakeService;
    }

protected:
    Login1SeatService *m_fakeService;
    DLoginSeat *m_dLoginSeat;
    static const QString Service;
};

const QString TestDLoginSeat::Service = QStringLiteral("org.freedesktop.fakelogin1");

TEST_F(TestDLoginSeat, propertySessions)
{
    m_fakeService->m_sessions = {
        { "session0", QDBusObjectPath("/org/freedesktop/login1/session0") },
        { "session1", QDBusObjectPath("/org/freedesktop/login1/session0") }
    };
    auto sessionList = m_dLoginSeat->sessions();
    ASSERT_THAT(sessionList, testing::SizeIs(2));
    EXPECT_EQ("session0", sessionList[0]);
    EXPECT_EQ("session1", sessionList[1]);
}

TEST_F(TestDLoginSeat, propertyCanGraphical)
{
    m_fakeService->m_canGraphical = false;
    ASSERT_FALSE(m_fakeService->canGraphical());
    EXPECT_FALSE(m_dLoginSeat->canGraphical());
    m_fakeService->m_canGraphical = true;
    ASSERT_TRUE(m_fakeService->canGraphical());
    EXPECT_TRUE(m_dLoginSeat->canGraphical());
}

TEST_F(TestDLoginSeat, propertyCanTTY)
{
    m_fakeService->m_canTTY = false;
    ASSERT_FALSE(m_fakeService->canTTY());
    EXPECT_FALSE(m_dLoginSeat->canTTY());
    m_fakeService->m_canTTY = true;
    ASSERT_TRUE(m_fakeService->canTTY());
    EXPECT_TRUE(m_dLoginSeat->canTTY());
}

TEST_F(TestDLoginSeat, propertyidleHint)
{
    m_fakeService->m_idleHint = false;
    ASSERT_FALSE(m_fakeService->idleHint());
    EXPECT_FALSE(m_dLoginSeat->idleHint());
    m_fakeService->m_idleHint = true;
    ASSERT_TRUE(m_fakeService->idleHint());
    EXPECT_TRUE(m_dLoginSeat->idleHint());
}

TEST_F(TestDLoginSeat, propertyId)
{
    m_fakeService->m_id = "seat0";
    ASSERT_EQ("seat0", m_fakeService->id());
    EXPECT_EQ("seat0", m_dLoginSeat->id());
}

TEST_F(TestDLoginSeat, propertyActiveSession)
{
    m_fakeService->m_activeSession.sessionId = "session0";
    m_fakeService->m_activeSession.path = QDBusObjectPath("/org/freedesktop/login1/session");
    ASSERT_EQ("session0", m_fakeService->activeSession().sessionId);
    EXPECT_EQ("session0", m_dLoginSeat->activeSession());
}

TEST_F(TestDLoginSeat, propertyIdleSinceHint)
{
    m_fakeService->m_idleSinceHint = 1000;
    ASSERT_EQ(1000, m_fakeService->idleSinceHint());
    EXPECT_EQ(1, m_dLoginSeat->idleSinceHint().toMSecsSinceEpoch());
    m_fakeService->m_idleSinceHint = 3000;
    ASSERT_EQ(3000, m_fakeService->idleSinceHint());
    EXPECT_EQ(3, m_dLoginSeat->idleSinceHint().toMSecsSinceEpoch());
}

TEST_F(TestDLoginSeat, propertyIdleSinceHintMonotonic)
{
    m_fakeService->m_idleSinceHintMonotonic = 1500;
    ASSERT_EQ(1500, m_fakeService->idleSinceHintMonotonic());
    EXPECT_EQ(1500, m_dLoginSeat->idleSinceHintMonotonic());
    m_fakeService->m_idleSinceHintMonotonic = 3000;
    ASSERT_EQ(3000, m_fakeService->idleSinceHintMonotonic());
    EXPECT_EQ(3000, m_dLoginSeat->idleSinceHintMonotonic());
}

TEST_F(TestDLoginSeat, activateSession)
{
    m_fakeService->m_sessionId = "";
    m_dLoginSeat->activateSession("session1");
    EXPECT_EQ("session1", m_fakeService->m_sessionId);
}

TEST_F(TestDLoginSeat, switchTo)
{
    m_fakeService->m_VTNr = 0;
    m_dLoginSeat->switchTo(32);
    EXPECT_EQ(32, m_fakeService->m_VTNr);
}
