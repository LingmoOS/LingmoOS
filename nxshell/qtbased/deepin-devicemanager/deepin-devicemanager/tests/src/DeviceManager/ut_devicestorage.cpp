// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DeviceStorage.h"

#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class UT_DeviceStorage : public UT_HEAD
{
public:
    void SetUp()
    {
        m_deviceStorage = new DeviceStorage;
    }
    void TearDown()
    {
        delete m_deviceStorage;
    }
    DeviceStorage *m_deviceStorage;
};

void ut_storage_setHwinfoInfo(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("SysFS BusID", "nvme0n12:0:0:0");
    mapinfo.insert("Model", "ST240BX500SSD1");
    mapinfo.insert("Vendor", "");
    mapinfo.insert("Driver", "ahci, sd");
    mapinfo.insert("Attached to", "#2 (SATA controller)");
    mapinfo.insert("Revision", "R013");
    mapinfo.insert("Hardware Class", "disk");
    mapinfo.insert("Capacity", "223 GB (240057409536 bytes)");
    mapinfo.insert("Serial ID", "2002E3E0B393");
    mapinfo.insert("Device File", "/dev/nvme0n1");
    mapinfo.insert("SysFS Device Link", "/devices/pci0000:00/0000:00:17.0/ata3/host2/target2:0:0/2:0:0:0");
}

void ut_storage_setLshwInfo(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("bus info", "scsi@nvme0n12:0.0.0");
    mapinfo.insert("interface", "sata");
    mapinfo.insert("capabilities", "gpt-1.00 partitioned partitioned:gpt");
    mapinfo.insert("version", "R013");
    mapinfo.insert("serial", "2002E3E0B393");
    mapinfo.insert("product", "CT240BX500SSD1");
    mapinfo.insert("description", "ATA Disk");
    mapinfo.insert("size", "223GiB (240GB)");
}

void ut_storage_setSmartCtlInfo_001(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("Firmware Version", "M6CR013");
    mapinfo.insert("SATA Version is", "SATA 3.2, 6.0 Gb/s (current: 6.0 Gb/s)");
    mapinfo.insert("Rotation Rate", "Solid State Device");
    mapinfo.insert("Power_On_Hours", "3717");
    mapinfo.insert("Power_Cycle_Count", "306");
    mapinfo.insert("User Capacity", "240,057,409,536 bytes [240 GB]");
    mapinfo.insert("Device Model", "CT240BX500SSD1");
    mapinfo.insert("Serial Number", "2002E3E0B393");
}

