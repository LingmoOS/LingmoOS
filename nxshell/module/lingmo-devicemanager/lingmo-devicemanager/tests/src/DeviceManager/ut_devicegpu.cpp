// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DeviceGpu.h"

#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class UT_DeviceGpu : public UT_HEAD
{
public:
    void SetUp()
    {
        m_deviceGpu = new DeviceGpu;
    }
    void TearDown()
    {
        delete m_deviceGpu;
    }
    DeviceGpu *m_deviceGpu;
};

void setLshwInfo(QMap<QString, QString> &mapLshw)
{
    mapLshw.insert("product", "GK208B [GeForce GT 730]");
    mapLshw.insert("vendor", "NVIDIA Corporation");
    mapLshw.insert("bus info", "pci@0000:01:00.0");
    mapLshw.insert("version", "a1");
    mapLshw.insert("width", "64 bits");
    mapLshw.insert("clock", "33MHz");
    mapLshw.insert("irq", "126");
    mapLshw.insert("capabilities", "pm msi pciexpress vga_controller bus_master cap_list rom");
    mapLshw.insert("description", "VGA compatible controller");
    mapLshw.insert("driver", "nouveau");
    mapLshw.insert("ioport", "e000(size=128)");
    mapLshw.insert("memory", "de000000-deffffff  d0000000-d7ffffff  d8000000-d9ffffff  c0000-dffff");
    mapLshw.insert("physical id", "0");
}

void setHwinfoInfo(QMap<QString, QString> &mapHwinfo)
{
    mapHwinfo.insert("Vendor", "nVidia Corporation");
    mapHwinfo.insert("Device", "GK208B [GeForce GT 730]");
    mapHwinfo.insert("Model", "nVidia GK208B [GeForce GT 730]");
    mapHwinfo.insert("Revision", "0xa1");
    mapHwinfo.insert("IRQ", "126 (681 events)");
    mapHwinfo.insert("Driver", "nouveau");
    mapHwinfo.insert("Width", "64 bits");
    mapHwinfo.insert("SysFS BusID", "0000:01:00.0");
    mapHwinfo.insert("SysFS ID", "/devices/pci0000:00/0000:00:01.0/0000:01:00.0");
}

void setGpuInfo(QMap<QString, QString> &mapGpu)
{
    mapGpu.insert("Name", "GK208B");
}

TEST_F(UT_DeviceGpu, UT_DeviceGpu_initFilterKey)
{
    m_deviceGpu->initFilterKey();
    EXPECT_EQ(13, m_deviceGpu->m_FilterKey.size());
}

TEST_F(UT_DeviceGpu, UT_DeviceGpu_loadBaseDeviceInfo)
{
    QMap<QString, QString> mapinfo;
    setHwinfoInfo(mapinfo);
    m_deviceGpu->setHwinfoInfo(mapinfo);
    QMap<QString, QString> DmesgInfoMap;
    DmesgInfoMap.insert("Size","0000:01:00.0=2GB");
    m_deviceGpu->setDmesgInfo(DmesgInfoMap);
    m_deviceGpu->loadBaseDeviceInfo();

    QPair<QString, QString> value0 = m_deviceGpu->m_LstBaseInfo.at(0);
    EXPECT_STREQ("GK208B [GeForce GT 730]", value0.second.toStdString().c_str());
    QPair<QString, QString> value1 = m_deviceGpu->m_LstBaseInfo.at(1);
    EXPECT_STREQ("nVidia Corporation", value1.second.toStdString().c_str());
    QPair<QString, QString> value2 = m_deviceGpu->m_LstBaseInfo.at(2);
    EXPECT_STREQ("nVidia GK208B [GeForce GT 730]", value2.second.toStdString().c_str());
    QPair<QString, QString> value3 = m_deviceGpu->m_LstBaseInfo.at(3);
    EXPECT_STREQ("0xa1", value3.second.toStdString().c_str());
    QPair<QString, QString> value4 = m_deviceGpu->m_LstBaseInfo.at(4);
    EXPECT_STREQ("2GB", value4.second.toStdString().c_str());
}

