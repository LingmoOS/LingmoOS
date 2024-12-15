// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DevicePower.h"
#include "DeviceBios.h"

#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class UT_DevicePower : public UT_HEAD
{
public:
    void SetUp()
    {
        m_DevicePower = new DevicePower;
    }
    void TearDown()
    {
        delete m_DevicePower;
    }
    DevicePower *m_DevicePower;
};

void ut_power_setupowerinfo(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("Device", "Device");
    mapinfo.insert("serial", "serial");
    mapinfo.insert("capacity", "capacity");
    mapinfo.insert("voltage", "voltage");
    mapinfo.insert("temperature", "temperature");
}

TEST_F(UT_DevicePower, UT_DevicePower_setInfoFromUpower_001)
{
    QMap<QString, QString> mapinfo;
    ut_power_setupowerinfo(mapinfo);

    EXPECT_TRUE(m_DevicePower->setInfoFromUpower(mapinfo));
    EXPECT_STREQ("battery", m_DevicePower->m_Name.toStdString().c_str());
    EXPECT_STREQ("serial", m_DevicePower->m_SerialNumber.toStdString().c_str());
    EXPECT_STREQ("capacity", m_DevicePower->m_Capacity.toStdString().c_str());
    EXPECT_STREQ("voltage", m_DevicePower->m_Voltage.toStdString().c_str());
    EXPECT_STREQ("temperature", m_DevicePower->m_Temp.toStdString().c_str());
}

TEST_F(UT_DevicePower, UT_DevicePower_setInfoFromUpower_002)
{
    QMap<QString, QString> mapinfo;
    ut_power_setupowerinfo(mapinfo);
    mapinfo.insert("Device", "line_power");

    EXPECT_FALSE(m_DevicePower->setInfoFromUpower(mapinfo));
}

TEST_F(UT_DevicePower, UT_DevicePower_setDaemonInfo)
{
    QMap<QString, QString> mapinfo;
    ut_power_setupowerinfo(mapinfo);
    m_DevicePower->setInfoFromUpower(mapinfo);

    m_DevicePower->setDaemonInfo(mapinfo);
    EXPECT_STREQ("battery", m_DevicePower->m_Name.toStdString().c_str());
}

TEST_F(UT_DevicePower, UT_DevicePower_name)
{
    QMap<QString, QString> mapinfo;
    ut_power_setupowerinfo(mapinfo);
    m_DevicePower->setInfoFromUpower(mapinfo);
    QString name = m_DevicePower->name();

    EXPECT_STREQ("battery", name.toStdString().c_str());
}

TEST_F(UT_DevicePower, UT_DevicePower_driver)
{
    QMap<QString, QString> mapinfo;
    ut_power_setupowerinfo(mapinfo);
    m_DevicePower->setInfoFromUpower(mapinfo);
    QString driver = m_DevicePower->driver();

    EXPECT_STREQ("", driver.toStdString().c_str());
}

TEST_F(UT_DevicePower, UT_DevicePower_subTitle)
{
    QMap<QString, QString> mapinfo;
    ut_power_setupowerinfo(mapinfo);
    m_DevicePower->setInfoFromUpower(mapinfo);
    QString title = m_DevicePower->subTitle();

    EXPECT_STREQ("battery", title.toStdString().c_str());
}

TEST_F(UT_DevicePower, UT_DevicePower_getOverviewInfo)
{
    QMap<QString, QString> mapinfo;
    ut_power_setupowerinfo(mapinfo);
    m_DevicePower->setInfoFromUpower(mapinfo);
    QString overview = m_DevicePower->getOverviewInfo();

    EXPECT_STREQ("battery", overview.toStdString().c_str());
}

TEST_F(UT_DevicePower, UT_DevicePower_initFilterKey)
{
    m_DevicePower->initFilterKey();
    EXPECT_EQ(23, m_DevicePower->m_FilterKey.size());
}

TEST_F(UT_DevicePower, UT_DevicePower_loadBaseDeviceInfo)
{
    QMap<QString, QString> mapinfo;
    ut_power_setupowerinfo(mapinfo);
    m_DevicePower->setInfoFromUpower(mapinfo);

    m_DevicePower->loadBaseDeviceInfo();
    EXPECT_EQ(5, m_DevicePower->m_LstBaseInfo.size());
}

TEST_F(UT_DevicePower, UT_DevicePower_loadOtherDeviceInfo)
{
    m_DevicePower->loadOtherDeviceInfo();
    EXPECT_EQ(0, m_DevicePower->m_LstOtherInfo.size());
}


TEST_F(UT_DevicePower, UT_DevicePower_loadTableData)
{
    QMap<QString, QString> mapinfo;
    ut_power_setupowerinfo(mapinfo);
    m_DevicePower->setInfoFromUpower(mapinfo);

    m_DevicePower->loadTableData();
    EXPECT_EQ(3, m_DevicePower->m_TableData.size());
}

TEST_F(UT_DevicePower, UT_DevicePower_available)
{
    EXPECT_TRUE(m_DevicePower->available());
}
