// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include <gtest/gtest.h>

#include "dsystemtime.h"
#include "fakedbus/timedate1service.h"
#include "fakedbus/timesync1service.h"

#include <qdatetime.h>
#include <qdebug.h>

#include <QDBusConnection>
DSYSTEMTIME_USE_NAMESPACE

class TestDSystemTime : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        m_date_fakeInterface = new TimeDate1Service();
        m_dsystemtime = new DSystemTime();

        m_sync_fakeInterface = new TimeSync1Service();
    }

    static void TearDownTestCase()
    {
        delete m_date_fakeInterface;
        delete m_dsystemtime;
        delete m_sync_fakeInterface;
        m_date_fakeInterface = nullptr;
        m_dsystemtime = nullptr;
        m_sync_fakeInterface = nullptr;
    }

    void SetUp() override { }

    void TearDown() override { }

    static TimeDate1Service *m_date_fakeInterface;
    static DSystemTime *m_dsystemtime;

    static TimeSync1Service *m_sync_fakeInterface;
};

TimeDate1Service *TestDSystemTime::m_date_fakeInterface = nullptr;
TimeSync1Service *TestDSystemTime::m_sync_fakeInterface = nullptr;
DSystemTime *TestDSystemTime::m_dsystemtime = nullptr;

QT_BEGIN_NAMESPACE // for QString support
        inline void
        PrintTo(const QString &qString, ::std::ostream *os)
{
    *os << qUtf8Printable(qString);
}

QT_END_NAMESPACE

TEST_F(TestDSystemTime, fallbackNTPServers)
{
    ASSERT_EQ("NTP1", m_sync_fakeInterface->fallbackNTPServers()[0]);
    EXPECT_EQ("NTP2", m_dsystemtime->fallbackNTPServers()[1]);
}

TEST_F(TestDSystemTime, linkNTPServers)
{
    ASSERT_EQ("NTP1Server", m_sync_fakeInterface->linkNTPServers()[0]);
    EXPECT_EQ("NTP2Server", m_dsystemtime->linkNTPServers()[1]);
}

TEST_F(TestDSystemTime, systemNTPServers)
{
    ASSERT_EQ("NTP1Server", m_sync_fakeInterface->systemNTPServers()[0]);
    EXPECT_EQ("NTP2Server", m_dsystemtime->systemNTPServers()[1]);
}

TEST_F(TestDSystemTime, frequency)
{
    ASSERT_EQ(60, m_sync_fakeInterface->frequency());
    EXPECT_EQ(60, m_dsystemtime->frequency());
}

TEST_F(TestDSystemTime, serverName)
{
    ASSERT_EQ("NTP1", m_sync_fakeInterface->serverName());
    EXPECT_EQ("NTP1", m_dsystemtime->serverName());
}

TEST_F(TestDSystemTime, serverAddress)
{
    ASSERT_EQ("127.0.0.1", m_sync_fakeInterface->serverAddress().ip);
    EXPECT_EQ(1, m_dsystemtime->serverAddress().type);
}

TEST_F(TestDSystemTime, NTPMessage)
{
    ASSERT_EQ(1, m_sync_fakeInterface->NTPMessage().ntp_field_leap);
    EXPECT_TRUE(m_dsystemtime->NTPMessage().spike);
}

TEST_F(TestDSystemTime, pollIntervalMaxUSec)
{
    ASSERT_EQ(2048, m_sync_fakeInterface->pollIntervalMaxUSec());
    EXPECT_EQ(2048, m_dsystemtime->pollIntervalMaxUSec());
}

TEST_F(TestDSystemTime, pollIntervalMinUSec)
{
    ASSERT_EQ(60, m_sync_fakeInterface->pollIntervalMinUSec());
    EXPECT_EQ(60, m_dsystemtime->pollIntervalMinUSec());
}

TEST_F(TestDSystemTime, pollIntervalUSec)
{
    ASSERT_EQ(240, m_sync_fakeInterface->pollIntervalUSec());
    EXPECT_EQ(240, m_dsystemtime->pollIntervalUSec());
}

