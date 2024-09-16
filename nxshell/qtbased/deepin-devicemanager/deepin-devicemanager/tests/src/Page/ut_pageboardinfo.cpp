// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PageBoardInfo.h"
#include "DeviceInfo.h"
#include "DeviceInput.h"
#include "DeviceBios.h"
#include "PageInfoWidget.h"
#include "PageTableWidget.h"
#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class PageBoardInfo_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_pageBoardInfo = new PageBoardInfo;
    }
    void TearDown()
    {
        delete m_pageBoardInfo;
    }
    PageBoardInfo *m_pageBoardInfo;
};

TEST_F(PageBoardInfo_UT, ut_updateInfo)
{
    DeviceBios *device = new DeviceBios;
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    device->setBiosInfo(mapinfo);
    QList<DeviceBaseInfo *> bInfo;
    bInfo.append(device);
    m_pageBoardInfo->updateInfo(bInfo);
    EXPECT_TRUE(m_pageBoardInfo->mp_Device->getBaseAttribs().isEmpty());
    delete device;
}

void ut_board_getOtherInfoPair(void *obj, const QList<DeviceBaseInfo *> &lst, QList<QPair<QString, QString>> &lstPair)
{
    lstPair.append(QPair<QString, QString>("/", "abc:def\n123:456"));
    lstPair.append(QPair<QString, QString>("/", "abc:def\n123:456"));
}

TEST_F(PageBoardInfo_UT, ut_loadDeviceInfo)
{
    DeviceBios *device = new DeviceBios;
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    device->setBiosInfo(mapinfo);
    QList<DeviceBaseInfo *> bInfo;
    bInfo.append(device);
    QList<QPair<QString, QString>> lst;
    lst.append(QPair<QString, QString>("/", "abc:def\n123:456"));
    Stub stub;
    stub.set(ADDR(PageBoardInfo, getOtherInfoPair), ut_board_getOtherInfoPair);

    m_pageBoardInfo->loadDeviceInfo(bInfo, lst);

    EXPECT_FALSE(m_pageBoardInfo->mp_Content->toString().isEmpty());
    delete device;
}

TEST_F(PageBoardInfo_UT, ut_getOtherInfoPair)
{
    DeviceInput *device = new DeviceInput;
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    device->setInfoFromHwinfo(mapinfo);
    QList<DeviceBaseInfo *> bInfo;
    bInfo.append(device);
    QList<QPair<QString, QString>> lst;
    lst.append(QPair<QString, QString>("/", "/"));
    m_pageBoardInfo->getOtherInfoPair(bInfo, lst);
    EXPECT_EQ(1, lst.size());
    delete device;
}

TEST_F(PageBoardInfo_UT, ut_getValueInfo)
{
    DeviceInput *device = new DeviceInput;
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    device->setInfoFromHwinfo(mapinfo);
    QList<DeviceBaseInfo *> bInfo;
    bInfo.append(device);
    QPair<QString, QString> pair = QPair<QString, QString>("/", "/");
    m_pageBoardInfo->getValueInfo(bInfo.at(0), pair);
    EXPECT_EQ(15, pair.second.size());
    delete device;
}
