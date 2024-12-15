// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DeviceInput.h"
#include "DeviceBios.h"
#include "DeviceManager.h"
#include "DBusEnableInterface.h"
#include "stub.h"
#include "ut_Head.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class UT_DeviceInput : public UT_HEAD
{
public:
    void SetUp()
    {
        m_deviceInput = new DeviceInput;
    }
    void TearDown()
    {
        delete m_deviceInput;
    }
    DeviceInput *m_deviceInput;
};

void setLshwMap(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("bus info", "usb@1:8");
    mapinfo.insert("vendor", "Cherry Zhuhai");
    mapinfo.insert("version", "1.07");
    mapinfo.insert("capabilities", "usb-2.00");
    mapinfo.insert("description", "Keyboard");
    mapinfo.insert("driver", "usbhid");
    mapinfo.insert("maxpower", "350mA");
    mapinfo.insert("speed", "12Mbit/s");
}

void setHwinfoMap(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("Device", "MX board 8.0");
    mapinfo.insert("Vendor", "Cherry GmbH");
    mapinfo.insert("Model", "Cherry MX board 8.0");
    mapinfo.insert("Revision", "1.07");
    mapinfo.insert("Hotplug", "USB");
    mapinfo.insert("Model", "Cherry MX board 8.0");
    mapinfo.insert("Device", "MX board 8.0");
    mapinfo.insert("SysFS BusID", "1-8:1.1");
    mapinfo.insert("Hardware Class", "keyboard");
    mapinfo.insert("Driver", "usbhid");
    mapinfo.insert("Speed", "12 Mbps");
    mapinfo.insert("Device File", "/dev/input/event4, /dev/input/by-id/usb-Cherry_Zhuhai_MX_board_8.0-event-kbd, /dev/input/by-path/pci");
    mapinfo.insert("Enable", "Enable");
}