TEST_F(UT_DeviceGpu, UT_DeviceGpu_setLshwInfo_001)
{
    QMap<QString, QString> mapinfo;
    setLshwInfo(mapinfo);
    m_deviceGpu->m_HwinfoToLshw = "0000:01:00.0";

    m_deviceGpu->setLshwInfo(mapinfo);
    EXPECT_STREQ("GK208B [GeForce GT 730]", m_deviceGpu->m_Name.toStdString().c_str());
    EXPECT_STREQ("NVIDIA Corporation", m_deviceGpu->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("a1", m_deviceGpu->m_Version.toStdString().c_str());
    EXPECT_STREQ("64 bits", m_deviceGpu->m_Width.toStdString().c_str());
    EXPECT_STREQ("33MHz", m_deviceGpu->m_Clock.toStdString().c_str());
    EXPECT_STREQ("126", m_deviceGpu->m_IRQ.toStdString().c_str());
    EXPECT_STREQ("pm msi pciexpress vga_controller bus_master cap_list rom", m_deviceGpu->m_Capabilities.toStdString().c_str());
    EXPECT_STREQ("VGA compatible controller", m_deviceGpu->m_Description.toStdString().c_str());
    EXPECT_STREQ("nouveau", m_deviceGpu->m_Driver.toStdString().c_str());
    EXPECT_STREQ("pci@0000:01:00.0", m_deviceGpu->m_BusInfo.toStdString().c_str());
    EXPECT_STREQ("e000(size=128)", m_deviceGpu->m_IOPort.toStdString().c_str());
    EXPECT_STREQ("de000000-deffffff  d0000000-d7ffffff  d8000000-d9ffffff  c0000-dffff", m_deviceGpu->m_MemAddress.toStdString().c_str());
    EXPECT_STREQ("0", m_deviceGpu->m_PhysID.toStdString().c_str());
}

TEST_F(UT_DeviceGpu, UT_DeviceGpu_setLshwInfo_002)
{
    QMap<QString, QString> mapinfo;
    setLshwInfo(mapinfo);
    m_deviceGpu->m_HwinfoToLshw = "02:00.0";

    m_deviceGpu->setLshwInfo(mapinfo);
    EXPECT_STREQ("", m_deviceGpu->m_Name.toStdString().c_str());
    EXPECT_STREQ("", m_deviceGpu->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("", m_deviceGpu->m_Version.toStdString().c_str());
    EXPECT_STREQ("", m_deviceGpu->m_Width.toStdString().c_str());
    EXPECT_STREQ("", m_deviceGpu->m_Clock.toStdString().c_str());
    EXPECT_STREQ("", m_deviceGpu->m_IRQ.toStdString().c_str());
    EXPECT_STREQ("", m_deviceGpu->m_Capabilities.toStdString().c_str());
    EXPECT_STREQ("", m_deviceGpu->m_Description.toStdString().c_str());
    EXPECT_STREQ("", m_deviceGpu->m_Driver.toStdString().c_str());
    EXPECT_STREQ("", m_deviceGpu->m_BusInfo.toStdString().c_str());
    EXPECT_STREQ("", m_deviceGpu->m_IOPort.toStdString().c_str());
    EXPECT_STREQ("", m_deviceGpu->m_MemAddress.toStdString().c_str());
    EXPECT_STREQ("", m_deviceGpu->m_PhysID.toStdString().c_str());
}

TEST_F(UT_DeviceGpu, UT_DeviceGpu_setHwinfoInfo)
{
    QMap<QString, QString> mapinfo;
    setHwinfoInfo(mapinfo);

    m_deviceGpu->setHwinfoInfo(mapinfo);

    EXPECT_STREQ("nVidia Corporation", m_deviceGpu->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("GK208B [GeForce GT 730]", m_deviceGpu->m_Name.toStdString().c_str());
    EXPECT_STREQ("nVidia GK208B [GeForce GT 730]", m_deviceGpu->m_Model.toStdString().c_str());
    EXPECT_STREQ("0xa1", m_deviceGpu->m_Version.toStdString().c_str());
    EXPECT_STREQ("126 (681 events)", m_deviceGpu->m_IRQ.toStdString().c_str());
    EXPECT_STREQ("nouveau", m_deviceGpu->m_Driver.toStdString().c_str());
    EXPECT_STREQ("64 bits", m_deviceGpu->m_Width.toStdString().c_str());
    EXPECT_STREQ("0000:01:00.0", m_deviceGpu->m_HwinfoToLshw.toStdString().c_str());
}

TEST_F(UT_DeviceGpu, UT_DeviceGpu_setDmesgInfo_001)
{
    m_deviceGpu->m_HwinfoToLshw = "01:00.0";
    QMap<QString, QString> DmesgInfoMap;
    DmesgInfoMap.insert("Size","01:00.0=2GB");
    m_deviceGpu->setDmesgInfo(DmesgInfoMap);
    EXPECT_STREQ("2GB", m_deviceGpu->m_GraphicsMemory.toStdString().c_str());
}

TEST_F(UT_DeviceGpu, UT_DeviceGpu_setDmesgInfo_002)
{
    m_deviceGpu->m_HwinfoToLshw = "01:00.0";
    QMap<QString, QString> DmesgInfoMap;
    DmesgInfoMap.insert("Size","null=3GB");
    m_deviceGpu->setDmesgInfo(DmesgInfoMap);
    EXPECT_STREQ("3GB", m_deviceGpu->m_GraphicsMemory.toStdString().c_str());

}

TEST_F(UT_DeviceGpu, UT_DeviceGpu_setGpuInfo)
{
    QMap<QString, QString> mapinfo;
    setGpuInfo(mapinfo);

    m_deviceGpu->setGpuInfo(mapinfo);
    EXPECT_STREQ("", m_deviceGpu->m_HDMI.toStdString().c_str());
    EXPECT_STREQ("", m_deviceGpu->m_VGA.toStdString().c_str());
    EXPECT_STREQ("", m_deviceGpu->m_DisplayPort.toStdString().c_str());
    EXPECT_STREQ("", m_deviceGpu->m_eDP.toStdString().c_str());
    EXPECT_STREQ("", m_deviceGpu->m_DVI.toStdString().c_str());
    EXPECT_STREQ("GK208B", m_deviceGpu->m_Name.toStdString().c_str());
}

TEST_F(UT_DeviceGpu, UT_DeviceGpu_name)
{
    QMap<QString, QString> mapinfo;
    setHwinfoInfo(mapinfo);

    m_deviceGpu->setHwinfoInfo(mapinfo);
    QString name = m_deviceGpu->name();
    EXPECT_STREQ("GK208B [GeForce GT 730]", name.toStdString().c_str());
}
TEST_F(UT_DeviceGpu, UT_DeviceGpu_driver)
{
    QMap<QString, QString> mapinfo;
    setHwinfoInfo(mapinfo);

    m_deviceGpu->setHwinfoInfo(mapinfo);

    QString driver = m_deviceGpu->driver();
    EXPECT_STREQ("nouveau", driver.toStdString().c_str());

}
TEST_F(UT_DeviceGpu, UT_DeviceGpu_subTitle)
{
    QMap<QString, QString> mapinfo;
    setHwinfoInfo(mapinfo);

    m_deviceGpu->setHwinfoInfo(mapinfo);
    QString title = m_deviceGpu->subTitle();
    EXPECT_STREQ("nVidia GK208B [GeForce GT 730]", title.toStdString().c_str());
}

TEST_F(UT_DeviceGpu, UT_DeviceGpu_getOverviewInfo)
{
    QMap<QString, QString> mapinfo;
    setHwinfoInfo(mapinfo);

    m_deviceGpu->setHwinfoInfo(mapinfo);
    QString overview = m_deviceGpu->getOverviewInfo();
    EXPECT_STREQ("GK208B [GeForce GT 730]", overview.toStdString().c_str());
}

TEST_F(UT_DeviceGpu, UT_DeviceGpu_loadOtherDeviceInfo)
{
    m_deviceGpu->loadOtherDeviceInfo();
    EXPECT_EQ(6, m_deviceGpu->m_LstOtherInfo.size());
}

TEST_F(UT_DeviceGpu, UT_DeviceGpu_loadTableHeader)
{
    m_deviceGpu->loadTableHeader();
    EXPECT_EQ(3, m_deviceGpu->m_TableHeader.size());
}

TEST_F(UT_DeviceGpu, UT_DeviceGpu_loadTableData)
{
    m_deviceGpu->loadTableData();
    EXPECT_EQ(3, m_deviceGpu->m_TableData.size());
}

void ut_gpu_setxrandrmap(QMap<QString, QString> &mapInfo)
{
    mapInfo.insert("minResolution", "minResolution");
    mapInfo.insert("curResolution", "curResolution");
    mapInfo.insert("maxResolution", "maxResolution");
    mapInfo.insert("HDMI", "Unable");
    mapInfo.insert("VGA", "Unable");
    mapInfo.insert("DP", "Unable");
    mapInfo.insert("eDP", "Unable");
    mapInfo.insert("DVI", "Unable");
    mapInfo.insert("DigitalOutput", "Unable");
}

TEST_F(UT_DeviceGpu, UT_DeviceGpu_setXrandrInfo)
{
    QMap<QString, QString> mapInfo;
    ut_gpu_setxrandrmap(mapInfo);

    m_deviceGpu->setXrandrInfo(mapInfo);
    EXPECT_STREQ("minResolution", m_deviceGpu->m_MinimumResolution.toStdString().c_str());
    EXPECT_STREQ("curResolution", m_deviceGpu->m_CurrentResolution.toStdString().c_str());
    EXPECT_STREQ("maxResolution", m_deviceGpu->m_MaximumResolution.toStdString().c_str());
    EXPECT_STREQ("Unable", m_deviceGpu->m_HDMI.toStdString().c_str());
    EXPECT_STREQ("Unable", m_deviceGpu->m_VGA.toStdString().c_str());
    EXPECT_STREQ("Unable", m_deviceGpu->m_DisplayPort.toStdString().c_str());
    EXPECT_STREQ("Unable", m_deviceGpu->m_eDP.toStdString().c_str());
    EXPECT_STREQ("Unable", m_deviceGpu->m_DVI.toStdString().c_str());
}
