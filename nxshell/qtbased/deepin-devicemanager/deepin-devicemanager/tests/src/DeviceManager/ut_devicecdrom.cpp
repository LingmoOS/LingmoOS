// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DeviceCdrom.h"

#include "stub.h"
#include "ut_Head.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>


class UT_DeviceCdrom : public UT_HEAD
{
public:
    void SetUp()
    {
        m_deviceCdrom = new DeviceCdrom;
    }
    void TearDown()
    {
        delete m_deviceCdrom;
    }
    DeviceCdrom *m_deviceCdrom;
};

void ut_cdrom_setlshwmap(QMap<QString, QString> &mapInfo)
{
    mapInfo.insert("bus info", "usb@1:8");
    mapInfo.insert("product", "product");
    mapInfo.insert("vendor", "vendor");
    mapInfo.insert("version", "version");
    mapInfo.insert("capabilities", "capabilities");
}

TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_setInfoFromLshw_001)
{
    QMap<QString, QString> mapInfo;
    ut_cdrom_setlshwmap(mapInfo);
    mapInfo.remove("bus info");

    ASSERT_FALSE(m_deviceCdrom->setInfoFromLshw(mapInfo));
}

TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_setInfoFromLshw_002)
{
    QMap<QString, QString> mapInfo;
    ut_cdrom_setlshwmap(mapInfo);
    mapInfo.insert("bus info", "usb1:8");

    ASSERT_FALSE(m_deviceCdrom->setInfoFromLshw(mapInfo));
}

TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_setInfoFromLshw_003)
{
    QMap<QString, QString> mapInfo;
    ut_cdrom_setlshwmap(mapInfo);
    m_deviceCdrom->m_HwinfoToLshw = "usb@1:9";
    ASSERT_FALSE(m_deviceCdrom->setInfoFromLshw(mapInfo));
}

TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_setInfoFromLshw_004)
{
    QMap<QString, QString> mapInfo;
    ut_cdrom_setlshwmap(mapInfo);
    m_deviceCdrom->m_HwinfoToLshw = "usb@1:8";

    ASSERT_TRUE(m_deviceCdrom->setInfoFromLshw(mapInfo));
    EXPECT_STREQ("product", m_deviceCdrom->m_Name.toStdString().c_str());
    EXPECT_STREQ("vendor", m_deviceCdrom->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("version", m_deviceCdrom->m_Version.toStdString().c_str());
    EXPECT_STREQ("usb@1:8", m_deviceCdrom->m_BusInfo.toStdString().c_str());
    EXPECT_STREQ("capabilities", m_deviceCdrom->m_Capabilities.toStdString().c_str());
}

void ut_cdrom_sethwinfomap(QMap<QString, QString> &mapInfo)
{
    mapInfo.insert("Device", "Device");
    mapInfo.insert("Vendor", "Vendor");
    mapInfo.insert("Model", "Model");
    mapInfo.insert("Revision", "Revision");
    mapInfo.insert("SysFS BusID", "1-5:1.0");
    mapInfo.insert("Driver", "Driver");
    mapInfo.insert("Speed", "Speed");
}


TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_setInfoFromHwinfo)
{
    QMap<QString, QString> mapInfo;
    ut_cdrom_sethwinfomap(mapInfo);

    m_deviceCdrom->setInfoFromHwinfo(mapInfo);
    EXPECT_STREQ("Device", m_deviceCdrom->m_Name.toStdString().c_str());
    EXPECT_STREQ("Vendor", m_deviceCdrom->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("Model", m_deviceCdrom->m_Type.toStdString().c_str());
    EXPECT_STREQ("Revision", m_deviceCdrom->m_Version.toStdString().c_str());
    EXPECT_STREQ("1-5:1.0", m_deviceCdrom->m_BusInfo.toStdString().c_str());
    EXPECT_STREQ("Driver", m_deviceCdrom->m_Driver.toStdString().c_str());
    EXPECT_STREQ("Speed", m_deviceCdrom->m_Speed.toStdString().c_str());
    EXPECT_STREQ("usb@1:5", m_deviceCdrom->m_HwinfoToLshw.toStdString().c_str());
}

TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_name)
{
    QMap<QString, QString> mapInfo;
    ut_cdrom_sethwinfomap(mapInfo);
    m_deviceCdrom->setInfoFromHwinfo(mapInfo);

    QString name = m_deviceCdrom->name();
    EXPECT_STREQ("Device", name.toStdString().c_str());
}

TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_getOverviewInfo)
{
    QMap<QString, QString> mapInfo;
    ut_cdrom_sethwinfomap(mapInfo);
    m_deviceCdrom->setInfoFromHwinfo(mapInfo);

    QString overview = m_deviceCdrom->getOverviewInfo();
    EXPECT_STREQ("Device", overview.toStdString().c_str());
}

TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_driver)
{
    QMap<QString, QString> mapInfo;
    ut_cdrom_sethwinfomap(mapInfo);
    m_deviceCdrom->setInfoFromHwinfo(mapInfo);

    QString driver = m_deviceCdrom->driver();
    EXPECT_STREQ("Driver", driver.toStdString().c_str());
}

TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_subTitle)
{
    QMap<QString, QString> mapInfo;
    ut_cdrom_sethwinfomap(mapInfo);
    m_deviceCdrom->setInfoFromHwinfo(mapInfo);

    QString title = m_deviceCdrom->subTitle();
    EXPECT_STREQ("Device", title.toStdString().c_str());
}

TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_initFilterKey)
{
    m_deviceCdrom->initFilterKey();
    EXPECT_EQ(12, m_deviceCdrom->m_FilterKey.size());
}

TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_loadBaseDeviceInfo)
{
    m_deviceCdrom->loadBaseDeviceInfo();
    EXPECT_EQ(0, m_deviceCdrom->m_LstBaseInfo.size());
}

TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_loadOtherDeviceInfo)
{
    m_deviceCdrom->loadOtherDeviceInfo();
    EXPECT_EQ(0, m_deviceCdrom->m_LstOtherInfo.size());
}

TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_loadTableData_001)
{
    QMap<QString, QString> mapInfo;
    ut_cdrom_sethwinfomap(mapInfo);
    m_deviceCdrom->setInfoFromHwinfo(mapInfo);

    m_deviceCdrom->m_Enable = true;
    m_deviceCdrom->loadTableData();
    EXPECT_EQ(3, m_deviceCdrom->m_TableData.size());
}

TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_loadTableData_002)
{
    QMap<QString, QString> mapInfo;
    ut_cdrom_sethwinfomap(mapInfo);
    m_deviceCdrom->setInfoFromHwinfo(mapInfo);

    m_deviceCdrom->m_Enable = false;
    m_deviceCdrom->m_Available = false;
    m_deviceCdrom->loadTableData();
    EXPECT_EQ(3, m_deviceCdrom->m_TableData.size());
}
