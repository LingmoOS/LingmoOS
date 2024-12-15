// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DeviceBios.h"

#include "stub.h"
#include "ut_Head.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>


class UT_DeviceBios : public UT_HEAD
{
public:
    void SetUp()
    {
        m_deviceBios = new DeviceBios;
    }
    void TearDown()
    {
        delete m_deviceBios;
    }
    DeviceBios *m_deviceBios;
};

TEST_F(UT_DeviceBios, UT_DeviceBios_setBiosInfo_001)
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Vendor", "American Megatrends Inc.");
    mapInfo.insert("Version", "5.12");

    ASSERT_TRUE(m_deviceBios->setBiosInfo(mapInfo));
    EXPECT_STREQ("BIOS Information", m_deviceBios->m_Name.toStdString().c_str());
    EXPECT_STREQ("American Megatrends Inc.", m_deviceBios->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("5.12", m_deviceBios->m_Version.toStdString().c_str());
}

TEST_F(UT_DeviceBios, UT_DeviceBios_setBiosInfo_002)
{
    QMap<QString, QString> mapInfo;
    ASSERT_FALSE(m_deviceBios->setBiosInfo(mapInfo));
}

TEST_F(UT_DeviceBios, UT_DeviceBios_setBiosLanguageInfo)
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Language Description Format", "Long");

    ASSERT_TRUE(m_deviceBios->setBiosLanguageInfo(mapInfo));
}

TEST_F(UT_DeviceBios, UT_DeviceBios_setBaseBoardInfo_001)
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Manufacturer", "IPASON");
    mapInfo.insert("Version", "1.0");
    mapInfo.insert("Product Name", "H310MHP");
    mapInfo.insert("Board name", "H310MHP");
    mapInfo.insert("chipset", "Biostar Microtech Int'l Corp Device 3114");

    ASSERT_TRUE(m_deviceBios->setBaseBoardInfo(mapInfo));

    EXPECT_STREQ("Base Board Information", m_deviceBios->m_Name.toStdString().c_str());
    EXPECT_STREQ("IPASON", m_deviceBios->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("1.0", m_deviceBios->m_Version.toStdString().c_str());
    EXPECT_STREQ("H310MHP", m_deviceBios->m_ProductName.toStdString().c_str());
    EXPECT_STREQ("Biostar Microtech Int'l Corp Device 3114", m_deviceBios->m_ChipsetFamily.toStdString().c_str());
    EXPECT_TRUE(m_deviceBios->m_IsBoard);
}

TEST_F(UT_DeviceBios, UT_DeviceBios_setBaseBoardInfo_002)
{
    QMap<QString, QString> mapInfo;
    ASSERT_FALSE(m_deviceBios->setBaseBoardInfo(mapInfo));
}

TEST_F(UT_DeviceBios, UT_DeviceBios_setSystemInfo_001)
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Manufacturer", "IPASON");
    mapInfo.insert("Version", "1.0");

    ASSERT_TRUE(m_deviceBios->setSystemInfo(mapInfo));
    EXPECT_STREQ("System Information", m_deviceBios->m_Name.toStdString().c_str());
    EXPECT_STREQ("IPASON", m_deviceBios->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("1.0", m_deviceBios->m_Version.toStdString().c_str());

}

TEST_F(UT_DeviceBios, UT_DeviceBios_setSystemInfo_002)
{
    QMap<QString, QString> mapInfo;
    ASSERT_FALSE(m_deviceBios->setSystemInfo(mapInfo));

}
TEST_F(UT_DeviceBios, UT_DeviceBios_setChassisInfo_001)
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Manufacturer", "IPASON");
    mapInfo.insert("Version", "1.0");

    ASSERT_TRUE(m_deviceBios->setChassisInfo(mapInfo));
    EXPECT_STREQ("Chassis Information", m_deviceBios->m_Name.toStdString().c_str());
    EXPECT_STREQ("IPASON", m_deviceBios->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("1.0", m_deviceBios->m_Version.toStdString().c_str());

}

TEST_F(UT_DeviceBios, UT_DeviceBios_setChassisInfo_002)
{
    QMap<QString, QString> mapInfo;
    ASSERT_FALSE(m_deviceBios->setChassisInfo(mapInfo));

}

TEST_F(UT_DeviceBios, UT_DeviceBios_setMemoryInfo_001)
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Manufacturer", "IPASON");
    mapInfo.insert("Version", "1.0");

    ASSERT_TRUE(m_deviceBios->setMemoryInfo(mapInfo));
    EXPECT_STREQ("Physical Memory Array", m_deviceBios->m_Name.toStdString().c_str());
    EXPECT_STREQ("IPASON", m_deviceBios->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("1.0", m_deviceBios->m_Version.toStdString().c_str());
}

