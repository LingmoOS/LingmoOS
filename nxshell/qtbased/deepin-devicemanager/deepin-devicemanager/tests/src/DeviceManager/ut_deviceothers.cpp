// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DeviceOthers.h"
#include "DeviceBios.h"
#include "DBusEnableInterface.h"

#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class UT_DeviceOther : public UT_HEAD
{
public:
    void SetUp()
    {
        m_deviceOthers = new DeviceOthers;
    }
    void TearDown()
    {
        delete m_deviceOthers;
    }
    DeviceOthers *m_deviceOthers;
};

void ut_other_setlshwinfo(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("product", "product");
    mapinfo.insert("vendor", "vendor");
    mapinfo.insert("product", "product");
    mapinfo.insert("version", "version");
    mapinfo.insert("bus info", "usb@1:8");
    mapinfo.insert("capabilities", "capabilities");
    mapinfo.insert("driver", "driver");
    mapinfo.insert("maxpower", "maxpower");
    mapinfo.insert("speed", "speed");
    mapinfo.insert("logical name", "logical name");
}

void ut_other_sethwinfoinfo(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("Device", "Device");
    mapinfo.insert("Vendor", "Vendor");
    mapinfo.insert("Model", "Model");
    mapinfo.insert("Revision", "Revision");
    mapinfo.insert("Driver", "Driver");
    mapinfo.insert("Speed", "Speed");
    mapinfo.insert("Unique ID", "Unique ID");
    mapinfo.insert("SysFS BusID", "1-8:1.0");
}

