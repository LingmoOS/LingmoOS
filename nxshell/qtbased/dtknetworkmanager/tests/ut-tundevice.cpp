// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "dtundevice.h"
#include "fakeDBus/tundeviceservice.h"
#include "fakeDBus/networkmanagerservice.h"
#include <QDebug>

DNETWORKMANAGER_USE_NAMESPACE

class TestDTunDevice : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        m_nm = new FakeNetworkManagerService();
        m_fakeService = new FakeTunDeviceService();
        m_td = new DTunDevice(2);
    }

    static void TearDownTestCase()
    {
        delete m_td;
        delete m_fakeService;
        delete m_nm;
    }

    void SetUp() override {}
    void TearDown() override {}

    static inline DTunDevice *m_td{nullptr};
    static inline FakeTunDeviceService *m_fakeService{nullptr};
    static inline FakeNetworkManagerService *m_nm{nullptr};
};

TEST_F(TestDTunDevice, HwAddress)
{
    EXPECT_EQ("127.0.0.1", m_td->HwAddress());
}
