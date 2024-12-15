// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DeviceBluetooth.h"
#include "DBusEnableInterface.h"
#include "stub.h"
#include "ut_Head.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>


class UT_DeviceBluetooth : public UT_HEAD
{
public:
    void SetUp()
    {
        m_deviceBluetooth = new DeviceBluetooth;
    }
    void TearDown()
    {
        delete m_deviceBluetooth;
    }
    DeviceBluetooth *m_deviceBluetooth;
};

TEST_F(UT_DeviceBluetooth, UT_DeviceBluetooth_setInfoFromHciconfig)
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Name", "Name");
    mapInfo.insert("Manufacturer", "Manufacturer");

    m_deviceBluetooth->setInfoFromHciconfig(mapInfo);
    EXPECT_STREQ("Name", m_deviceBluetooth->m_Name.toStdString().c_str());
    EXPECT_STREQ("Manufacturer", m_deviceBluetooth->m_Vendor.toStdString().c_str());
}

void ut_bluetooth_sethwinfomap(QMap<QString, QString> &mapInfo)
{
    mapInfo.insert("Vendor", "Vendor");
    mapInfo.insert("Revision", "Revision");
    mapInfo.insert("Model", "Model");
    mapInfo.insert("SysFS BusID", "1-2:1.0");
    mapInfo.insert("Driver", "Driver");
    mapInfo.insert("Speed", "Speed");
}

TEST_F(UT_DeviceBluetooth, UT_DeviceBluetooth_setInfoFromHwinfo_001)
{
    QMap<QString, QString> mapInfo;
    ut_bluetooth_sethwinfomap(mapInfo);
    mapInfo.remove("vendor");

    ASSERT_TRUE(m_deviceBluetooth->setInfoFromHwinfo(mapInfo));
}

TEST_F(UT_DeviceBluetooth, UT_DeviceBluetooth_setInfoFromHwinfo_002)
{
    QMap<QString, QString> mapInfo;
    ut_bluetooth_sethwinfomap(mapInfo);
    m_deviceBluetooth->m_Vendor = "Centor";

    ASSERT_TRUE(m_deviceBluetooth->setInfoFromHwinfo(mapInfo));
}

TEST_F(UT_DeviceBluetooth, UT_DeviceBluetooth_setInfoFromHwinfo_003)
{
    QMap<QString, QString> mapInfo;
    ut_bluetooth_sethwinfomap(mapInfo);
    m_deviceBluetooth->m_Vendor = "Vendor";

    ASSERT_TRUE(m_deviceBluetooth->setInfoFromHwinfo(mapInfo));
    EXPECT_STREQ("Revision", m_deviceBluetooth->m_Version.toStdString().c_str());
    EXPECT_STREQ("Model", m_deviceBluetooth->m_Model.toStdString().c_str());
    EXPECT_STREQ("1-2:1.0", m_deviceBluetooth->m_BusInfo.toStdString().c_str());
    EXPECT_STREQ("Driver", m_deviceBluetooth->m_Driver.toStdString().c_str());
    EXPECT_STREQ("Speed", m_deviceBluetooth->m_Speed.toStdString().c_str());
}

TEST_F(UT_DeviceBluetooth, UT_DeviceBluetooth_setInfoFromHwinfo_004)
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("path", "path");
    mapInfo.insert("name", "name");
    mapInfo.insert("Hardware Class", "Hardware Class");
    mapInfo.insert("unique_id", "unique_id");

    ASSERT_TRUE(m_deviceBluetooth->setInfoFromHwinfo(mapInfo));
    EXPECT_STREQ("name", m_deviceBluetooth->m_Name.toStdString().c_str());
    EXPECT_STREQ("path", m_deviceBluetooth->m_SysPath.toStdString().c_str());
    EXPECT_STREQ("Hardware Class", m_deviceBluetooth->m_HardwareClass.toStdString().c_str());
    EXPECT_STREQ("unique_id", m_deviceBluetooth->m_UniqueID.toStdString().c_str());
    EXPECT_FALSE(m_deviceBluetooth->m_Enable);
}

void ut_bluetooth_setlshwmap(QMap<QString, QString> &mapInfo)
{
    mapInfo.insert("vendor", "vendor");
    mapInfo.insert("version", "version");
    mapInfo.insert("product", "product");
    mapInfo.insert("bus info", "usb@1:2");
    mapInfo.insert("capabilities", "capabilities");
    mapInfo.insert("driver", "driver");
    mapInfo.insert("maxpower", "maxpower");
    mapInfo.insert("speed", "speed");
}

TEST_F(UT_DeviceBluetooth, UT_DeviceBluetooth_setInfoFromLshw_001)
{
    QMap<QString, QString> mapInfo;
    ut_bluetooth_setlshwmap(mapInfo);
    m_deviceBluetooth->m_HwinfoToLshw = "usb@1:8";

    ASSERT_FALSE(m_deviceBluetooth->setInfoFromLshw(mapInfo));
}

