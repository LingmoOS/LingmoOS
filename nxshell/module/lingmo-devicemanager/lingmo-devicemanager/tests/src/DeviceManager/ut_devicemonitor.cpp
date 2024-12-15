// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DeviceMonitor.h"
#include "DeviceBios.h"
#include "EDIDParser.h"

#include "ut_Head.h"
#include "stub.h"

#include <DApplication>

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

DWIDGET_USE_NAMESPACE

class UT_DeviceMonitor : public UT_HEAD
{
public:
    void SetUp()
    {
        m_deviceMonitor = new DeviceMonitor;
    }
    void TearDown()
    {
        delete m_deviceMonitor;
    }
    DeviceMonitor *m_deviceMonitor;
};

TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_parseMonitorSize_001)
{
    QString sizeDescription = "527x296 mm";
    double inch = 0;
    QSize size = QSize(0, 0);

    QString ret = m_deviceMonitor->parseMonitorSize(sizeDescription, inch, size);
    EXPECT_STREQ("23.8 inch (527x296 mm)", ret.toStdString().c_str());
    EXPECT_EQ(527, m_deviceMonitor->m_Width);
    EXPECT_EQ(296, m_deviceMonitor->m_Height);
    EXPECT_STREQ("23.8", QString::number(inch, 10, 1).toStdString().c_str());
    EXPECT_EQ(QSize(527, 296), size);
}

TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_parseMonitorSize_002)
{
    QString sizeDescription = "527mm x 296mm";
    double inch = 0;
    QSize size = QSize(0, 0);

    QString ret = m_deviceMonitor->parseMonitorSize(sizeDescription, inch, size);
    EXPECT_STREQ("23.8 inch (527mm x 296mm)", ret.toStdString().c_str());
    EXPECT_EQ(527, m_deviceMonitor->m_Width);
    EXPECT_EQ(296, m_deviceMonitor->m_Height);
    EXPECT_STREQ("23.8", QString::number(inch, 10, 1).toStdString().c_str());
    EXPECT_EQ(QSize(527, 296), size);
}

void ut_monitor_sethwinfomap(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("Model", "VIEWSONIC VA2430-FHD");
    mapinfo.insert("Vendor", "VIEWSONIC");
    mapinfo.insert("Size", "527x296 mm");
    mapinfo.insert("Resolution", "720x400@70Hz 640x480@60Hz 640x480@67Hz");
    mapinfo.insert("Serial ID", "VSQ201321294");
    mapinfo.insert("Year of Manufacture", "2020");
    mapinfo.insert("Week of Manufacture", "13");
}

TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_setInfoFromHwinfo)
{
    QMap<QString, QString> mapinfo;
    ut_monitor_sethwinfomap(mapinfo);

    m_deviceMonitor->setInfoFromHwinfo(mapinfo);
    EXPECT_STREQ("VIEWSONIC VA2430-FHD", m_deviceMonitor->m_Name.toStdString().c_str());
    EXPECT_STREQ("VIEWSONIC", m_deviceMonitor->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("VIEWSONIC VA2430-FHD", m_deviceMonitor->m_Model.toStdString().c_str());
    EXPECT_STREQ("720x400@70Hz, 640x480@60Hz, 640x480@67Hz", m_deviceMonitor->m_SupportResolution.toStdString().c_str());
    EXPECT_STREQ("23.8 inch (527x296 mm)", m_deviceMonitor->m_ScreenSize.toStdString().c_str());
    EXPECT_STREQ("VSQ201321294", m_deviceMonitor->m_SerialNumber.toStdString().c_str());
    EXPECT_STREQ("2020-03", m_deviceMonitor->m_ProductionWeek.toStdString().c_str());
}

void ut_monitor_setselfmap(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("Name", "Name");
    mapinfo.insert("Vendor", "Vendor");
    mapinfo.insert("CurResolution", "CurResolution");
    mapinfo.insert("SupportResolution", "SupportResolution");
    mapinfo.insert("Size", "Size");
    mapinfo.insert("Date", "Date");
}

TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_setInfoFromSelfDefine)
{
    QMap<QString, QString> mapinfo;
    ut_monitor_setselfmap(mapinfo);

    m_deviceMonitor->setInfoFromSelfDefine(mapinfo);
    EXPECT_STREQ("Name", m_deviceMonitor->m_Name.toStdString().c_str());
    EXPECT_STREQ("Vendor", m_deviceMonitor->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("CurResolution", m_deviceMonitor->m_CurrentResolution.toStdString().c_str());
    EXPECT_STREQ("SupportResolution", m_deviceMonitor->m_SupportResolution.toStdString().c_str());
    EXPECT_STREQ("Size", m_deviceMonitor->m_ScreenSize.toStdString().c_str());
    EXPECT_STREQ("Date", m_deviceMonitor->m_ProductionWeek.toStdString().c_str());

}

void ut_monitor_setedidmap(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("Vendor", "Vendor");
    mapinfo.insert("Size", "Size");
}

TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_setInfoFromEdid)
{
    QMap<QString, QString> mapinfo;
    ut_monitor_setedidmap(mapinfo);

    m_deviceMonitor->setInfoFromEdid(mapinfo);
    EXPECT_STREQ("Monitor Vendor", m_deviceMonitor->m_Name.toStdString().c_str());
    EXPECT_STREQ("Size", m_deviceMonitor->m_ScreenSize.toStdString().c_str());
    EXPECT_STREQ("Vendor", m_deviceMonitor->m_Vendor.toStdString().c_str());
}

TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_transWeekToDate)
{
    QString year = "2020";
    QString week = "13";

    EXPECT_STREQ("2020-03", m_deviceMonitor->transWeekToDate(year, week).toStdString().c_str());
}

TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_setInfoFromXradr_001)
{
    m_deviceMonitor->m_Interface = "VGA";

    EXPECT_FALSE(m_deviceMonitor->setInfoFromXradr("/", "/", "/"));
}

TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_setInfoFromXradr_002)
{
    m_deviceMonitor->m_Interface = "";

    EXPECT_FALSE(m_deviceMonitor->setInfoFromXradr("disconnected", "/", "/"));
}

bool ut_monitor_isDXcbPlatform()
{
    return true;
}

TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_setInfoFromXradr_003)
{
    m_deviceMonitor->m_Interface = "";
    QString main = "HDMI-1 connected primary 1920x1080+0+0 (normal left inverted right x axis y axis) 527mm x 296mm";
    QString edid = "";

    Stub stub;
    stub.set(ADDR(DApplication, isDXcbPlatform), ut_monitor_isDXcbPlatform);

    EXPECT_FALSE(m_deviceMonitor->setInfoFromXradr(main, edid, "/"));
}

TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_setInfoFromXradr_004)
{
    m_deviceMonitor->m_Interface = "";
    m_deviceMonitor->m_ProductionWeek = "2020-04";
    QString main = "HDMI-1 connected primary 1920x1080+0+0 (normal left inverted right x axis y axis) 527mm x 296mm";
    QString edid = "00ffffffffffff005a63384001010101\n0d1e010380351d782ece65a657519f27\n";

    Stub stub;
    stub.set(ADDR(DApplication, isDXcbPlatform), ut_monitor_isDXcbPlatform);

    EXPECT_FALSE(m_deviceMonitor->setInfoFromXradr(main, edid, "/"));
}

TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_setInfoFromXradr_005)
{
    m_deviceMonitor->m_Interface = "";
    m_deviceMonitor->m_ProductionWeek = "2020-03";
    QString main = "HDMI-1 connected primary 1920x1080+0+0 (normal left inverted right x axis y axis) 527mm x 296mm";
    QString edid = "00ffffffffffff005a63384001010101\n0d1e010380351d782ece65a657519f27\n";
    QString rate = "60.00Hz";
    EXPECT_TRUE(m_deviceMonitor->setInfoFromXradr(main, edid, rate));
}

TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_setMainInfoFromXrandr_001)
{
    QString main = "HDMI-1 connected primary 1920x1080+0+0 (normal left inverted right x axis y axis) 527mm x 296mm";
    QString rate = "60.00Hz";

    EXPECT_TRUE(m_deviceMonitor->setMainInfoFromXrandr(main, rate));
    EXPECT_STREQ("1920x1080@60.00Hz", m_deviceMonitor->m_CurrentResolution.toStdString().c_str());
}

TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_setMainInfoFromXrandr_002)
{
    QString main = "HDMI-1 connected 1920x1080+0+0 (normal left inverted right x axis y axis) 527mm x 296mm";
    QString rate = "";

    EXPECT_TRUE(m_deviceMonitor->setMainInfoFromXrandr(main, rate));
    EXPECT_STREQ("1920x1080", m_deviceMonitor->m_CurrentResolution.toStdString().c_str());
}

TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_initFilterKey)
{
    m_deviceMonitor->initFilterKey();
    EXPECT_EQ(1, m_deviceMonitor->m_FilterKey.size());
}

TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_name)
{
    QMap<QString, QString> mapinfo;
    ut_monitor_sethwinfomap(mapinfo);
    m_deviceMonitor->setInfoFromHwinfo(mapinfo);

    QString name = m_deviceMonitor->name();
    EXPECT_STREQ("VIEWSONIC VA2430-FHD", name.toStdString().c_str());
}

TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_driver)
{
    QString driver = m_deviceMonitor->driver();
    EXPECT_STREQ("", driver.toStdString().c_str());
}

TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_subTitle)
{
    QMap<QString, QString> mapinfo;
    ut_monitor_sethwinfomap(mapinfo);
    m_deviceMonitor->setInfoFromHwinfo(mapinfo);

    QString title = m_deviceMonitor->subTitle();
    EXPECT_STREQ("VIEWSONIC VA2430-FHD", title.toStdString().c_str());
}


TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_getOverviewInfo)
{
    QMap<QString, QString> mapinfo;
    ut_monitor_sethwinfomap(mapinfo);
    m_deviceMonitor->setInfoFromHwinfo(mapinfo);

    QString overview = m_deviceMonitor->getOverviewInfo();
    EXPECT_STREQ("VIEWSONIC VA2430-FHD(23.8 inch (527x296 mm))", overview.toStdString().c_str());
}

TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_caculateScreenRatio)
{
    m_deviceMonitor->m_CurrentResolution = "1920x1080";
    m_deviceMonitor->caculateScreenRatio();
    EXPECT_STREQ("16 : 9", m_deviceMonitor->m_AspectRatio.toStdString().c_str());
}

TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_gcd_001)
{
    int a = 10;
    int b = 11;

    EXPECT_EQ(1, m_deviceMonitor->gcd(a, b));
}

TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_gcd_002)
{
    int a = 10;
    int b = 10;

    EXPECT_EQ(10, m_deviceMonitor->gcd(a, b));
}

TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_findAspectRatio)
{
    int arw = 1920;
    int arh = 1080;
    ASSERT_TRUE(m_deviceMonitor->findAspectRatio(1920, 1080, arw, arh));
}

TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_caculateScreenSize_001)
{
    QString edid = "00ffffffffffff005a63384001010101\n0d1e010380351d782ece65a657519f27\n";
    m_deviceMonitor->m_ProductionWeek = "2020-03";
    EXPECT_TRUE(m_deviceMonitor->caculateScreenSize(edid));
    EXPECT_STREQ("23.8 inch(53cm X 29cm)", m_deviceMonitor->m_ScreenSize.toStdString().c_str());
}

TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_caculateScreenSize_002)
{
    QString edid = "";
    m_deviceMonitor->m_ProductionWeek = "2020-03";
    EXPECT_FALSE(m_deviceMonitor->caculateScreenSize(edid));
}

TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_caculateScreenSize_003)
{
    m_deviceMonitor->m_Width = 527;
    m_deviceMonitor->m_Height = 296;
    QString edid = "00ffffffffffff005a63384001010101\n0d1e010380351d782ece65a657519f27\n";
    m_deviceMonitor->m_ProductionWeek = "2020-03";

    EXPECT_TRUE(m_deviceMonitor->caculateScreenSize(edid));
}

int ut_monitor_height()
{
    return -1;
}

TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_caculateScreenSize_004)
{
    m_deviceMonitor->m_Width = 527;
    m_deviceMonitor->m_Height = 296;
    QString edid = "00ffffffffffff005a63384001010101\n0d1e010380351d782ece65a657519f27\n";
    m_deviceMonitor->m_ProductionWeek = "2020-03";

    Stub stub;
    stub.set(ADDR(EDIDParser, height), ut_monitor_height);
    EXPECT_FALSE(m_deviceMonitor->caculateScreenSize(edid));
}

TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_caculateScreenSize_005)
{
    m_deviceMonitor->m_ScreenSize = "527x296 mm";

    m_deviceMonitor->caculateScreenSize();
    EXPECT_STREQ("23.8 inch(527mm X 296mm)", m_deviceMonitor->m_ScreenSize.toStdString().c_str());
    EXPECT_EQ(527, m_deviceMonitor->m_Width);
    EXPECT_EQ(296, m_deviceMonitor->m_Height);
}

TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_loadBaseDeviceInfo)
{
    m_deviceMonitor->loadBaseDeviceInfo();
    EXPECT_EQ(0, m_deviceMonitor->m_LstBaseInfo.size());
}

TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_loadOtherDeviceInfo)
{
    m_deviceMonitor->m_CurrentResolution = "1920*1080@60Hz";
    m_deviceMonitor->loadOtherDeviceInfo();
    EXPECT_EQ(1, m_deviceMonitor->m_LstOtherInfo.size());
}

TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_loadTableData)
{
    m_deviceMonitor->loadTableData();
    EXPECT_EQ(3, m_deviceMonitor->m_TableData.size());
}

TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_available)
{
    EXPECT_TRUE(m_deviceMonitor->available());
}