void ut_storage_setSmartCtlInfo_002(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("Firmware Version", "M6CR013");
    mapinfo.insert("SATA Version is", "SATA 3.2, 6.0 Gb/s (current: 6.0 Gb/s)");
    mapinfo.insert("Rotation Rate", "Solid State Device");
    mapinfo.insert("Power On Hours", "3717");
    mapinfo.insert("Power Cycles", "306");
    mapinfo.insert("Total NVM Capacity", "240,057,409,536 bytes [240 GB]");
    mapinfo.insert("Namespace 1 Size/Capacity", "240,057,409,536 bytes [240 GB]");
    mapinfo.insert("Model Number", "CT240BX500SSD1");
    mapinfo.insert("Serial Number", "2002E3E0B393");
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_setHwinfoInfo_001)
{
    QMap<QString, QString> mapinfo;
    ut_storage_setHwinfoInfo(mapinfo);
    mapinfo.remove("SysFS BusID");

    ASSERT_FALSE(m_deviceStorage->setHwinfoInfo(mapinfo));
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_setHwinfoInfo_002)
{
    QMap<QString, QString> mapinfo;
    ut_storage_setHwinfoInfo(mapinfo);

    ASSERT_TRUE(m_deviceStorage->setHwinfoInfo(mapinfo));
    EXPECT_STREQ("ST240BX500SSD1", m_deviceStorage->m_Model.toStdString().c_str());
    EXPECT_STREQ("ST", m_deviceStorage->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("ahci, sd", m_deviceStorage->m_Driver.toStdString().c_str());
    EXPECT_STREQ("SATA ", m_deviceStorage->m_Interface.toStdString().c_str());
    EXPECT_STREQ("R013", m_deviceStorage->m_Version.toStdString().c_str());
    EXPECT_STREQ("disk", m_deviceStorage->m_Description.toStdString().c_str());
    EXPECT_STREQ("223GB", m_deviceStorage->m_Size.toStdString().c_str());
    EXPECT_STREQ("2002E3E0B393", m_deviceStorage->m_SerialNumber.toStdString().c_str());
    EXPECT_STREQ("/dev/nvme0n1", m_deviceStorage->m_DeviceFile.toStdString().c_str());
    EXPECT_STREQ("nvme0n12:0:0:0", m_deviceStorage->m_KeyToLshw.toStdString().c_str());
    EXPECT_STREQ("/devices/pci0000:00/0000:00:17.0/ata3/host2/target2:0:0/2:0:0:0", m_deviceStorage->m_NvmeKey.toStdString().c_str());
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_setHwinfoInfo_003)
{
    QMap<QString, QString> mapinfo;
    ut_storage_setHwinfoInfo(mapinfo);
    mapinfo.insert("Vendor", "pci 0x2A1f");
    mapinfo.remove("Serial ID");
    mapinfo.remove("Capacity");

    ASSERT_FALSE(m_deviceStorage->setHwinfoInfo(mapinfo));
    EXPECT_STREQ("ST240BX500SSD1", m_deviceStorage->m_Model.toStdString().c_str());
    EXPECT_STREQ("", m_deviceStorage->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("ahci, sd", m_deviceStorage->m_Driver.toStdString().c_str());
    EXPECT_STREQ("SATA ", m_deviceStorage->m_Interface.toStdString().c_str());
    EXPECT_STREQ("R013", m_deviceStorage->m_Version.toStdString().c_str());
    EXPECT_STREQ("disk", m_deviceStorage->m_Description.toStdString().c_str());
    EXPECT_STREQ("", m_deviceStorage->m_Size.toStdString().c_str());
    EXPECT_STREQ("", m_deviceStorage->m_SerialNumber.toStdString().c_str());
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_setKLUHwinfoInfo_001)
{
    QMap<QString, QString> mapinfo;
    ut_storage_setHwinfoInfo(mapinfo);
    mapinfo.insert("Driver", "usb-storage");
    mapinfo.insert("Vendor", "PASON");

    ASSERT_TRUE(m_deviceStorage->setKLUHwinfoInfo(mapinfo));
    EXPECT_STREQ("ST240BX500SSD1", m_deviceStorage->m_Model.toStdString().c_str());
    EXPECT_STREQ("PASON", m_deviceStorage->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("usb-storage", m_deviceStorage->m_Driver.toStdString().c_str());
    EXPECT_STREQ("SATA ", m_deviceStorage->m_Interface.toStdString().c_str());
    EXPECT_STREQ("R013", m_deviceStorage->m_Version.toStdString().c_str());
    EXPECT_STREQ("disk", m_deviceStorage->m_Description.toStdString().c_str());
    EXPECT_STREQ("223GB", m_deviceStorage->m_Size.toStdString().c_str());
    EXPECT_STREQ("2002E3E0B393", m_deviceStorage->m_SerialNumber.toStdString().c_str());
    EXPECT_STREQ("/dev/nvme0n1", m_deviceStorage->m_DeviceFile.toStdString().c_str());
    EXPECT_STREQ("nvme0n12:0:0:0", m_deviceStorage->m_KeyToLshw.toStdString().c_str());
    EXPECT_STREQ("USB", m_deviceStorage->m_MediaType.toStdString().c_str());
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_setKLUHwinfoInfo_002)
{
    QMap<QString, QString> mapinfo;
    ut_storage_setHwinfoInfo(mapinfo);
    mapinfo.remove("SysFS BusID");

    ASSERT_FALSE(m_deviceStorage->setKLUHwinfoInfo(mapinfo));
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_setKLUHwinfoInfo_003)
{
    QMap<QString, QString> mapinfo;
    ut_storage_setHwinfoInfo(mapinfo);
    mapinfo.insert("Vendor", "PASON");
    mapinfo.remove("Capacity");

    ASSERT_FALSE(m_deviceStorage->setKLUHwinfoInfo(mapinfo));
    EXPECT_STREQ("ST240BX500SSD1", m_deviceStorage->m_Model.toStdString().c_str());
    EXPECT_STREQ("PASON", m_deviceStorage->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("ahci, sd", m_deviceStorage->m_Driver.toStdString().c_str());
    EXPECT_STREQ("SATA ", m_deviceStorage->m_Interface.toStdString().c_str());
    EXPECT_STREQ("R013", m_deviceStorage->m_Version.toStdString().c_str());
    EXPECT_STREQ("disk", m_deviceStorage->m_Description.toStdString().c_str());
    EXPECT_STREQ("", m_deviceStorage->m_Size.toStdString().c_str());
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_addInfoFromlshw_001)
{
    QMap<QString, QString> mapinfo;
    ut_storage_setLshwInfo(mapinfo);
    mapinfo.remove("bus info");

    ASSERT_FALSE(m_deviceStorage->addInfoFromlshw(mapinfo));
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_addInfoFromlshw_002)
{
    QMap<QString, QString> mapinfo;
    ut_storage_setLshwInfo(mapinfo);
    mapinfo.insert("bus info", "scsi@2:0.0.0");
    m_deviceStorage->m_KeyToLshw = "nvme0n1";
    ASSERT_FALSE(m_deviceStorage->addInfoFromlshw(mapinfo));
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_addInfoFromlshw_003)
{
    QMap<QString, QString> mapinfo;
    ut_storage_setLshwInfo(mapinfo);
    mapinfo.insert("bus info", "scsi@2:0.0.0");
    m_deviceStorage->m_KeyToLshw = "2:0:0:0";

    ASSERT_TRUE(m_deviceStorage->addInfoFromlshw(mapinfo));
    EXPECT_STREQ("scsi2", m_deviceStorage->m_KeyFromStorage.toStdString().c_str());
    EXPECT_STREQ("sata", m_deviceStorage->m_Interface.toStdString().c_str());
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_addNVMEInfoFromlshw_001)
{
    QMap<QString, QString> mapinfo;
    ut_storage_setLshwInfo(mapinfo);
    mapinfo.remove("bus info");

    ASSERT_FALSE(m_deviceStorage->addNVMEInfoFromlshw(mapinfo));
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_addNVMEInfoFromlshw_002)
{
    QMap<QString, QString> mapinfo;
    ut_storage_setLshwInfo(mapinfo);
    mapinfo.insert("bus info", "scsi@2:0.0.0");
    mapinfo.insert("vendor", "PASON");
    m_deviceStorage->m_NvmeKey = "/devices/pci0000:00/0000:00:17.0/ata3/host2/target2:0:0/2:0.0.0";

    ASSERT_TRUE(m_deviceStorage->addNVMEInfoFromlshw(mapinfo));
    EXPECT_STREQ("PASON", m_deviceStorage->m_Vendor.toStdString().c_str());
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_addNVMEInfoFromlshw_003)
{
    QMap<QString, QString> mapinfo;
    ut_storage_setLshwInfo(mapinfo);
    mapinfo.insert("bus info", "scsi@nvme0n12:0.0.0");
    m_deviceStorage->m_NvmeKey = "/devices/pci0000:00/0000:00:17.0/ata3/host2/target2:0:0/2:0:0:0";

    ASSERT_FALSE(m_deviceStorage->addNVMEInfoFromlshw(mapinfo));
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_addInfoFromSmartctl_001)
{
    QMap<QString, QString> mapinfo;
    m_deviceStorage->m_DeviceFile = "/dev/nvme0n1";
    EXPECT_TRUE(m_deviceStorage->addInfoFromSmartctl("nvme0n1", mapinfo));
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_addInfoFromSmartctl_002)
{
    QMap<QString, QString> mapinfo;
    m_deviceStorage->m_DeviceFile = "/dev/sda";
    EXPECT_FALSE(m_deviceStorage->addInfoFromSmartctl("nvme0n1", mapinfo));
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_setMediaType_001)
{
    m_deviceStorage->m_DeviceFile = "/dev/sda";
    EXPECT_FALSE(m_deviceStorage->setMediaType("nvme0n1", "0"));
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_setMediaType_002)
{
    m_deviceStorage->m_DeviceFile = "/dev/sda";
    EXPECT_TRUE(m_deviceStorage->setMediaType("sda", "0"));
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_setMediaType_003)
{
    m_deviceStorage->m_DeviceFile = "/dev/sda";
    EXPECT_TRUE(m_deviceStorage->setMediaType("sda", "1"));
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_setMediaType_004)
{
    m_deviceStorage->m_DeviceFile = "/dev/sda";
    EXPECT_TRUE(m_deviceStorage->setMediaType("sda", "2"));
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_setKLUMediaType_001)
{
    m_deviceStorage->m_DeviceFile = "/dev/sda";
    EXPECT_FALSE(m_deviceStorage->setKLUMediaType("nvme0n1", "0"));
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_setKLUMediaType_002)
{
    m_deviceStorage->m_DeviceFile = "/dev/sda";
    EXPECT_TRUE(m_deviceStorage->setKLUMediaType("sda", "0"));
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_setKLUMediaType_003)
{
    m_deviceStorage->m_DeviceFile = "/dev/sda";
    EXPECT_TRUE(m_deviceStorage->setKLUMediaType("sda", "1"));
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_setKLUMediaType_004)
{
    m_deviceStorage->m_DeviceFile = "/dev/sda";
    EXPECT_TRUE(m_deviceStorage->setKLUMediaType("sda", "2"));
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_setKLUMediaType_005)
{
    m_deviceStorage->m_DeviceFile = "/dev/sda";
    m_deviceStorage->m_MediaType = "USB";
    EXPECT_TRUE(m_deviceStorage->setKLUMediaType("sda", "2"));
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_isValid_001)
{
    m_deviceStorage->m_Size = "32GB";
    EXPECT_TRUE(m_deviceStorage->isValid());
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_isValid_002)
{
    EXPECT_FALSE(m_deviceStorage->isValid());
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_setDiskSerialID_001)
{
    QString deviceFiles = "/dev/sda, /dev/disk/by-path/pci-0000:00:17.0-ata-3, /dev/disk/by-id/ata-CT240BX500SSD1_2002E3E0B393";
    m_deviceStorage->m_SerialNumber = "2002E3E0B393";
    m_deviceStorage->setDiskSerialID(deviceFiles);

    EXPECT_STREQ("2002E3E0B393", m_deviceStorage->m_SerialNumber.toStdString().c_str());
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_setDiskSerialID_002)
{
    QString deviceFiles = "/dev/sda, /dev/disk/by-path/pci-0000:00:17.0-ata-3, /dev/disk/by-id/ata-CT240BX500SSD1_2002E3E0B393";
    m_deviceStorage->m_SerialNumber = "XXX2002E3E0B393";
    m_deviceStorage->setDiskSerialID(deviceFiles);

    EXPECT_STREQ("2002E3E0B393", m_deviceStorage->m_SerialNumber.toStdString().c_str());
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_compareSize_001)
{
    QString size1 = "";
    QString size2 = "32GB";
    EXPECT_EQ(m_deviceStorage->compareSize(size1, size2), "32GB");
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_compareSize_002)
{
    QString size1 = "16GB";
    QString size2 = "32GB";
    EXPECT_EQ(m_deviceStorage->compareSize(size1, size2), "32GB");
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_compareSize_003)
{
    QString size1 = "64GB";
    QString size2 = "32GB";
    EXPECT_EQ(m_deviceStorage->compareSize(size1, size2), "64GB");
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_name)
{
    QMap<QString, QString> mapinfo;
    ut_storage_setHwinfoInfo(mapinfo);
    m_deviceStorage->setHwinfoInfo(mapinfo);

    QString name = m_deviceStorage->name();
    EXPECT_STREQ("ST240BX500SSD1", name.toStdString().c_str());
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_driver)
{
    QMap<QString, QString> mapinfo;
    ut_storage_setHwinfoInfo(mapinfo);
    m_deviceStorage->setHwinfoInfo(mapinfo);

    QString drivier = m_deviceStorage->driver();
    EXPECT_STREQ("ahci, sd", drivier.toStdString().c_str());

}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_initFilterKey)
{
    m_deviceStorage->initFilterKey();
    EXPECT_EQ(12, m_deviceStorage->m_FilterKey.size());
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_keyFromStorage)
{
    QMap<QString, QString> mapinfo;
    ut_storage_setLshwInfo(mapinfo);
    mapinfo.insert("bus info", "scsi@2:0.0.0");
    m_deviceStorage->m_KeyToLshw = "2:0:0:0";
    m_deviceStorage->addInfoFromlshw(mapinfo);

    QString key = m_deviceStorage->keyFromStorage();
    EXPECT_STREQ("scsi2", key.toStdString().c_str());
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_subTitle)
{
    QMap<QString, QString> mapinfo;
    ut_storage_setHwinfoInfo(mapinfo);
    m_deviceStorage->setHwinfoInfo(mapinfo);

    QString title = m_deviceStorage->subTitle();
    EXPECT_STREQ("ST240BX500SSD1", title.toStdString().c_str());
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_getOverviewInfo)
{
    QMap<QString, QString> mapinfo;
    ut_storage_setHwinfoInfo(mapinfo);
    m_deviceStorage->setHwinfoInfo(mapinfo);

    QString overview = m_deviceStorage->getOverviewInfo();
    EXPECT_STREQ("ST240BX500SSD1 (223GB)", overview.toStdString().c_str());
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_loadTableHeader)
{
    m_deviceStorage->loadTableHeader();
    EXPECT_EQ(4, m_deviceStorage->m_TableHeader.size());
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_loadTableData)
{

    m_deviceStorage->loadTableData();
    EXPECT_EQ(4, m_deviceStorage->m_TableData.size());
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_loadBaseDeviceInfo)
{
    m_deviceStorage->loadBaseDeviceInfo();
    EXPECT_EQ(0, m_deviceStorage->m_LstBaseInfo.size());

}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_loadOtherDeviceInfo)
{
    m_deviceStorage->m_RotationRate = "Solid State Device";
    m_deviceStorage->loadOtherDeviceInfo();
    EXPECT_EQ(1, m_deviceStorage->m_LstOtherInfo.size());
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_getInfoFromLshw)
{
    QMap<QString, QString> mapinfo;
    ut_storage_setLshwInfo(mapinfo);

    m_deviceStorage->getInfoFromLshw(mapinfo);
    EXPECT_STREQ("gpt-1.00 partitioned partitioned:gpt", m_deviceStorage->m_Capabilities.toStdString().c_str());
    EXPECT_STREQ("R013", m_deviceStorage->m_Version.toStdString().c_str());
    EXPECT_STREQ("2002E3E0B393", m_deviceStorage->m_SerialNumber.toStdString().c_str());
    EXPECT_STREQ("CT240BX500SSD1", m_deviceStorage->m_Model.toStdString().c_str());
    EXPECT_STREQ("ATA Disk", m_deviceStorage->m_Description.toStdString().c_str());
    EXPECT_STREQ("240GB", m_deviceStorage->m_Size.toStdString().c_str());
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_getInfoFromsmartctl_001)
{
    QMap<QString, QString> mapinfo;
    ut_storage_setSmartCtlInfo_001(mapinfo);

    m_deviceStorage->getInfoFromsmartctl(mapinfo);
    EXPECT_STREQ("M6CR013", m_deviceStorage->m_FirmwareVersion.toStdString().c_str());
    EXPECT_STREQ(" 6.0 Gb/s (current: 6.0 Gb/s)", m_deviceStorage->m_Speed.toStdString().c_str());
    EXPECT_STREQ("Solid State Device", m_deviceStorage->m_RotationRate.toStdString().c_str());
    EXPECT_STREQ("SSD", m_deviceStorage->m_MediaType.toStdString().c_str());
    EXPECT_STREQ("240 GB", m_deviceStorage->m_Size.toStdString().c_str());
    EXPECT_STREQ("CT240BX500SSD1", m_deviceStorage->m_Model.toStdString().c_str());
    EXPECT_STREQ("2002E3E0B393", m_deviceStorage->m_SerialNumber.toStdString().c_str());
}

TEST_F(UT_DeviceStorage, UT_DeviceStorage_getInfoFromsmartctl_002)
{
    QMap<QString, QString> mapinfo;
    ut_storage_setSmartCtlInfo_002(mapinfo);

    m_deviceStorage->getInfoFromsmartctl(mapinfo);
    EXPECT_STREQ("M6CR013", m_deviceStorage->m_FirmwareVersion.toStdString().c_str());
    EXPECT_STREQ(" 6.0 Gb/s (current: 6.0 Gb/s)", m_deviceStorage->m_Speed.toStdString().c_str());
    EXPECT_STREQ("Solid State Device", m_deviceStorage->m_RotationRate.toStdString().c_str());
    EXPECT_STREQ("SSD", m_deviceStorage->m_MediaType.toStdString().c_str());
    EXPECT_STREQ("240 GB", m_deviceStorage->m_Size.toStdString().c_str());
    EXPECT_STREQ("CT240BX500SSD1", m_deviceStorage->m_Model.toStdString().c_str());
    EXPECT_STREQ("2002E3E0B393", m_deviceStorage->m_SerialNumber.toStdString().c_str());
}