TEST_F(UT_DeviceBluetooth, UT_DeviceBluetooth_setInfoFromLshw_002)
{
    QMap<QString, QString> mapInfo;
    ut_bluetooth_setlshwmap(mapInfo);
    m_deviceBluetooth->m_HwinfoToLshw = "usb@1:2";

    ASSERT_TRUE(m_deviceBluetooth->setInfoFromLshw(mapInfo));
    EXPECT_STREQ("vendor", m_deviceBluetooth->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("version", m_deviceBluetooth->m_Version.toStdString().c_str());
    EXPECT_STREQ("product", m_deviceBluetooth->m_LogicalName.toStdString().c_str());
    EXPECT_STREQ("usb@1:2", m_deviceBluetooth->m_BusInfo.toStdString().c_str());
    EXPECT_STREQ("capabilities", m_deviceBluetooth->m_Capabilities.toStdString().c_str());
    EXPECT_STREQ("driver", m_deviceBluetooth->m_Driver.toStdString().c_str());
    EXPECT_STREQ("maxpower", m_deviceBluetooth->m_MaximumPower.toStdString().c_str());
    EXPECT_STREQ("speed", m_deviceBluetooth->m_Speed.toStdString().c_str());
}

TEST_F(UT_DeviceBluetooth, UT_DeviceBluetooth_name)
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Name", "Name");
    mapInfo.insert("Manufacturer", "Manufacturer");
    m_deviceBluetooth->setInfoFromHciconfig(mapInfo);

    QString name = m_deviceBluetooth->name();
    EXPECT_STREQ("Name", name.toStdString().c_str());

}

TEST_F(UT_DeviceBluetooth, UT_DeviceBluetooth_driver)
{
    QMap<QString, QString> mapInfo;
    ut_bluetooth_sethwinfomap(mapInfo);
    m_deviceBluetooth->m_Vendor = "Vendor";
    m_deviceBluetooth->setInfoFromHwinfo(mapInfo);

    QString driver = m_deviceBluetooth->driver();
    EXPECT_STREQ("Driver", driver.toStdString().c_str());
}

TEST_F(UT_DeviceBluetooth, UT_DeviceBluetooth_subTitle)
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Name", "Name");
    mapInfo.insert("Manufacturer", "Manufacturer");
    m_deviceBluetooth->setInfoFromHciconfig(mapInfo);

    QString title = m_deviceBluetooth->subTitle();
    EXPECT_STREQ("Name", title.toStdString().c_str());

}

TEST_F(UT_DeviceBluetooth, UT_DeviceBluetooth_getOverviewInfo)
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Name", "Name");
    mapInfo.insert("Manufacturer", "Manufacturer");
    m_deviceBluetooth->setInfoFromHciconfig(mapInfo);

    QString overview = m_deviceBluetooth->getOverviewInfo();
    EXPECT_STREQ("Name", overview.toStdString().c_str());
}

TEST_F(UT_DeviceBluetooth, UT_DeviceBluetooth_setEnable_001)
{
    m_deviceBluetooth->m_SerialID = "SerialID";
    EXPECT_EQ(EnableDeviceStatus::EDS_Faild, m_deviceBluetooth->setEnable(true));
}

TEST_F(UT_DeviceBluetooth, UT_DeviceBluetooth_setEnable_002)
{
    EXPECT_EQ(EnableDeviceStatus::EDS_NoSerial, m_deviceBluetooth->setEnable(false));
}

bool ut_bluetooth_enable_true()
{
    return true;
}

TEST_F(UT_DeviceBluetooth, UT_DeviceBluetooth_setEnable_003)
{
    m_deviceBluetooth->m_SerialID = "SerialID";
    m_deviceBluetooth->m_UniqueID = "UniqueID";
    m_deviceBluetooth->m_SysPath = "SysPath";

    Stub stub;
    stub.set(ADDR(DBusEnableInterface, enable), ut_bluetooth_enable_true);

    EXPECT_EQ(EnableDeviceStatus::EDS_Success, m_deviceBluetooth->setEnable(false));
}

TEST_F(UT_DeviceBluetooth, UT_DeviceBluetooth_enable)
{
    EXPECT_TRUE(m_deviceBluetooth->enable());
}

TEST_F(UT_DeviceBluetooth, UT_DeviceBluetooth_initFilterKey)
{
    m_deviceBluetooth->initFilterKey();
    EXPECT_EQ(25, m_deviceBluetooth->m_FilterKey.size());
}

TEST_F(UT_DeviceBluetooth, UT_DeviceBluetooth_loadBaseDeviceInfo)
{
    m_deviceBluetooth->loadBaseDeviceInfo();
    EXPECT_EQ(0, m_deviceBluetooth->m_LstBaseInfo.size());
}

TEST_F(UT_DeviceBluetooth, UT_DeviceBluetooth_loadOtherDeviceInfo)
{
    m_deviceBluetooth->loadOtherDeviceInfo();
    EXPECT_EQ(1, m_deviceBluetooth->m_LstOtherInfo.size());
}

TEST_F(UT_DeviceBluetooth, UT_DeviceBluetooth_loadTableData_001)
{
    QMap<QString, QString> mapinfo;
    ut_bluetooth_sethwinfomap(mapinfo);
    m_deviceBluetooth->setInfoFromHwinfo(mapinfo);

    m_deviceBluetooth->loadTableData();
    EXPECT_EQ(3, m_deviceBluetooth->m_TableData.size());
}

TEST_F(UT_DeviceBluetooth, UT_DeviceBluetooth_loadTableData_002)
{
    QMap<QString, QString> mapinfo;
    ut_bluetooth_sethwinfomap(mapinfo);
    m_deviceBluetooth->setInfoFromHwinfo(mapinfo);
    m_deviceBluetooth->m_Available = false;
    m_deviceBluetooth->m_Enable = false;

    m_deviceBluetooth->loadTableData();
    EXPECT_EQ(3, m_deviceBluetooth->m_TableData.size());
}