TEST_F(UT_DeviceInput, UT_DeviceInput_setInfoFromlshw_001)
{
    m_deviceInput->m_HwinfoToLshw = "usb@1:8";
    QMap<QString, QString> map;
    setLshwMap(map);

    EXPECT_TRUE(m_deviceInput->setInfoFromlshw(map));
    EXPECT_STREQ("Cherry Zhuhai", m_deviceInput->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("1.07", m_deviceInput->m_Version.toStdString().c_str());
    EXPECT_STREQ("usb@1:8", m_deviceInput->m_BusInfo.toStdString().c_str());
    EXPECT_STREQ("usb-2.00", m_deviceInput->m_Capabilities.toStdString().c_str());
    EXPECT_STREQ("Keyboard", m_deviceInput->m_Description.toStdString().c_str());
    EXPECT_STREQ("usbhid", m_deviceInput->m_Driver.toStdString().c_str());
    EXPECT_STREQ("350mA", m_deviceInput->m_MaximumPower.toStdString().c_str());
    EXPECT_STREQ("12Mbit/s", m_deviceInput->m_Speed.toStdString().c_str());
}

TEST_F(UT_DeviceInput, UT_DeviceInput_setInfoFromlshw_002)
{
    m_deviceInput->m_HwinfoToLshw = "usb@10:8";
    QMap<QString, QString> map;
    setLshwMap(map);
    EXPECT_FALSE(m_deviceInput->setInfoFromlshw(map));
}

TEST_F(UT_DeviceInput, UT_DeviceInput_setInfoFromHwinfo_001)
{
    QMap<QString, QString> map;
    setHwinfoMap(map);

    m_deviceInput->setInfoFromHwinfo(map);
    EXPECT_STREQ("MX board 8.0", m_deviceInput->m_Name.toStdString().c_str());
    EXPECT_STREQ("Cherry GmbH", m_deviceInput->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("Cherry MX board 8.0", m_deviceInput->m_Model.toStdString().c_str());
    EXPECT_STREQ("1.07", m_deviceInput->m_Version.toStdString().c_str());
    EXPECT_STREQ("USB", m_deviceInput->m_Interface.toStdString().c_str());
    EXPECT_STREQ("1-8:1.1", m_deviceInput->m_BusInfo.toStdString().c_str());
    EXPECT_STREQ("keyboard", m_deviceInput->m_Description.toStdString().c_str());
    EXPECT_STREQ("usbhid", m_deviceInput->m_Driver.toStdString().c_str());
    EXPECT_STREQ("12 Mbps", m_deviceInput->m_Speed.toStdString().c_str());
    EXPECT_STREQ("usb@1:8", m_deviceInput->m_HwinfoToLshw.toStdString().c_str());
}

TEST_F(UT_DeviceInput, UT_DeviceInput_setInfoFromHwinfo_002)
{
    QMap<QString, QString> map;
    setHwinfoMap(map);
    map.insert("Model", "Cherry MX board 8.0 Bluetooth");
    map.insert("Device", "MX board 8.0 Bluetooth");
    map.insert("Device File", "/dev/input/mice (/dev/input/mouse1)");

    m_deviceInput->setInfoFromHwinfo(map);
    EXPECT_STREQ("MX board 8.0 Bluetooth", m_deviceInput->m_Name.toStdString().c_str());
    EXPECT_STREQ("Cherry GmbH", m_deviceInput->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("Cherry MX board 8.0 Bluetooth", m_deviceInput->m_Model.toStdString().c_str());
    EXPECT_STREQ("1.07", m_deviceInput->m_Version.toStdString().c_str());
    EXPECT_STREQ("USB", m_deviceInput->m_Interface.toStdString().c_str());
    EXPECT_STREQ("1-8:1.1", m_deviceInput->m_BusInfo.toStdString().c_str());
    EXPECT_STREQ("keyboard", m_deviceInput->m_Description.toStdString().c_str());
    EXPECT_STREQ("usbhid", m_deviceInput->m_Driver.toStdString().c_str());
    EXPECT_STREQ("12 Mbps", m_deviceInput->m_Speed.toStdString().c_str());
    EXPECT_STREQ("usb@1:8", m_deviceInput->m_HwinfoToLshw.toStdString().c_str());
}

TEST_F(UT_DeviceInput, UT_DeviceInput_setKLUInfoFromHwinfo_001)
{
    QMap<QString, QString> map;
    setHwinfoMap(map);
    map.insert("Device File", "/dev/input/mice (/dev/input/mouse1)");

    m_deviceInput->setKLUInfoFromHwinfo(map);
    EXPECT_STREQ("MX board 8.0", m_deviceInput->m_Name.toStdString().c_str());
    EXPECT_STREQ("Cherry GmbH", m_deviceInput->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("Cherry MX board 8.0", m_deviceInput->m_Model.toStdString().c_str());
    EXPECT_STREQ("1.07", m_deviceInput->m_Version.toStdString().c_str());
    EXPECT_STREQ("USB", m_deviceInput->m_Interface.toStdString().c_str());
    EXPECT_STREQ("1-8:1.1", m_deviceInput->m_BusInfo.toStdString().c_str());
    EXPECT_STREQ("keyboard", m_deviceInput->m_Description.toStdString().c_str());
    EXPECT_STREQ("usbhid", m_deviceInput->m_Driver.toStdString().c_str());
    EXPECT_STREQ("12 Mbps", m_deviceInput->m_Speed.toStdString().c_str());
    EXPECT_STREQ("usb@1:8", m_deviceInput->m_KeyToLshw.toStdString().c_str());
}

TEST_F(UT_DeviceInput, UT_DeviceInput_setKLUInfoFromHwinfo_002)
{
    QMap<QString, QString> map;
    setHwinfoMap(map);
    map.insert("Model", "Cherry MX board 8.0 Bluetooth");
    map.insert("Device", "MX board 8.0 Bluetooth");
    map.remove("Hotplug");

    m_deviceInput->setKLUInfoFromHwinfo(map);
    EXPECT_STREQ("MX board 8.0 Bluetooth", m_deviceInput->m_Name.toStdString().c_str());
    EXPECT_STREQ("Cherry GmbH", m_deviceInput->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("Cherry MX board 8.0 Bluetooth", m_deviceInput->m_Model.toStdString().c_str());
    EXPECT_STREQ("1.07", m_deviceInput->m_Version.toStdString().c_str());
    EXPECT_STREQ("Bluetooth", m_deviceInput->m_Interface.toStdString().c_str());
    EXPECT_STREQ("1-8:1.1", m_deviceInput->m_BusInfo.toStdString().c_str());
    EXPECT_STREQ("keyboard", m_deviceInput->m_Description.toStdString().c_str());
    EXPECT_STREQ("usbhid", m_deviceInput->m_Driver.toStdString().c_str());
    EXPECT_STREQ("12 Mbps", m_deviceInput->m_Speed.toStdString().c_str());
    EXPECT_STREQ("usb@1:8", m_deviceInput->m_KeyToLshw.toStdString().c_str());
}

bool ut_isValueValid()
{
    return true;
}

bool ut_input_isDeviceExistInPairedDevice()
{
    return true;
}

TEST_F(UT_DeviceInput, UT_DeviceInput_setInfoFromBluetoothctl)
{
    Stub stub;
    stub.set(ADDR(DeviceInput, isValueValid), ut_isValueValid);
    stub.set(ADDR(DeviceManager, isDeviceExistInPairedDevice), ut_input_isDeviceExistInPairedDevice);
    m_deviceInput->setInfoFromBluetoothctl();

    EXPECT_STREQ("Bluetooth", m_deviceInput->m_Interface.toStdString().c_str());
}

TEST_F(UT_DeviceInput, UT_DeviceInput_name)
{
    QMap<QString, QString> map;
    setHwinfoMap(map);
    m_deviceInput->setInfoFromHwinfo(map);

    QString name = m_deviceInput->name();
    EXPECT_STREQ("MX board 8.0", name.toStdString().c_str());
}

TEST_F(UT_DeviceInput, UT_DeviceInput_driver)
{
    QMap<QString, QString> map;
    setHwinfoMap(map);
    m_deviceInput->setInfoFromHwinfo(map);

    QString driver = m_deviceInput->driver();
    EXPECT_STREQ("usbhid", driver.toStdString().c_str());
}

TEST_F(UT_DeviceInput, UT_DeviceInput_subTitle_001)
{
    QMap<QString, QString> map;
    setHwinfoMap(map);
    map.insert("Device", "");
    m_deviceInput->setInfoFromHwinfo(map);

    QString title = m_deviceInput->subTitle();
    EXPECT_STREQ("Cherry MX board 8.0", title.toStdString().c_str());
}

TEST_F(UT_DeviceInput, UT_DeviceInput_subTitle_002)
{
    QMap<QString, QString> map;
    setHwinfoMap(map);
    m_deviceInput->setInfoFromHwinfo(map);

    QString title = m_deviceInput->subTitle();
    EXPECT_STREQ("MX board 8.0", title.toStdString().c_str());
}

TEST_F(UT_DeviceInput, UT_DeviceInput_getOverviewInfo)
{
    QMap<QString, QString> map;
    setHwinfoMap(map);
    m_deviceInput->setInfoFromHwinfo(map);

    QString overview = m_deviceInput->getOverviewInfo();
    EXPECT_STREQ("MX board 8.0 (Cherry MX board 8.0)", overview.toStdString().c_str());
}

TEST_F(UT_DeviceInput, UT_DeviceInput_setEnable_001)
{
    m_deviceInput->m_SerialID = "";
    EnableDeviceStatus value = m_deviceInput->setEnable(true);
    EXPECT_EQ(EnableDeviceStatus::EDS_NoSerial, value);
}

bool ut_input_enable_true()
{
    return true;
}

TEST_F(UT_DeviceInput, UT_DeviceInput_setEnable_002)
{
    m_deviceInput->m_SerialID = "serial";
    m_deviceInput->m_UniqueID = "";
    m_deviceInput->m_SysPath = "";
    EnableDeviceStatus value = m_deviceInput->setEnable(true);
    EXPECT_EQ(EnableDeviceStatus::EDS_Faild, value);
}

TEST_F(UT_DeviceInput, UT_DeviceInput_setEnable_003)
{
    m_deviceInput->m_SerialID = "serial";
    m_deviceInput->m_UniqueID = "unique";
    m_deviceInput->m_SysPath = "unique";

    Stub stub;
    stub.set(ADDR(DBusEnableInterface, enable), ut_input_enable_true);

    EnableDeviceStatus value = m_deviceInput->setEnable(true);
    EXPECT_EQ(EnableDeviceStatus::EDS_Success, value);
    EXPECT_TRUE(m_deviceInput->m_Enable);
}

TEST_F(UT_DeviceInput, UT_DeviceInput_enable)
{
    m_deviceInput->setEnable(true);
    m_deviceInput->m_HardwareClass = "keyboard";

    EXPECT_TRUE(m_deviceInput->enable());
}

TEST_F(UT_DeviceInput, UT_DeviceInput_initFilterKey)
{
    m_deviceInput->initFilterKey();
    EXPECT_EQ(8, m_deviceInput->m_FilterKey.size());
}

TEST_F(UT_DeviceInput, UT_DeviceInput_loadBaseDeviceInfo)
{
    QMap<QString, QString> map;
    setHwinfoMap(map);
    m_deviceInput->setInfoFromHwinfo(map);

    m_deviceInput->loadBaseDeviceInfo();

    QPair<QString, QString> value0 = m_deviceInput->m_LstBaseInfo.at(0);
    EXPECT_STREQ("MX board 8.0", value0.second.toStdString().c_str());
    QPair<QString, QString> value1 = m_deviceInput->m_LstBaseInfo.at(1);
    EXPECT_STREQ("Cherry GmbH", value1.second.toStdString().c_str());
    QPair<QString, QString> value2 = m_deviceInput->m_LstBaseInfo.at(2);
    EXPECT_STREQ("Cherry MX board 8.0", value2.second.toStdString().c_str());
    QPair<QString, QString> value3 = m_deviceInput->m_LstBaseInfo.at(3);
    EXPECT_STREQ("USB", value3.second.toStdString().c_str());
    QPair<QString, QString> value4 = m_deviceInput->m_LstBaseInfo.at(4);
    EXPECT_STREQ("1-8:1.1", value4.second.toStdString().c_str());
}

TEST_F(UT_DeviceInput, UT_DeviceInput_loadOtherDeviceInfo)
{
    m_deviceInput->loadOtherDeviceInfo();
    EXPECT_EQ(0, m_deviceInput->m_LstOtherInfo.size());
}

TEST_F(UT_DeviceInput, UT_DeviceInput_loadTableHeader)
{
    m_deviceInput->loadTableHeader();
    EXPECT_EQ(3, m_deviceInput->m_TableHeader.size());
}

TEST_F(UT_DeviceInput, UT_DeviceInput_loadTableData_001)
{
    m_deviceInput->loadTableData();
    EXPECT_EQ(3, m_deviceInput->m_TableData.size());
}


TEST_F(UT_DeviceInput, UT_DeviceInput_loadTableData_002)
{
    m_deviceInput->m_Enable = false;
    m_deviceInput->loadTableData();
    EXPECT_EQ(3, m_deviceInput->m_TableData.size());
}
