// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DeviceImage.h"
#include "DBusEnableInterface.h"
#include "DeviceInfo.h"

#include "stub.h"
#include "ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class UT_DeviceImage : public UT_HEAD
{
public:
    void SetUp()
    {
        m_deviceImage = new DeviceImage;
    }
    void TearDown()
    {
        delete m_deviceImage;
    }
    DeviceImage *m_deviceImage;
};

void ut_image_setlshwmap(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("bus info", "usb@1:8");
    mapinfo.insert("product", "product");
    mapinfo.insert("vendor", "vendor");
    mapinfo.insert("version", "version");
    mapinfo.insert("capabilities", "capabilities");
    mapinfo.insert("driver", "driver");
    mapinfo.insert("maxpower", "maxpower");
    mapinfo.insert("speed", "speed");
}

void ut_image_sethwinfomap(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("Device", "Device");
    mapinfo.insert("Vendor", "Vendor");
    mapinfo.insert("Model", "Model");
    mapinfo.insert("Revision", "Revision");
    mapinfo.insert("SysFS BusID", "1-8:1.0");
    mapinfo.insert("Driver", "Driver");
    mapinfo.insert("Driver Modules", "Driver Modules");
    mapinfo.insert("Speed", "Speed");
    mapinfo.insert("Enable", "Enable");
}

