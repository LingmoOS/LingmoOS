// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtkmount_global.h"
#include "ddiskjob.h"
#include "udisks2_interface.h"

#include <gtest/gtest.h>
#include <stubext.h>

DMOUNT_USE_NAMESPACE

class TestDDiskJob : public testing::Test
{
public:
    void SetUp() override { }
    void TearDown() override { m_stub.clear(); }

    stub_ext::StubExt m_stub;
    DDiskJob m_job { "test" };
};

TEST_F(TestDDiskJob, path)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2JobInterface::path, [] { __DBG_STUB_INVOKE__ return "test"; });
    EXPECT_TRUE(m_job.path() == "test");
}

TEST_F(TestDDiskJob, objects)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2JobInterface::objects, [] {
        __DBG_STUB_INVOKE__
        return QList<QDBusObjectPath> { QDBusObjectPath("/com/test/unit") };
    });
    EXPECT_TRUE(m_job.objects().count() == 1);
    EXPECT_TRUE(m_job.objects().first() == "/com/test/unit");
}

TEST_F(TestDDiskJob, cancelable)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2JobInterface::cancelable, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_TRUE(m_job.cancelable());
}

TEST_F(TestDDiskJob, progressValid)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2JobInterface::progressValid, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_TRUE(m_job.progressValid());
}

TEST_F(TestDDiskJob, progress)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2JobInterface::progress, [] { __DBG_STUB_INVOKE__ return 100.0; });
    EXPECT_TRUE(m_job.progress() == 100);
}

TEST_F(TestDDiskJob, operation)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2JobInterface::operation, [] { __DBG_STUB_INVOKE__ return "test"; });
    EXPECT_EQ("test", m_job.operation());
}

TEST_F(TestDDiskJob, startedByUid)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2JobInterface::startedByUID, [] { __DBG_STUB_INVOKE__ return 0; });
    EXPECT_EQ(0, m_job.startedByUid());
}

TEST_F(TestDDiskJob, bytes)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2JobInterface::bytes, [] { __DBG_STUB_INVOKE__ return 1024; });
    EXPECT_EQ(1024, m_job.bytes());
}

TEST_F(TestDDiskJob, expectedEndTime)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2JobInterface::expectedEndTime, [] { __DBG_STUB_INVOKE__ return 0; });
    EXPECT_EQ(0, m_job.expectedEndTime());
}

TEST_F(TestDDiskJob, rate)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2JobInterface::rate, [] { __DBG_STUB_INVOKE__ return 0; });
    EXPECT_EQ(0, m_job.rate());
}

TEST_F(TestDDiskJob, startTime)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2JobInterface::startTime, [] { __DBG_STUB_INVOKE__ return 0; });
    EXPECT_EQ(0, m_job.startTime());
}

TEST_F(TestDDiskJob, cancel)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2JobInterface::Cancel, [] { __DBG_STUB_INVOKE__ return QDBusPendingReply<>(); });
    m_stub.set_lamda(&QDBusPendingCall::waitForFinished, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(m_job.cancel({}));
}
