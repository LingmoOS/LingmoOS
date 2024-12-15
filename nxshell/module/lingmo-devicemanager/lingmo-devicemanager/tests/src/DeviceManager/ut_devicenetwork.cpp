// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DeviceNetwork.h"
#include "DeviceBios.h"
#include "DeviceInfo.h"
#include "DBusEnableInterface.h"

#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class UT_DeviceNetwork : public UT_HEAD
{
public:
    void SetUp()
    {
        m_deviceNetwork = new DeviceNetwork;
    }
    void TearDown()
    {
        delete m_deviceNetwork;
    }
    DeviceNetwork *m_deviceNetwork;
};

void ut_network_setlshwinfo(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("description", "description");
    mapinfo.insert("product", "product");
    mapinfo.insert("description", "description");
    mapinfo.insert("vendor", "vendor");
    mapinfo.insert("version", "version");
    mapinfo.insert("bus info", "bus info");
    mapinfo.insert("logical name", "enp2s0");
    mapinfo.insert("serial", "f4:b5:20:24:5e:4f");
    mapinfo.insert("irq", "irq");
    mapinfo.insert("memory", "memory");
    mapinfo.insert("width", "width");
    mapinfo.insert("clock", "clock");
    mapinfo.insert("capabilities", "capabilities");
    mapinfo.insert("autonegotiation", "autonegotiation");
    mapinfo.insert("broadcast", "broadcast");
    mapinfo.insert("driver", "driver");
    mapinfo.insert("driverversion", "driverversion");
    mapinfo.insert("duplex", "duplex");
    mapinfo.insert("firmware", "firmware");
    mapinfo.insert("port", "port");
    mapinfo.insert("link", "link");
    mapinfo.insert("ip", "ip");
    mapinfo.insert("size", "size");
    mapinfo.insert("capacity", "capacity");
    mapinfo.insert("latency", "latency");
    mapinfo.insert("multicast", "multicast");
}

void ut_network_sethwinfomap(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("Device File", "enp2s0");
    mapinfo.insert("Model", "Model");
    mapinfo.insert("path", "path");
}

TEST_F(UT_DeviceNetwork, DeviceNetwork_UT_setInfoFromHwinfo_001)
{
    QMap<QString, QString> mapinfo;
    ut_network_sethwinfomap(mapinfo);

    EXPECT_TRUE(m_deviceNetwork->setInfoFromHwinfo(mapinfo));
}

TEST_F(UT_DeviceNetwork, DeviceNetwork_UT_setInfoFromHwinfo_002)
{
    QMap<QString, QString> mapinfo;
    ut_network_sethwinfomap(mapinfo);

    EXPECT_TRUE(m_deviceNetwork->setInfoFromHwinfo(mapinfo));
}

