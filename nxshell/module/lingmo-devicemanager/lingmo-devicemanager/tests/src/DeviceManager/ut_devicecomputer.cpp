// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DeviceComputer.h"

#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class UT_DeviceComputer : public UT_HEAD
{
public:
    void SetUp()
    {
        m_deviceComputer = new DeviceComputer;
    }
    void TearDown()
    {
        delete m_deviceComputer;
    }
    DeviceComputer *m_deviceComputer;
};

TEST_F(UT_DeviceComputer, UT_DeviceComputer_name)
{
    m_deviceComputer->name();
}

TEST_F(UT_DeviceComputer, UT_DeviceComputer_driver)
{
    QString driver = m_deviceComputer->driver();
    EXPECT_STREQ("", driver.toStdString().c_str());
}



TEST_F(UT_DeviceComputer, UT_DeviceComputer_setHomeUrl)
{
    m_deviceComputer->setHomeUrl("http://url");
    EXPECT_STREQ("http://url", m_deviceComputer->m_HomeUrl.toStdString().c_str());

}

TEST_F(UT_DeviceComputer, UT_DeviceComputer_setOsDescription)
{
    m_deviceComputer->setOsDescription("OsDescription");
    EXPECT_STREQ("OsDescription", m_deviceComputer->m_OsDescription.toStdString().c_str());
}

TEST_F(UT_DeviceComputer, UT_DeviceComputer_setOS)
{
    m_deviceComputer->setOS("OS");
    EXPECT_STREQ("OS", m_deviceComputer->m_OS.toStdString().c_str());
}

TEST_F(UT_DeviceComputer, UT_DeviceComputer_setVendor_001)
{
    m_deviceComputer->setVendor("Vendor");
    EXPECT_STREQ("Vendor", m_deviceComputer->m_Vendor.toStdString().c_str());
}

TEST_F(UT_DeviceComputer, UT_DeviceComputer_setVendor_002)
{
    m_deviceComputer->setVendor("System manufacturer", "Vendor");
    EXPECT_STREQ("Vendor", m_deviceComputer->m_Vendor.toStdString().c_str());
}

TEST_F(UT_DeviceComputer, UT_DeviceComputer_setVendor_003)
{
    m_deviceComputer->setVendor("Vendor1", "Vendor2");
    EXPECT_STREQ("Vendor1", m_deviceComputer->m_Vendor.toStdString().c_str());
}

TEST_F(UT_DeviceComputer, UT_DeviceComputer_setName_001)
{
    m_deviceComputer->setName("None");
    EXPECT_STREQ("", m_deviceComputer->m_Name.toStdString().c_str());
}

TEST_F(UT_DeviceComputer, UT_DeviceComputer_setName_002)
{
    QString dmi1Name = "System Product Name";
    QString dmi2Name = "dmi2Name";
    QString dmi1Family = "Not Applicable";
    QString dmi1Version = "Not Applicable";
    m_deviceComputer->m_Vendor = "vendor";

    m_deviceComputer->setName(dmi1Name, dmi2Name, dmi1Family, dmi1Version);
    EXPECT_STREQ("  dmi2Name", m_deviceComputer->m_Name.toStdString().c_str());
}

TEST_F(UT_DeviceComputer, UT_DeviceComputer_setName_003)
{
    QString dmi1Name = "dmi1Name";
    QString dmi2Name = "dmi2Name";
    QString dmi1Family = "Not Applicable";
    QString dmi1Version = "Not Applicable";
    m_deviceComputer->m_Vendor = "vendor";

    m_deviceComputer->setName(dmi1Name, dmi2Name, dmi1Family, dmi1Version);
    EXPECT_STREQ("  dmi1Name", m_deviceComputer->m_Name.toStdString().c_str());
}

TEST_F(UT_DeviceComputer, UT_DeviceComputer_setType)
{
    m_deviceComputer->setType("Type");
    EXPECT_STREQ("Type", m_deviceComputer->m_Type.toStdString().c_str());
}

TEST_F(UT_DeviceComputer, UT_DeviceComputer_getOverviewInfo)
{
    m_deviceComputer->m_Vendor = "vendor";
    m_deviceComputer->m_Name = "name";
    m_deviceComputer->m_Type = "type";

    QString model = m_deviceComputer->getOverviewInfo();
    EXPECT_STREQ("vendor name type ", model.toStdString().c_str());
}

TEST_F(UT_DeviceComputer, UT_DeviceComputer_getOSInfo)
{
    m_deviceComputer->m_OS = "os";
    m_deviceComputer->m_OsDescription = "OsDescription";

    QString model = m_deviceComputer->getOSInfo();
    EXPECT_STREQ("OsDescription os", model.toStdString().c_str());
}

TEST_F(UT_DeviceComputer, UT_DeviceComputer_initFilterKey)
{
    m_deviceComputer->initFilterKey();
    EXPECT_EQ(0, m_deviceComputer->m_FilterKey.size());
}

TEST_F(UT_DeviceComputer, UT_DeviceComputer_loadBaseDeviceInfo)
{
    m_deviceComputer->loadBaseDeviceInfo();
    EXPECT_EQ(0, m_deviceComputer->m_LstBaseInfo.size());
}

TEST_F(UT_DeviceComputer, UT_DeviceComputer_loadOtherDeviceInfo)
{
    m_deviceComputer->loadOtherDeviceInfo();
    EXPECT_EQ(0, m_deviceComputer->m_LstOtherInfo.size());
}

TEST_F(UT_DeviceComputer, UT_DeviceComputer_loadTableData)
{
    m_deviceComputer->loadTableData();
    EXPECT_EQ(0, m_deviceComputer->m_TableData.size());
}
