// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "dwireddevice.h"
#include "fakeDBus/wireddeviceservice.h"
#include "fakeDBus/networkmanagerservice.h"
#include <QDebug>

DNETWORKMANAGER_USE_NAMESPACE

class TestDWiredDevice : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        m_nm = new FakeNetworkManagerService();
        m_fakeService = new FakeWiredDeviceService();
        m_wd = new DWiredDevice(2);
    }

    static void TearDownTestCase()
    {
        delete m_wd;
        delete m_fakeService;
        delete m_nm;
    }

    void SetUp() override {}
    void TearDown() override {}

    static inline DWiredDevice *m_wd{nullptr};
    static inline FakeWiredDeviceService *m_fakeService{nullptr};
    static inline FakeNetworkManagerService *m_nm{nullptr};
};

TEST_F(TestDWiredDevice, HwAddress)
{
    EXPECT_EQ("127.0.0.1", m_wd->HwAddress());
}

TEST_F(TestDWiredDevice, permHwAddress)
{
    EXPECT_EQ("127.0.0.1", m_wd->permHwAddress());
}

TEST_F(TestDWiredDevice, speed)
{
    EXPECT_EQ(200, m_wd->speed());
}

TEST_F(TestDWiredDevice, S390Subchannels)
{
    EXPECT_EQ("subchannel", m_wd->S390Subchannels()[0]);
}

TEST_F(TestDWiredDevice, carrier)
{
    EXPECT_EQ(true, m_wd->carrier());
}