TEST_F(UT_DeviceNetwork, DeviceNetwork_UT_setInfoFromLshw)
{
    QMap<QString, QString> mapinfo;
    ut_network_setlshwinfo(mapinfo);
    m_deviceNetwork->m_HwinfoToLshw = "f4:b5:20:24:5e:4f";

    m_deviceNetwork->setInfoFromLshw(mapinfo);
    EXPECT_STREQ("description", m_deviceNetwork->m_Model.toStdString().c_str());
    EXPECT_STREQ("product", m_deviceNetwork->m_Name.toStdString().c_str());
    EXPECT_STREQ("vendor", m_deviceNetwork->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("version", m_deviceNetwork->m_Version.toStdString().c_str());
    EXPECT_STREQ("bus info", m_deviceNetwork->m_BusInfo.toStdString().c_str());
    EXPECT_STREQ("enp2s0", m_deviceNetwork->m_LogicalName.toStdString().c_str());
    EXPECT_STREQ("f4:b5:20:24:5e:4f", m_deviceNetwork->m_MACAddress.toStdString().c_str());
    EXPECT_STREQ("irq", m_deviceNetwork->m_Irq.toStdString().c_str());
    EXPECT_STREQ("memory", m_deviceNetwork->m_Memory.toStdString().c_str());
    EXPECT_STREQ("width", m_deviceNetwork->m_Width.toStdString().c_str());
    EXPECT_STREQ("clock", m_deviceNetwork->m_Clock.toStdString().c_str());
    EXPECT_STREQ("capabilities", m_deviceNetwork->m_Capabilities.toStdString().c_str());
    EXPECT_STREQ("autonegotiation", m_deviceNetwork->m_Autonegotiation.toStdString().c_str());
    EXPECT_STREQ("broadcast", m_deviceNetwork->m_Broadcast.toStdString().c_str());
    EXPECT_STREQ("driver", m_deviceNetwork->m_Driver.toStdString().c_str());
    EXPECT_STREQ("driverversion", m_deviceNetwork->m_DriverVersion.toStdString().c_str());
    EXPECT_STREQ("duplex", m_deviceNetwork->m_Duplex.toStdString().c_str());
    EXPECT_STREQ("firmware", m_deviceNetwork->m_Firmware.toStdString().c_str());
    EXPECT_STREQ("link", m_deviceNetwork->m_Link.toStdString().c_str());
    EXPECT_STREQ("ip", m_deviceNetwork->m_Ip.toStdString().c_str());
    EXPECT_STREQ("size", m_deviceNetwork->m_Speed.toStdString().c_str());
    EXPECT_STREQ("capacity", m_deviceNetwork->m_Capacity.toStdString().c_str());
    EXPECT_STREQ("latency", m_deviceNetwork->m_Latency.toStdString().c_str());
    EXPECT_STREQ("multicast", m_deviceNetwork->m_Multicast.toStdString().c_str());
}

TEST_F(UT_DeviceNetwork, DeviceNetwork_UT_name)
{
    QMap<QString, QString> mapinfo;
    ut_network_setlshwinfo(mapinfo);
    m_deviceNetwork->m_HwinfoToLshw = "f4:b5:20:24:5e:4f";
    m_deviceNetwork->setInfoFromLshw(mapinfo);

    QString name = m_deviceNetwork->name();
    EXPECT_STREQ("product", name.toStdString().c_str());
}

TEST_F(UT_DeviceNetwork, DeviceNetwork_UT_driver)
{
    QMap<QString, QString> mapinfo;
    ut_network_setlshwinfo(mapinfo);
    m_deviceNetwork->m_HwinfoToLshw = "f4:b5:20:24:5e:4f";
    m_deviceNetwork->setInfoFromLshw(mapinfo);

    QString driver = m_deviceNetwork->driver();
    EXPECT_STREQ("driver", driver.toStdString().c_str());
}

TEST_F(UT_DeviceNetwork, DeviceNetwork_UT_subTitle)
{
    QMap<QString, QString> mapinfo;
    ut_network_setlshwinfo(mapinfo);
    m_deviceNetwork->m_HwinfoToLshw = "f4:b5:20:24:5e:4f";
    m_deviceNetwork->setInfoFromLshw(mapinfo);

    QString title = m_deviceNetwork->subTitle();
    EXPECT_STREQ("product", title.toStdString().c_str());
}

TEST_F(UT_DeviceNetwork, DeviceNetwork_UT_getOverviewInfo)
{
    QMap<QString, QString> mapinfo;
    ut_network_setlshwinfo(mapinfo);
    m_deviceNetwork->m_HwinfoToLshw = "f4:b5:20:24:5e:4f";
    m_deviceNetwork->setInfoFromLshw(mapinfo);

    QString overview = m_deviceNetwork->getOverviewInfo();
    EXPECT_STREQ("product", overview.toStdString().c_str());
}

bool ut_network_enable_true()
{
    return true;
}

TEST_F(UT_DeviceNetwork, DeviceNetwork_UT_setEnable_001)
{
    m_deviceNetwork->m_SysPath = "";

    EXPECT_EQ(EnableDeviceStatus::EDS_Faild, m_deviceNetwork->setEnable(true));
}

TEST_F(UT_DeviceNetwork, DeviceNetwork_UT_setEnable_002)
{
    m_deviceNetwork->m_SysPath = "usb";
    m_deviceNetwork->m_UniqueID = "";

    EXPECT_EQ(EnableDeviceStatus::EDS_Faild, m_deviceNetwork->setEnable(true));
}

TEST_F(UT_DeviceNetwork, DeviceNetwork_UT_setEnable_003)
{
    m_deviceNetwork->m_SysPath = "usb";
    m_deviceNetwork->m_UniqueID = "unique";

    Stub stub;
    stub.set(ADDR(DBusEnableInterface, enable), ut_network_enable_true);

    EXPECT_EQ(EnableDeviceStatus::EDS_Success, m_deviceNetwork->setEnable(true));
    EXPECT_TRUE(m_deviceNetwork->m_Enable);
}


TEST_F(UT_DeviceNetwork, DeviceNetwork_UT_enable)
{
    EXPECT_TRUE(m_deviceNetwork->enable());
}

TEST_F(UT_DeviceNetwork, DeviceNetwork_UT_correctCurrentLinkStatus)
{
    QMap<QString, QString> mapinfo;
    ut_network_setlshwinfo(mapinfo);
    mapinfo.insert("link", "link1");
    m_deviceNetwork->setInfoFromLshw(mapinfo);

    m_deviceNetwork->correctCurrentLinkStatus("link");
    EXPECT_STREQ("link", m_deviceNetwork->m_Link.toStdString().c_str());
}

TEST_F(UT_DeviceNetwork, DeviceNetwork_UT_logicalName)
{
    QMap<QString, QString> mapinfo;
    ut_network_setlshwinfo(mapinfo);
    m_deviceNetwork->m_HwinfoToLshw = "f4:b5:20:24:5e:4f";
    m_deviceNetwork->setInfoFromLshw(mapinfo);

    QString logicalName = m_deviceNetwork->logicalName();
    EXPECT_STREQ("enp2s0", logicalName.toStdString().c_str());
}

TEST_F(UT_DeviceNetwork, DeviceNetwork_UT_initFilterKey)
{
    m_deviceNetwork->initFilterKey();
    EXPECT_EQ(3, m_deviceNetwork->m_FilterKey.size());
}

TEST_F(UT_DeviceNetwork, DeviceNetwork_UT_loadBaseDeviceInfo)
{
    QMap<QString, QString> mapinfo;
    ut_network_setlshwinfo(mapinfo);
    m_deviceNetwork->m_HwinfoToLshw = "f4:b5:20:24:5e:4f";
    m_deviceNetwork->setInfoFromLshw(mapinfo);

    m_deviceNetwork->loadBaseDeviceInfo();
    EXPECT_EQ(8, m_deviceNetwork->m_LstBaseInfo.size());
}

TEST_F(UT_DeviceNetwork, DeviceNetwork_UT_loadOtherDeviceInfo)
{
    QMap<QString, QString> mapinfo;
    ut_network_setlshwinfo(mapinfo);
    m_deviceNetwork->m_HwinfoToLshw = "f4:b5:20:24:5e:4f";
    m_deviceNetwork->setInfoFromLshw(mapinfo);

    m_deviceNetwork->loadOtherDeviceInfo();
    EXPECT_EQ(17, m_deviceNetwork->m_LstOtherInfo.size());
}

TEST_F(UT_DeviceNetwork, DeviceNetwork_UT_loadTableData_001)
{
    m_deviceNetwork->loadTableData();
    EXPECT_EQ(3, m_deviceNetwork->m_TableData.size());
}

TEST_F(UT_DeviceNetwork, DeviceNetwork_UT_loadTableData_002)
{
    m_deviceNetwork->m_Available = false;
    m_deviceNetwork->m_Enable = false;

    m_deviceNetwork->loadTableData();
    EXPECT_EQ(3, m_deviceNetwork->m_TableData.size());
}
