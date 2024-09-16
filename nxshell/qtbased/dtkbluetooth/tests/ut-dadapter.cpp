// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "adapterservice.h"
#include <gtest/gtest.h>
#include "dbluetoothadapter.h"

DBLUETOOTH_USE_NAMESPACE

class TestDAdapter : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        m_ad = new DAdapter(0);
        m_fakeService = new FakeAdapterService();
    }

    static void TearDownTestCase()
    {
        delete m_ad;
        delete m_fakeService;
    }

    void SetUp() override {}
    void TearDown() override {}

private:
    static inline DAdapter *m_ad{nullptr};
    static inline FakeAdapterService *m_fakeService{nullptr};
};

TEST_F(TestDAdapter, address)
{
    EXPECT_EQ("FF:FF:FF:FF:FF:FF", m_ad->address());
}

TEST_F(TestDAdapter, addressType)
{
    EXPECT_EQ(DDevice::AddressType::Public, m_ad->addressType());
}

TEST_F(TestDAdapter, name)
{
    EXPECT_EQ("testbt", m_ad->name());
}

TEST_F(TestDAdapter, alias)
{
    EXPECT_EQ("alias-testbt", m_ad->alias());
    m_ad->setAlias("testbt");
    EXPECT_EQ("testbt", m_ad->alias());
}

TEST_F(TestDAdapter, powered)
{
    EXPECT_EQ(true, m_ad->powered());
    m_ad->setPowered(false);
    EXPECT_EQ(false, m_ad->powered());
}

TEST_F(TestDAdapter, discoverable)
{
    EXPECT_EQ(true, m_ad->discoverable());
    m_ad->setDiscoverable(false);
    EXPECT_EQ(false, m_ad->discoverable());
}

TEST_F(TestDAdapter, discoverableTimeout)
{
    EXPECT_EQ(300, m_ad->discoverableTimeout());
    m_ad->setDiscoverableTimeout(400);
    EXPECT_EQ(400, m_ad->discoverableTimeout());
}

TEST_F(TestDAdapter, removeDevice)
{
    EXPECT_EQ(false, m_fakeService->m_removeDevice);
    m_ad->removeDevice("");
    EXPECT_EQ(true, m_fakeService->m_removeDevice);
}

TEST_F(TestDAdapter, discover)
{
    m_ad->startDiscovery();
    EXPECT_EQ(true, m_ad->discovering());
    m_ad->stopDiscovery();
    EXPECT_EQ(false, m_ad->discovering());
}
