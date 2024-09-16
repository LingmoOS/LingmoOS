// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "deviceservice.h"
#include <gtest/gtest.h>
#include "dbluetoothdevice.h"
#include <QBluetoothAddress>

DBLUETOOTH_USE_NAMESPACE

class TestDevice : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        m_de = new DDevice("/org/bluez/hci0", "FF:FF:FF:FF:FF:FF");
        m_fakeService = new FakeDeviceService();
    }

    static void TearDownTestCase()
    {
        delete m_de;
        delete m_fakeService;
    }

    void SetUp() override {}
    void TearDown() override {}

private:
    static inline DDevice *m_de{nullptr};
    static inline FakeDeviceService *m_fakeService{nullptr};
};

TEST_F(TestDevice, blocked)
{
    EXPECT_EQ(false, m_de->blocked());
    m_de->setBlocked(true);
    EXPECT_EQ(true, m_de->blocked());
}

TEST_F(TestDevice, connected)
{
    EXPECT_EQ(false, m_de->connected());
}

TEST_F(TestDevice, legacyPairing)
{
    EXPECT_EQ(false, m_de->legacyPairing());
}

TEST_F(TestDevice, paired)
{
    EXPECT_EQ(false, m_de->paired());
}

TEST_F(TestDevice, servicesResolved)
{
    EXPECT_EQ(false, m_de->servicesResolved());
}

TEST_F(TestDevice, trusted)
{
    EXPECT_EQ(false, m_de->trusted());
    m_de->setTrusted(true);
    EXPECT_EQ(true, m_de->trusted());
}

TEST_F(TestDevice, adapter)
{
    EXPECT_EQ(0, m_de->adapter());
}

TEST_F(TestDevice, addressType)
{
    EXPECT_EQ(DDevice::AddressType::Public, m_de->addressType());
}

TEST_F(TestDevice, alias)
{
    EXPECT_EQ("testbt", m_de->alias());
}

TEST_F(TestDevice, icon)
{
    EXPECT_EQ("computer", m_de->icon());
}

TEST_F(TestDevice, appearance)
{
    EXPECT_EQ(0x40, m_de->appearance());
}

TEST_F(TestDevice, deviceInfo)
{
    const auto &info = m_de->deviceInfo();
    EXPECT_EQ(true, info.isValid());
    EXPECT_EQ(QBluetoothAddress("FF:FF:FF:FF:FF:FF"), info.address());
    EXPECT_EQ("testbt", info.name());
    EXPECT_TRUE(info.serviceClasses().testFlag(QBluetoothDeviceInfo::ServiceClass::AudioService));
    EXPECT_TRUE(info.serviceClasses().testFlag(QBluetoothDeviceInfo::ServiceClass::TelephonyService));
    EXPECT_TRUE(info.serviceClasses().testFlag(QBluetoothDeviceInfo::ServiceClass::InformationService));
    EXPECT_EQ(QBluetoothDeviceInfo::MajorDeviceClass::ComputerDevice, info.majorDeviceClass());
    EXPECT_EQ(QBluetoothDeviceInfo::MinorComputerClass::LaptopComputer, info.minorDeviceClass());
    EXPECT_EQ(10, info.rssi());
    EXPECT_EQ(QBluetoothUuid{QString("00001104-0000-1000-8000-00805f9b34fb")}, info.serviceUuids()[0]);
}

TEST_F(TestDevice, disconnect)
{
    EXPECT_EQ(false, m_fakeService->m_disconnect);
    m_de->disconnectDevice();
    EXPECT_EQ(true, m_fakeService->m_disconnect);
}

TEST_F(TestDevice, connect)
{
    EXPECT_EQ(false, m_fakeService->m_connect);
    m_de->connectDevice();
    EXPECT_EQ(true, m_fakeService->m_connect);
}

TEST_F(TestDevice, cancelPairing)
{
    EXPECT_EQ(false, m_fakeService->m_cancelPairing);
    m_de->cancelPairing();
    EXPECT_EQ(true, m_fakeService->m_cancelPairing);
}

TEST_F(TestDevice, pair)
{
    EXPECT_EQ(false, m_fakeService->m_pair);
    m_de->pair();
    EXPECT_EQ(true, m_fakeService->m_pair);
}