TEST_F(TestDSystemTime, rootDistanceMaxUSec)
{
    ASSERT_EQ(5, m_sync_fakeInterface->rootDistanceMaxUSec());
    EXPECT_EQ(5, m_dsystemtime->rootDistanceMaxUSec());
}

TEST_F(TestDSystemTime, canNTP) // return true
{
    ASSERT_TRUE(m_date_fakeInterface->canNTP());
    EXPECT_TRUE(m_dsystemtime->canNTP());
}

TEST_F(TestDSystemTime, localRTC) // return true
{
    ASSERT_TRUE(m_date_fakeInterface->localRTC());
    EXPECT_TRUE(m_dsystemtime->localRTC());
}

TEST_F(TestDSystemTime, NTP) // return true
{
    ASSERT_TRUE(m_date_fakeInterface->NTP());
    EXPECT_TRUE(m_dsystemtime->NTP());
}

TEST_F(TestDSystemTime, NTPSynchronized) // return true
{
    ASSERT_TRUE(m_date_fakeInterface->NTPSynchronized());
    EXPECT_TRUE(m_dsystemtime->NTPSynchronized());
}

TEST_F(TestDSystemTime, timezone) // return Asia/Tokyo
{
    ASSERT_EQ("Asia/Tokyo", m_date_fakeInterface->timezone());
    EXPECT_EQ("Asia/Tokyo", m_dsystemtime->timezone());
}

TEST_F(TestDSystemTime, RTCTimeUSec) // return 42
{
    ASSERT_EQ(42, m_date_fakeInterface->RTCTimeUSec());
    EXPECT_EQ(42, m_dsystemtime->RTCTimeUSec());
}

TEST_F(TestDSystemTime, timeDate) // return 1666171692
{
    ASSERT_EQ(1666171692, m_date_fakeInterface->timeUSec());
    EXPECT_EQ(qint64(1666171692 / 1000), m_dsystemtime->timeDate().toMSecsSinceEpoch());
}

TEST_F(TestDSystemTime,
       ListTimezones) // return {"Asia/Tokyo", "Asia/Shanghai", "Asia/Hongkong", "Asia/Korea"}
{
    ASSERT_EQ("Asia/Hongkong", m_date_fakeInterface->ListTimezones()[2]);
    auto eTimezones = m_dsystemtime->listTimezones();
    ASSERT_TRUE(eTimezones.hasValue());
    EXPECT_EQ("Asia/Korea", eTimezones.value()[3]);
}

TEST_F(TestDSystemTime, SetLocalRTC) // focus local_rtc
{
    m_date_fakeInterface->SetLocalRTC(true, false, false);
    ASSERT_TRUE(m_date_fakeInterface->setLocalRTC_sig);
    m_dsystemtime->setLocalRTC(false, false, false);
    EXPECT_FALSE(m_date_fakeInterface->setLocalRTC_sig);
}

TEST_F(TestDSystemTime, SetLocalTime) // focus use_NTP
{
    m_date_fakeInterface->SetNTP(true, false);
    ASSERT_TRUE(m_date_fakeInterface->setLocalTime_sig);
    m_dsystemtime->enableNTP(false, false);
    EXPECT_FALSE(m_date_fakeInterface->setLocalTime_sig);
}

TEST_F(TestDSystemTime, SetAbsoluteTime) // focus usec_utc
{
    m_date_fakeInterface->SetTime(42, false, false);
    ASSERT_EQ(42, m_date_fakeInterface->setTime_sig);
    m_dsystemtime->setAbsoluteTime(QDateTime::fromMSecsSinceEpoch(1666171692), false);
    EXPECT_EQ(1666171692, m_date_fakeInterface->setTime_sig / 1000);
}

TEST_F(TestDSystemTime, SetTimezone) // focus timezone
{
    m_date_fakeInterface->SetTimezone("Asia/Shanghai", true);
    ASSERT_EQ("Asia/Shanghai", m_date_fakeInterface->setTimezone_sig);
    m_dsystemtime->setTimezone("Asia/Tokyo", true);
    EXPECT_EQ("Asia/Tokyo", m_date_fakeInterface->setTimezone_sig);
}
