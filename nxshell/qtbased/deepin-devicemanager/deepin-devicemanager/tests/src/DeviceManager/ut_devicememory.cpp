// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DeviceMemory.h"
#include "DeviceBios.h"

#include "stub.h"
#include "ut_Head.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class UT_DeviceMemory : public UT_HEAD
{
public:
    void SetUp()
    {
        m_deviceMemory = new DeviceMemory;
    }
    void TearDown()
    {
        delete m_deviceMemory;
    }
    DeviceMemory *m_deviceMemory;
};

void ut_memory_setlshwmap(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("product", "CT8G4DFS8266.M8FD");
    mapinfo.insert("description", "DIMM DDR4 Synchronous Unbuffered (Unregistered) 2667 MHz (0.4 ns)");
    mapinfo.insert("vendor", "859B");
    mapinfo.insert("slot", "ChannelA-DIMM0");
    mapinfo.insert("size", "8GiB");
    mapinfo.insert("clock", "2667MT/s");
    mapinfo.insert("width", "64 bits");
    mapinfo.insert("serial", "25A1185D");
}

TEST_F(UT_DeviceMemory, UT_DeviceMemory_setInfoFromLshw_001)
{
    QMap<QString, QString> mapinfo;
    ut_memory_setlshwmap(mapinfo);

    m_deviceMemory->setInfoFromLshw(mapinfo);
    EXPECT_STREQ("CT8G4DFS8266.M8FD", m_deviceMemory->m_Name.toStdString().c_str());
    EXPECT_STREQ("859B", m_deviceMemory->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("ChannelA-DIMM0", m_deviceMemory->m_Locator.toStdString().c_str());
    EXPECT_STREQ("8GB", m_deviceMemory->m_Size.toStdString().c_str());
    EXPECT_STREQ("2667MHz", m_deviceMemory->m_Speed.toStdString().c_str());
    EXPECT_STREQ("64 bits", m_deviceMemory->m_TotalBandwidth.toStdString().c_str());
    EXPECT_STREQ("64 bits", m_deviceMemory->m_DataBandwidth.toStdString().c_str());
    EXPECT_STREQ("25A1185D", m_deviceMemory->m_SerialNumber.toStdString().c_str());
}

TEST_F(UT_DeviceMemory, UT_DeviceMemory_setInfoFromLshw_002)
{
    QMap<QString, QString> mapinfo;
    ut_memory_setlshwmap(mapinfo);
    mapinfo.insert("size", "1024MiB");

    m_deviceMemory->setInfoFromLshw(mapinfo);
    EXPECT_STREQ("CT8G4DFS8266.M8FD", m_deviceMemory->m_Name.toStdString().c_str());
    EXPECT_STREQ("859B", m_deviceMemory->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("ChannelA-DIMM0", m_deviceMemory->m_Locator.toStdString().c_str());
    EXPECT_STREQ("1GB", m_deviceMemory->m_Size.toStdString().c_str());
    EXPECT_STREQ("2667MHz", m_deviceMemory->m_Speed.toStdString().c_str());
    EXPECT_STREQ("64 bits", m_deviceMemory->m_TotalBandwidth.toStdString().c_str());
    EXPECT_STREQ("64 bits", m_deviceMemory->m_DataBandwidth.toStdString().c_str());
    EXPECT_STREQ("25A1185D", m_deviceMemory->m_SerialNumber.toStdString().c_str());
}

void ut_memory_setdmimap(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("Locator", "ChannelA-DIMM0");
    mapinfo.insert("Part Number", "CT8G4DFS8266.M8FD");
    mapinfo.insert("Serial Number", "25A1185D");
    mapinfo.insert("Configured Memory Speed", "2400 MT/s");
    mapinfo.insert("Minimum Voltage", "1.2 V");
    mapinfo.insert("Maximum Voltage", "1.2 V");
    mapinfo.insert("Configured Voltage", "1.2 V");
    mapinfo.insert("Total Width", "64 bits");
    mapinfo.insert("Data Width", "64 bits");
    mapinfo.insert("Type", "DDR4");
}

TEST_F(UT_DeviceMemory, UT_DeviceMemory_setInfoFromDmidecode_001)
{
    QMap<QString, QString> mapinfo;
    m_deviceMemory->m_MatchedFromDmi = true;

    EXPECT_FALSE(m_deviceMemory->setInfoFromDmidecode(mapinfo));
}

TEST_F(UT_DeviceMemory, UT_DeviceMemory_setInfoFromDmidecode_002)
{
    QMap<QString, QString> mapinfo;
    ut_memory_setdmimap(mapinfo);
    m_deviceMemory->m_MatchedFromDmi = false;
    m_deviceMemory->m_Locator = "ChannelA-DIMM0";

    EXPECT_TRUE(m_deviceMemory->setInfoFromDmidecode(mapinfo));
    EXPECT_STREQ("CT8G4DFS8266.M8FD", m_deviceMemory->m_Name.toStdString().c_str());
    EXPECT_STREQ("25A1185D", m_deviceMemory->m_SerialNumber.toStdString().c_str());
    EXPECT_STREQ("2400 MHz", m_deviceMemory->m_ConfiguredSpeed.toStdString().c_str());
    EXPECT_STREQ("1.2 V", m_deviceMemory->m_MinimumVoltage.toStdString().c_str());
    EXPECT_STREQ("1.2 V", m_deviceMemory->m_MaximumVoltage.toStdString().c_str());
    EXPECT_STREQ("1.2 V", m_deviceMemory->m_ConfiguredVoltage.toStdString().c_str());
    EXPECT_STREQ("64 bits", m_deviceMemory->m_TotalBandwidth.toStdString().c_str());
    EXPECT_STREQ("64 bits", m_deviceMemory->m_DataBandwidth.toStdString().c_str());
    EXPECT_STREQ("DDR4", m_deviceMemory->m_Type.toStdString().c_str());
    EXPECT_TRUE(m_deviceMemory->m_MatchedFromDmi);
}

TEST_F(UT_DeviceMemory, UT_DeviceMemory_setInfoFromDmidecode_003)
{
    QMap<QString, QString> mapinfo;
    ut_memory_setdmimap(mapinfo);
    mapinfo.insert("Type", "<OUT OF SPEC>");
    m_deviceMemory->m_MatchedFromDmi = false;
    m_deviceMemory->m_Locator = "ChannelA-DIMM0";

    EXPECT_TRUE(m_deviceMemory->setInfoFromDmidecode(mapinfo));
    EXPECT_STREQ("CT8G4DFS8266.M8FD", m_deviceMemory->m_Name.toStdString().c_str());
    EXPECT_STREQ("25A1185D", m_deviceMemory->m_SerialNumber.toStdString().c_str());
    EXPECT_STREQ("2400 MHz", m_deviceMemory->m_ConfiguredSpeed.toStdString().c_str());
    EXPECT_STREQ("1.2 V", m_deviceMemory->m_MinimumVoltage.toStdString().c_str());
    EXPECT_STREQ("1.2 V", m_deviceMemory->m_MaximumVoltage.toStdString().c_str());
    EXPECT_STREQ("1.2 V", m_deviceMemory->m_ConfiguredVoltage.toStdString().c_str());
    EXPECT_STREQ("64 bits", m_deviceMemory->m_TotalBandwidth.toStdString().c_str());
    EXPECT_STREQ("64 bits", m_deviceMemory->m_DataBandwidth.toStdString().c_str());
    EXPECT_STREQ("", m_deviceMemory->m_Type.toStdString().c_str());
    EXPECT_TRUE(m_deviceMemory->m_MatchedFromDmi);
}

TEST_F(UT_DeviceMemory, UT_DeviceMemory_initFilterKey)
{
    m_deviceMemory->initFilterKey();
    EXPECT_EQ(20, m_deviceMemory->m_FilterKey.size());
}

TEST_F(UT_DeviceMemory, UT_DeviceMemory_loadBaseDeviceInfo)
{
    m_deviceMemory->loadBaseDeviceInfo();
    EXPECT_EQ(0, m_deviceMemory->m_LstBaseInfo.size());
}

TEST_F(UT_DeviceMemory, UT_DeviceMemory_loadOtherDeviceInfo)
{
    m_deviceMemory->loadOtherDeviceInfo();
    EXPECT_EQ(0, m_deviceMemory->m_LstOtherInfo.size());
}

TEST_F(UT_DeviceMemory, UT_DeviceMemory_loadTableHeader)
{
    m_deviceMemory->loadTableHeader();
    EXPECT_EQ(5, m_deviceMemory->m_TableHeader.size());
}

TEST_F(UT_DeviceMemory, UT_DeviceMemory_loadTableData)
{
    m_deviceMemory->loadTableData();
    EXPECT_EQ(5, m_deviceMemory->m_TableData.size());
}

TEST_F(UT_DeviceMemory, UT_DeviceMemory_name)
{
    QMap<QString, QString> mapinfo;
    ut_memory_setlshwmap(mapinfo);
    m_deviceMemory->setInfoFromLshw(mapinfo);

    QString name = m_deviceMemory->name();
    EXPECT_STREQ("CT8G4DFS8266.M8FD", name.toStdString().c_str());
}

TEST_F(UT_DeviceMemory, UT_DeviceMemory_driver)
{
    QString driver = m_deviceMemory->driver();
    EXPECT_STREQ("", driver.toStdString().c_str());
}

TEST_F(UT_DeviceMemory, UT_DeviceMemory_subTitle)
{
    QMap<QString, QString> mapinfo;
    ut_memory_setlshwmap(mapinfo);
    m_deviceMemory->setInfoFromLshw(mapinfo);

    QString title = m_deviceMemory->subTitle();
    EXPECT_STREQ("859B CT8G4DFS8266.M8FD", title.toStdString().c_str());
}


TEST_F(UT_DeviceMemory, UT_DeviceMemory_getOverviewInfo)
{
    QMap<QString, QString> mapinfo;
    ut_memory_setdmimap(mapinfo);
    m_deviceMemory->m_MatchedFromDmi = false;
    m_deviceMemory->m_Locator = "ChannelA-DIMM0";
    m_deviceMemory->setInfoFromDmidecode(mapinfo);

    QMap<QString, QString> mapinfo1;
    ut_memory_setlshwmap(mapinfo1);
    m_deviceMemory->setInfoFromLshw(mapinfo1);

    QString overview = m_deviceMemory->getOverviewInfo();
    EXPECT_STREQ("8GB(CT8G4DFS8266.M8FD DDR4 2667MHz)", overview.toStdString().c_str());
}

TEST_F(UT_DeviceMemory, UT_DeviceMemory_available)
{
    EXPECT_TRUE(m_deviceMemory->available());
}