TEST_F(UT_DeviceImage, UT_DeviceImage_setInfoFromLshw_001)
{
    QMap<QString, QString> mapinfo;
    ut_image_setlshwmap(mapinfo);
    m_deviceImage->m_HwinfoToLshw = "usb@1:8";

    m_deviceImage->setInfoFromLshw(mapinfo);
    EXPECT_STREQ("product", m_deviceImage->m_Name.toStdString().c_str());
    EXPECT_STREQ("vendor", m_deviceImage->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("version", m_deviceImage->m_Version.toStdString().c_str());
    EXPECT_STREQ("usb@1:8", m_deviceImage->m_BusInfo.toStdString().c_str());
    EXPECT_STREQ("capabilities", m_deviceImage->m_Capabilities.toStdString().c_str());
    EXPECT_STREQ("driver", m_deviceImage->m_Driver.toStdString().c_str());
    EXPECT_STREQ("maxpower", m_deviceImage->m_MaximumPower.toStdString().c_str());
    EXPECT_STREQ("speed", m_deviceImage->m_Speed.toStdString().c_str());
}

TEST_F(UT_DeviceImage, UT_DeviceImage_setInfoFromLshw_002)
{
    QMap<QString, QString> mapinfo;
    ut_image_setlshwmap(mapinfo);
    m_deviceImage->m_HwinfoToLshw = "usb@1:9";

    m_deviceImage->setInfoFromLshw(mapinfo);
}

TEST_F(UT_DeviceImage, UT_DeviceImage_setInfoFromHwinfo_001)
{
    QMap<QString, QString> mapinfo;
    ut_image_sethwinfomap(mapinfo);

    m_deviceImage->setInfoFromHwinfo(mapinfo);
    EXPECT_STREQ("Device", m_deviceImage->m_Name.toStdString().c_str());
    EXPECT_STREQ("Vendor", m_deviceImage->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("Model", m_deviceImage->m_Model.toStdString().c_str());
    EXPECT_STREQ("Revision", m_deviceImage->m_Version.toStdString().c_str());
    EXPECT_STREQ("1-8:1.0", m_deviceImage->m_BusInfo.toStdString().c_str());
    EXPECT_STREQ("Driver Modules", m_deviceImage->m_Driver.toStdString().c_str());
    EXPECT_STREQ("Speed", m_deviceImage->m_Speed.toStdString().c_str());
    EXPECT_STREQ("usb@1:8", m_deviceImage->m_HwinfoToLshw.toStdString().c_str());
    EXPECT_FALSE(m_deviceImage->m_Enable);
}

TEST_F(UT_DeviceImage, UT_DeviceImage_setInfoFromHwinfo_002)
{
    QMap<QString, QString> mapinfo;
    mapinfo.insert("unique_id", "unique_id");
    mapinfo.insert("name", "name");
    mapinfo.insert("path", "path");
    mapinfo.insert("Hardware Class", "Hardware Class");

    m_deviceImage->setInfoFromHwinfo(mapinfo);
    EXPECT_STREQ("name", m_deviceImage->m_Name.toStdString().c_str());
    EXPECT_STREQ("path", m_deviceImage->m_SysPath.toStdString().c_str());
    EXPECT_STREQ("Hardware Class", m_deviceImage->m_HardwareClass.toStdString().c_str());
    EXPECT_STREQ("unique_id", m_deviceImage->m_UniqueID.toStdString().c_str());
    EXPECT_FALSE(m_deviceImage->m_Enable);
}

TEST_F(UT_DeviceImage, UT_DeviceImage_name)
{
    QMap<QString, QString> mapinfo;
    ut_image_sethwinfomap(mapinfo);
    m_deviceImage->setInfoFromHwinfo(mapinfo);

    QString name = m_deviceImage->name();
    EXPECT_STREQ("Device", name.toStdString().c_str());
}

TEST_F(UT_DeviceImage, UT_DeviceImage_driver)
{
    QMap<QString, QString> mapinfo;
    ut_image_sethwinfomap(mapinfo);
    m_deviceImage->setInfoFromHwinfo(mapinfo);

    QString driver = m_deviceImage->driver();
    EXPECT_STREQ("Driver Modules", driver.toStdString().c_str());
}

TEST_F(UT_DeviceImage, UT_DeviceImage_subTitle)
{
    QMap<QString, QString> mapinfo;
    ut_image_sethwinfomap(mapinfo);
    m_deviceImage->setInfoFromHwinfo(mapinfo);

    QString title = m_deviceImage->subTitle();
    EXPECT_STREQ("Device", title.toStdString().c_str());
}

TEST_F(UT_DeviceImage, UT_DeviceImage_getOverviewInfo_001)
{
    QMap<QString, QString> mapinfo;
    ut_image_sethwinfomap(mapinfo);
    m_deviceImage->setInfoFromHwinfo(mapinfo);

    QString overview = m_deviceImage->getOverviewInfo();
    EXPECT_STREQ("Device (Model)", overview.toStdString().c_str());
}

bool ut_image_enable_true()
{
    return true;
}

TEST_F(UT_DeviceImage, UT_DeviceImage_setEnable_001)
{
    EXPECT_EQ(EnableDeviceStatus::EDS_NoSerial, m_deviceImage->setEnable(true));
}

TEST_F(UT_DeviceImage, UT_DeviceImage_setEnable_002)
{
    m_deviceImage->m_SerialID = "serial";
    m_deviceImage->m_UniqueID = "unique_id";
    m_deviceImage->m_SysPath = "path";
    Stub stub;
    stub.set(ADDR(DBusEnableInterface, enable), ut_image_enable_true);

    EnableDeviceStatus ret = m_deviceImage->setEnable(false);
    EXPECT_EQ(EnableDeviceStatus::EDS_Success, ret);
    EXPECT_FALSE(m_deviceImage->m_Enable);
}

TEST_F(UT_DeviceImage, UT_DeviceImage_enable)
{
    EXPECT_TRUE(m_deviceImage->enable());
}

TEST_F(UT_DeviceImage, UT_DeviceImage_initFilterKey)
{
    m_deviceImage->initFilterKey();
    EXPECT_EQ(0, m_deviceImage->m_FilterKey.size());
}

TEST_F(UT_DeviceImage, UT_DeviceImage_loadBaseDeviceInfo)
{
    m_deviceImage->loadBaseDeviceInfo();
    EXPECT_EQ(0, m_deviceImage->m_LstBaseInfo.size());
}

TEST_F(UT_DeviceImage, UT_DeviceImage_loadOtherDeviceInfo)
{
    m_deviceImage->loadOtherDeviceInfo();
    EXPECT_EQ(0, m_deviceImage->m_LstOtherInfo.size());
}

TEST_F(UT_DeviceImage, UT_DeviceImage_loadTableData_001)
{
    QMap<QString, QString> mapinfo;
    ut_image_sethwinfomap(mapinfo);
    m_deviceImage->setInfoFromHwinfo(mapinfo);
    m_deviceImage->m_Enable = true;
    m_deviceImage->m_Available = false;

    m_deviceImage->loadTableData();
    EXPECT_STREQ("(Unavailable) Device", m_deviceImage->m_TableData.at(0).toStdString().c_str());
    EXPECT_EQ(3, m_deviceImage->m_TableData.size());
}

TEST_F(UT_DeviceImage, UT_DeviceImage_loadTableData_002)
{
    QMap<QString, QString> mapinfo;
    ut_image_sethwinfomap(mapinfo);
    m_deviceImage->setInfoFromHwinfo(mapinfo);

    m_deviceImage->loadTableData();
    EXPECT_STREQ("(Disable) Device", m_deviceImage->m_TableData.at(0).toStdString().c_str());
    EXPECT_EQ(3, m_deviceImage->m_TableData.size());
}
