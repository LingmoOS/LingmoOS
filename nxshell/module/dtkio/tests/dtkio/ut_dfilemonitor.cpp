// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include <DFileMonitor>

#include <gtest/gtest.h>
#include <gio/gio.h>
#include <stubext.h>

#include "dfilemonitor_p.h"

DIO_USE_NAMESPACE
DCORE_USE_NAMESPACE

class TestDFileMonitor : public testing::Test
{
public:
    void SetUp() override
    {
        m_monitor = new DFileMonitor(QUrl("test"));
    }
    void TearDown() override
    {
        delete m_monitor;
        m_stub.clear();
    }

    stub_ext::StubExt m_stub;
    DFileMonitor *m_monitor { nullptr };
};

TEST_F(TestDFileMonitor, private_createMonitor)
{
    // test success

    GFile *file = g_file_new_for_uri("test");
    GFileMonitor *gmonitor = m_monitor->d->createMonitor(file, WatchType::Auto);
    EXPECT_TRUE(gmonitor != nullptr);

    // test failed

    file = nullptr;
    gmonitor = m_monitor->d->createMonitor(file, WatchType::Auto);
    EXPECT_TRUE(gmonitor == nullptr);
}

TEST_F(TestDFileMonitor, start)
{
    // test success
    {
        auto retSuccess = m_monitor->start(0);
        EXPECT_TRUE(retSuccess.hasValue());
        EXPECT_TRUE(retSuccess.value());
    }

    // test failed
    {
        m_stub.set_lamda(ADDR(DFileMonitorPrivate, createMonitor),
                         [](DFileMonitorPrivate *, GFile *gfile, WatchType type) {
                             Q_UNUSED(gfile);
                             Q_UNUSED(type);
                             __DBG_STUB_INVOKE__
                             return nullptr;
                         });
        auto retFailed = m_monitor->start(0);
        EXPECT_TRUE(retFailed.hasValue());
        EXPECT_FALSE(retFailed.value());
    }
}

TEST_F(TestDFileMonitor, stop)
{
    // test success
    {
        GFileMonitor gmonitor;
        m_monitor->d->gmonitor = &gmonitor;
        m_stub.set_lamda(g_file_monitor_cancel, [](GFileMonitor *monitor) {
            Q_UNUSED(monitor);
            __DBG_STUB_INVOKE__
            return true;
        });
        auto retSuccess = m_monitor->stop();
        EXPECT_TRUE(retSuccess);
    }

    // test failed
    {
        m_stub.set_lamda(g_file_monitor_cancel, [](GFileMonitor *monitor) {
            Q_UNUSED(monitor);
            __DBG_STUB_INVOKE__
            return false;
        });
        auto retFailed = m_monitor->stop();
        EXPECT_FALSE(retFailed);
    }
}

TEST_F(TestDFileMonitor, url)
{
    m_monitor->d->url = QUrl("file:///");
    auto ret = m_monitor->url();
    EXPECT_TRUE(ret.hasValue());
    EXPECT_EQ(ret.value(), QUrl("file:///"));
}

TEST_F(TestDFileMonitor, timeRate)
{
    m_monitor->d->timeRate = 0;
    auto ret = m_monitor->timeRate();
    EXPECT_TRUE(ret.hasValue());
    EXPECT_EQ(ret.value(), 0);
}

TEST_F(TestDFileMonitor, watchType)
{
    m_monitor->d->watchType = WatchType::Dir;
    auto ret = m_monitor->watchType();
    EXPECT_TRUE(ret.hasValue());
    EXPECT_EQ(ret.value(), WatchType::Dir);
}

TEST_F(TestDFileMonitor, running)
{
    GFileMonitor gmonitor;
    m_monitor->d->gmonitor = &gmonitor;
    auto ret = m_monitor->running();
    EXPECT_TRUE(ret.hasValue());
    EXPECT_TRUE(ret.value());
}
