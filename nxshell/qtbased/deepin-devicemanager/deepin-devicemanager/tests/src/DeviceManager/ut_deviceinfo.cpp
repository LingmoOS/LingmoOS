// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DeviceInfo.h"
#include "DeviceAudio.h"

#include "xlsxdocument.h"
#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class UT_DeviceInfo : public UT_HEAD
{
public:
    void SetUp()
    {
    }
    void TearDown()
    {
        delete audio;
    }
    DeviceBaseInfo *m_deviceBaseInfo;
    DeviceAudio *audio = new DeviceAudio;
};

TEST_F(UT_DeviceInfo, UT_DeviceInfo_getOtherAttribs)
{
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(audio);
    QList<QPair<QString, QString>> lst = m_deviceBaseInfo->getOtherAttribs();
    EXPECT_EQ(0, lst.size());
    EXPECT_EQ(0, m_deviceBaseInfo->m_LstOtherInfo.size());
}

TEST_F(UT_DeviceInfo, UT_DeviceInfo_getBaseAttribs)
{
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(audio);
    QList<QPair<QString, QString>> lst = m_deviceBaseInfo->getBaseAttribs();
    EXPECT_EQ(0, lst.size());
    EXPECT_EQ(0, m_deviceBaseInfo->m_LstBaseInfo.size());
}

TEST_F(UT_DeviceInfo, UT_DeviceInfo_getTableHeader)
{
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(audio);
    QStringList lst = m_deviceBaseInfo->getTableHeader();
    EXPECT_EQ(3, lst.size());
    EXPECT_EQ(3, m_deviceBaseInfo->m_TableHeader.size());
}

TEST_F(UT_DeviceInfo, UT_DeviceInfo_getTableData)
{
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(audio);
    QStringList lst = m_deviceBaseInfo->getTableData();
    EXPECT_EQ(2, lst.size());
    EXPECT_EQ(2, m_deviceBaseInfo->m_TableData.size());
}

TEST_F(UT_DeviceInfo, UT_DeviceInfo_subTitle)
{
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(audio);
    QString title = m_deviceBaseInfo->subTitle();
    EXPECT_STREQ("", title.toStdString().c_str());
}

TEST_F(UT_DeviceInfo, UT_DeviceInfo_isValueValid)
{
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(audio);
    QString value = "";
    EXPECT_FALSE(m_deviceBaseInfo->isValueValid(value));
}

TEST_F(UT_DeviceInfo, UT_DeviceInfo_setEnable)
{
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(audio);
    m_deviceBaseInfo->setEnable(true);
    m_deviceBaseInfo->enable();
    m_deviceBaseInfo->setCanEnale(true);
    ASSERT_TRUE(m_deviceBaseInfo->canEnable());
}

TEST_F(UT_DeviceInfo, UT_DeviceInfo_getOverviewInfo)
{
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(audio);
    QString ret = m_deviceBaseInfo->getOverviewInfo();
    EXPECT_STREQ("", ret.toStdString().c_str());
}

TEST_F(UT_DeviceInfo, UT_DeviceInfo_getOtherMapInfo)
{
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(audio);
    QMap<QString, QString> mapinfo;
    mapinfo.insert("bus info", "1@n");
    m_deviceBaseInfo->getOtherMapInfo(mapinfo);

    EXPECT_EQ(0, m_deviceBaseInfo->m_LstOtherInfo.size());
}

TEST_F(UT_DeviceInfo, UT_DeviceInfo_addBaseDeviceInfo)
{
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(audio);
    m_deviceBaseInfo->addBaseDeviceInfo("name", "abc@123");
    EXPECT_EQ(1, m_deviceBaseInfo->m_LstBaseInfo.size());
}

TEST_F(UT_DeviceInfo, UT_DeviceInfo_setAttribute)
{
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(audio);
    QMap<QString, QString> mapinfo;
    mapinfo.insert("bus info", "1@n");
    QString value = "abc@123";
    m_deviceBaseInfo->setAttribute(mapinfo, "name", value, true);
    EXPECT_STREQ("abc@123", value.toStdString().c_str());
}

TEST_F(UT_DeviceInfo, UT_DeviceInfo_mapInfoToList)
{
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(audio);
    QMap<QString, QString> mapinfo;
    mapinfo.insert("bus info", "1@n");

    m_deviceBaseInfo->m_MapOtherInfo = mapinfo;
    m_deviceBaseInfo->mapInfoToList();

    EXPECT_EQ(1, m_deviceBaseInfo->m_LstOtherInfo.size());
}
