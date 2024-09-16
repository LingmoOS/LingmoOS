// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "dgenericdevice.h"
#include "fakeDBus/genericdeviceservice.h"
#include "fakeDBus/networkmanagerservice.h"
#include <QDebug>

DNETWORKMANAGER_USE_NAMESPACE

class TestDGenericDevice : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        m_nm = new FakeNetworkManagerService();
        m_fakeService = new FakeGenericDeviceService();
        m_gd = new DGenericDevice(2);
    }

    static void TearDownTestCase()
    {
        delete m_gd;
        delete m_fakeService;
        delete m_nm;
    }

    void SetUp() override {}
    void TearDown() override {}

    static inline DGenericDevice *m_gd{nullptr};
    static inline FakeGenericDeviceService *m_fakeService{nullptr};
    static inline FakeNetworkManagerService *m_nm{nullptr};
};

TEST_F(TestDGenericDevice, typeDescription)
{
    EXPECT_EQ("generic", m_gd->typeDescription());
}

TEST_F(TestDGenericDevice, HwAddress)
{
    EXPECT_EQ("127.0.0.1", m_gd->HwAddress());
}