TEST_F(UT_DeviceBios, UT_DeviceBios_setMemoryInfo_002)
{
    QMap<QString, QString> mapInfo;
    ASSERT_FALSE(m_deviceBios->setMemoryInfo(mapInfo));
}

TEST_F(UT_DeviceBios, UT_DeviceBios_name)
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Vendor", "American Megatrends Inc.");
    mapInfo.insert("Version", "5.12");
    m_deviceBios->setBiosInfo(mapInfo);

    QString name = m_deviceBios->name();
    EXPECT_STREQ("BIOS Information", name.toStdString().c_str());
}

TEST_F(UT_DeviceBios, UT_DeviceBios_vendor)
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Vendor", "American Megatrends Inc.");
    mapInfo.insert("Version", "5.12");
    m_deviceBios->setBiosInfo(mapInfo);

    QString vendor = m_deviceBios->vendor();
    EXPECT_STREQ("American Megatrends Inc.", vendor.toStdString().c_str());
}

TEST_F(UT_DeviceBios, UT_DeviceBios_driver)
{
    QString driver = m_deviceBios->driver();
    EXPECT_STREQ("", driver.toStdString().c_str());
}

TEST_F(UT_DeviceBios, UT_DeviceBios_version)
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Vendor", "American Megatrends Inc.");
    mapInfo.insert("Version", "5.12");
    m_deviceBios->setBiosInfo(mapInfo);

    QString version = m_deviceBios->version();
    EXPECT_STREQ("5.12", version.toStdString().c_str());
}


TEST_F(UT_DeviceBios, UT_DeviceBios_subTitle)
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Vendor", "American Megatrends Inc.");
    mapInfo.insert("Version", "5.12");
    m_deviceBios->setBiosInfo(mapInfo);

    QString title = m_deviceBios->subTitle();
    EXPECT_STREQ("BIOS Information", title.toStdString().c_str());
}

TEST_F(UT_DeviceBios, UT_DeviceBios_getOverviewInfo_001)
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Manufacturer", "IPASON");
    mapInfo.insert("Version", "1.0");
    mapInfo.insert("Product Name", "H310MHP");
    mapInfo.insert("Board name", "H310MHP");
    mapInfo.insert("chipset", "Biostar Microtech Int'l Corp Device 3114");
    m_deviceBios->setBaseBoardInfo(mapInfo);

    EXPECT_STREQ("H310MHP", m_deviceBios->getOverviewInfo().toStdString().c_str());
}

TEST_F(UT_DeviceBios, UT_DeviceBios_getOverviewInfo_002)
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Vendor", "American Megatrends Inc.");
    mapInfo.insert("Version", "5.12");
    m_deviceBios->setBiosInfo(mapInfo);


    EXPECT_STREQ("", m_deviceBios->getOverviewInfo().toStdString().c_str());
}

TEST_F(UT_DeviceBios, UT_DeviceBios_initFilterKey)
{
    m_deviceBios->initFilterKey();
    EXPECT_EQ(45, m_deviceBios->m_FilterKey.size());
}

TEST_F(UT_DeviceBios, UT_DeviceBios_loadBaseDeviceInfo)
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Manufacturer", "IPASON");
    mapInfo.insert("Version", "1.0");
    mapInfo.insert("Product Name", "H310MHP");
    mapInfo.insert("Board name", "H310MHP");
    mapInfo.insert("chipset", "Biostar Microtech Int'l Corp Device 3114");
    m_deviceBios->setBaseBoardInfo(mapInfo);

    m_deviceBios->loadBaseDeviceInfo();

    QPair<QString, QString> value0 = m_deviceBios->m_LstBaseInfo.at(0);
    EXPECT_STREQ("IPASON", value0.second.toStdString().c_str());
    QPair<QString, QString> value1 = m_deviceBios->m_LstBaseInfo.at(1);
    EXPECT_STREQ("1.0", value1.second.toStdString().c_str());
    QPair<QString, QString> value2 = m_deviceBios->m_LstBaseInfo.at(2);
    EXPECT_STREQ("Biostar Microtech Int'l Corp Device 3114", value2.second.toStdString().c_str());
}

TEST_F(UT_DeviceBios, UT_DeviceBios_loadOtherDeviceInfo)
{
    m_deviceBios->loadOtherDeviceInfo();
    EXPECT_EQ(0, m_deviceBios->m_LstOtherInfo.size());
}

TEST_F(UT_DeviceBios, UT_DeviceBios_loadTableHeader)
{
    m_deviceBios->loadTableHeader();
    EXPECT_EQ(0, m_deviceBios->m_TableHeader.size());
}

TEST_F(UT_DeviceBios, UT_DeviceBios_loadTableData)
{
    m_deviceBios->loadTableData();
    EXPECT_EQ(0, m_deviceBios->m_TableData.size());
}