TEST_F(UT_DeviceOther, UT_DeviceOther_setInfoFromLshw_001)
{
    QMap<QString, QString> mapinfo;
    ut_other_setlshwinfo(mapinfo);
    m_deviceOthers->m_HwinfoToLshw = "usb@1:8";

    m_deviceOthers->setInfoFromLshw(mapinfo);
    EXPECT_STREQ("product", m_deviceOthers->m_Name.toStdString().c_str());
    EXPECT_STREQ("vendor", m_deviceOthers->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("product", m_deviceOthers->m_Model.toStdString().c_str());
    EXPECT_STREQ("version", m_deviceOthers->m_Version.toStdString().c_str());
    EXPECT_STREQ("usb@1:8", m_deviceOthers->m_BusInfo.toStdString().c_str());
    EXPECT_STREQ("capabilities", m_deviceOthers->m_Capabilities.toStdString().c_str());
    EXPECT_STREQ("driver", m_deviceOthers->m_Driver.toStdString().c_str());
    EXPECT_STREQ("maxpower", m_deviceOthers->m_MaximumPower.toStdString().c_str());
    EXPECT_STREQ("speed", m_deviceOthers->m_Speed.toStdString().c_str());
    EXPECT_STREQ("logical name", m_deviceOthers->m_LogicalName.toStdString().c_str());
}

TEST_F(UT_DeviceOther, UT_DeviceOther_setInfoFromLshw_002)
{
    QMap<QString, QString> mapinfo;
    ut_other_setlshwinfo(mapinfo);
    mapinfo.insert("bus info", "usb@1:7");
    m_deviceOthers->m_HwinfoToLshw = "usb@1:8";

    m_deviceOthers->setInfoFromLshw(mapinfo);
}

TEST_F(UT_DeviceOther, UT_DeviceOther_setInfoFromHwinfo)
{
    QMap<QString, QString> mapinfo;
    ut_other_sethwinfoinfo(mapinfo);

    m_deviceOthers->setInfoFromHwinfo(mapinfo);
    EXPECT_STREQ("Device", m_deviceOthers->m_Name.toStdString().c_str());
    EXPECT_STREQ("Vendor", m_deviceOthers->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("Model", m_deviceOthers->m_Model.toStdString().c_str());
    EXPECT_STREQ("Revision", m_deviceOthers->m_Version.toStdString().c_str());
    EXPECT_STREQ("Driver", m_deviceOthers->m_Driver.toStdString().c_str());
    EXPECT_STREQ("Speed", m_deviceOthers->m_Speed.toStdString().c_str());
    EXPECT_STREQ("1-8:1", m_deviceOthers->m_BusID.toStdString().c_str());
}

TEST_F(UT_DeviceOther, UT_DeviceOther_name)
{
    QMap<QString, QString> mapinfo;
    ut_other_sethwinfoinfo(mapinfo);
    m_deviceOthers->setInfoFromHwinfo(mapinfo);

    QString name = m_deviceOthers->name();
    EXPECT_STREQ("Device", name.toStdString().c_str());
}

TEST_F(UT_DeviceOther, UT_DeviceOther_busInfo)
{
    QMap<QString, QString> mapinfo;
    ut_other_setlshwinfo(mapinfo);
    m_deviceOthers->m_HwinfoToLshw = "usb@1:8";
    m_deviceOthers->setInfoFromLshw(mapinfo);

    QString busInfo = m_deviceOthers->busInfo();
    EXPECT_STREQ("usb@1:8", busInfo.toStdString().c_str());
}

TEST_F(UT_DeviceOther, UT_DeviceOther_driver)
{
    QMap<QString, QString> mapinfo;
    ut_other_sethwinfoinfo(mapinfo);
    m_deviceOthers->setInfoFromHwinfo(mapinfo);

    QString driver = m_deviceOthers->driver();
    EXPECT_STREQ("Driver", driver.toStdString().c_str());
}

TEST_F(UT_DeviceOther, UT_DeviceOther_logicalName)
{
    QMap<QString, QString> mapinfo;
    ut_other_setlshwinfo(mapinfo);
    m_deviceOthers->m_HwinfoToLshw = "usb@1:8";
    m_deviceOthers->setInfoFromLshw(mapinfo);

    QString logicName = m_deviceOthers->logicalName();
    EXPECT_STREQ("logical name", logicName.toStdString().c_str());
}

TEST_F(UT_DeviceOther, UT_DeviceOther_subTitle)
{
    QMap<QString, QString> mapinfo;
    ut_other_sethwinfoinfo(mapinfo);
    m_deviceOthers->setInfoFromHwinfo(mapinfo);

    QString logicName = m_deviceOthers->subTitle();
    EXPECT_STREQ("Model", logicName.toStdString().c_str());
}

TEST_F(UT_DeviceOther, UT_DeviceOther_getOverviewInfo)
{
    QMap<QString, QString> mapinfo;
    ut_other_sethwinfoinfo(mapinfo);
    m_deviceOthers->setInfoFromHwinfo(mapinfo);

    QString overview = m_deviceOthers->getOverviewInfo();
    EXPECT_STREQ("Device", overview.toStdString().c_str());
}

TEST_F(UT_DeviceOther, UT_DeviceOther_initFilterKey)
{
    m_deviceOthers->initFilterKey();
    EXPECT_EQ(2, m_deviceOthers->m_FilterKey.size());
}

TEST_F(UT_DeviceOther, UT_DeviceOther_loadBaseDeviceInfo)
{
    QMap<QString, QString> mapinfo;
    ut_other_sethwinfoinfo(mapinfo);
    m_deviceOthers->setInfoFromHwinfo(mapinfo);
    m_deviceOthers->loadBaseDeviceInfo();
    EXPECT_EQ(6, m_deviceOthers->m_LstBaseInfo.size());
}

TEST_F(UT_DeviceOther, UT_DeviceOther_loadOtherDeviceInfo)
{
    m_deviceOthers->loadOtherDeviceInfo();
    EXPECT_EQ(0, m_deviceOthers->m_LstOtherInfo.size());
}

TEST_F(UT_DeviceOther, UT_DeviceOther_loadTableData)
{
    QMap<QString, QString> mapinfo;
    ut_other_sethwinfoinfo(mapinfo);
    m_deviceOthers->setInfoFromHwinfo(mapinfo);
    m_deviceOthers->m_Available = false;
    m_deviceOthers->m_Enable = false;

    m_deviceOthers->loadTableData();
    EXPECT_EQ(3, m_deviceOthers->m_TableData.size());
}

bool ut_others_enable_true()
{
    return true;
}

TEST_F(UT_DeviceOther, UT_DeviceOther_setEnable_001)
{
    m_deviceOthers->m_SerialID = "";
    EnableDeviceStatus value = m_deviceOthers->setEnable(true);
    EXPECT_EQ(EnableDeviceStatus::EDS_NoSerial, value);
}

TEST_F(UT_DeviceOther, UT_DeviceOther_setEnable_002)
{
    m_deviceOthers->m_SerialID = "serial";
    m_deviceOthers->m_UniqueID = "";
    m_deviceOthers->m_SysPath = "";
    EnableDeviceStatus value = m_deviceOthers->setEnable(true);
    EXPECT_EQ(EnableDeviceStatus::EDS_Faild, value);
}

TEST_F(UT_DeviceOther, UT_DeviceOther_setEnable_003)
{
    m_deviceOthers->m_SerialID = "serial";
    m_deviceOthers->m_UniqueID = "unique";
    m_deviceOthers->m_SysPath = "unique";

    Stub stub;
    stub.set(ADDR(DBusEnableInterface, enable), ut_others_enable_true);

    EnableDeviceStatus value = m_deviceOthers->setEnable(true);
    EXPECT_EQ(EnableDeviceStatus::EDS_Success, value);
    EXPECT_TRUE(m_deviceOthers->m_Enable);
}
