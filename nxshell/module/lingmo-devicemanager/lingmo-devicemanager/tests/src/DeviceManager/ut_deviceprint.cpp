// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DevicePrint.h"
#include "DeviceInfo.h"
#include "DBusEnableInterface.h"
#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class UT_DevicePrint : public UT_HEAD
{
public:
    void SetUp()
    {
        m_devicePrint = new DevicePrint;
    }
    void TearDown()
    {
        delete m_devicePrint;
    }
    DevicePrint *m_devicePrint;
};

void ut_print_setmap(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("printer-info", "Canon iR-ADV C3720 22.21");
    mapinfo.insert("Name", "Canon-iR-ADV-C3720-UFR");
    mapinfo.insert("device-uri", "socket://10.4.12.241");
    mapinfo.insert("printer-state", "3");
}

TEST_F(UT_DevicePrint, UT_DevicePrint_setInfo)
{
    QMap<QString, QString> mapinfo;
    ut_print_setmap(mapinfo);
    m_devicePrint->setInfo(mapinfo);

    EXPECT_STREQ("Canon", m_devicePrint->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("iR-ADV C3720 22.21", m_devicePrint->m_Model.toStdString().c_str());
    EXPECT_STREQ("Canon-iR-ADV-C3720-UFR", m_devicePrint->m_Name.toStdString().c_str());
    EXPECT_STREQ("socket://10.4.12.241", m_devicePrint->m_URI.toStdString().c_str());
    EXPECT_STREQ("3", m_devicePrint->m_Status.toStdString().c_str());
    EXPECT_STREQ("socket", m_devicePrint->m_InterfaceType.toStdString().c_str());
}

TEST_F(UT_DevicePrint, UT_DevicePrint_name)
{
    QMap<QString, QString> mapinfo;
    ut_print_setmap(mapinfo);
    m_devicePrint->setInfo(mapinfo);

    QString name = m_devicePrint->name();
    EXPECT_STREQ("Canon-iR-ADV-C3720-UFR", name.toStdString().c_str());
}

TEST_F(UT_DevicePrint, UT_DevicePrint_driver)
{
    QString driver = m_devicePrint->driver();
    EXPECT_STREQ("", driver.toStdString().c_str());
}

TEST_F(UT_DevicePrint, UT_DevicePrint_subTitle)
{
    QMap<QString, QString> mapinfo;
    ut_print_setmap(mapinfo);
    m_devicePrint->setInfo(mapinfo);

    QString title = m_devicePrint->subTitle();
    EXPECT_STREQ("Canon-iR-ADV-C3720-UFR", title.toStdString().c_str());
}

TEST_F(UT_DevicePrint, UT_DevicePrint_getOverviewInfo)
{
    QMap<QString, QString> mapinfo;
    ut_print_setmap(mapinfo);
    m_devicePrint->setInfo(mapinfo);

    QString overview = m_devicePrint->getOverviewInfo();
    EXPECT_STREQ("Canon-iR-ADV-C3720-UFR", overview.toStdString().c_str());
}

bool ut_print_enablePrinter_false()
{
    return false;
}

TEST_F(UT_DevicePrint, UT_DevicePrint_setEnable_001)
{
    Stub stub;
    stub.set(ADDR(DBusEnableInterface, enablePrinter), ut_print_enablePrinter_false);

    EXPECT_EQ(EnableDeviceStatus::EDS_Faild, m_devicePrint->setEnable(true));
}

bool ut_print_enablePrinter_true()
{
    return true;
}

TEST_F(UT_DevicePrint, UT_DevicePrint_setEnable_002)
{
    Stub stub;
    stub.set(ADDR(DBusEnableInterface, enablePrinter), ut_print_enablePrinter_true);

    EXPECT_EQ(EnableDeviceStatus::EDS_Success, m_devicePrint->setEnable(true));
    EXPECT_TRUE(m_devicePrint->m_Enable);
}


TEST_F(UT_DevicePrint, UT_DevicePrint_enable)
{
    m_devicePrint->m_Status = "5";
    EXPECT_FALSE(m_devicePrint->enable());
}

TEST_F(UT_DevicePrint, UT_DevicePrint_initFilterKey)
{
    m_devicePrint->initFilterKey();
    EXPECT_EQ(16, m_devicePrint->m_FilterKey.size());
}

TEST_F(UT_DevicePrint, UT_DevicePrint_loadBaseDeviceInfo)
{
    QMap<QString, QString> mapinfo;
    ut_print_setmap(mapinfo);
    m_devicePrint->setInfo(mapinfo);

    m_devicePrint->loadBaseDeviceInfo();
    EXPECT_EQ(3, m_devicePrint->m_LstBaseInfo.size());

}

TEST_F(UT_DevicePrint, UT_DevicePrint_loadOtherDeviceInfo)
{
    m_devicePrint->loadOtherDeviceInfo();
    EXPECT_EQ(0, m_devicePrint->m_LstOtherInfo.size());
}

TEST_F(UT_DevicePrint, UT_DevicePrint_loadTableData_001)
{
    QMap<QString, QString> mapinfo;
    ut_print_setmap(mapinfo);
    m_devicePrint->setInfo(mapinfo);

    m_devicePrint->m_Status = "5";
    m_devicePrint->loadTableData();
    EXPECT_EQ(3, m_devicePrint->m_TableData.size());
}

TEST_F(UT_DevicePrint, UT_DevicePrint_loadTableData_002)
{
    QMap<QString, QString> mapinfo;
    ut_print_setmap(mapinfo);
    m_devicePrint->setInfo(mapinfo);

    m_devicePrint->m_Status = "3";
    m_devicePrint->loadTableData();
    EXPECT_EQ(3, m_devicePrint->m_TableData.size());
}

TEST_F(UT_DevicePrint, UT_DevicePrint_available)
{
    EXPECT_TRUE(m_devicePrint->available());
}

TEST_F(UT_DevicePrint, UT_DevicePrint_getVendor)
{
    QMap<QString, QString> mapinfo;
    ut_print_setmap(mapinfo);
    m_devicePrint->setInfo(mapinfo);
    EXPECT_STREQ("Canon", m_devicePrint->getVendor().toStdString().c_str());
}

TEST_F(UT_DevicePrint, UT_DevicePrint_getModel)
{
    QMap<QString, QString> mapinfo;
    ut_print_setmap(mapinfo);
    m_devicePrint->setInfo(mapinfo);
    EXPECT_STREQ("iR-ADV C3720 22.21", m_devicePrint->getModel().toStdString().c_str());
}
