// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PageInfo.h"
#include "PageMultiInfo.h"
#include "DeviceInput.h"
#include "DevicePrint.h"
#include "PageDetail.h"
#include "ut_Head.h"
#include "stub.h"

#include <DMessageManager>

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QScrollBar>

#include <gtest/gtest.h>

class PageMultiInfo_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_pageMultiInfo = new PageMultiInfo;
    }
    void TearDown()
    {
        delete m_pageMultiInfo;
    }
    PageMultiInfo *m_pageMultiInfo = nullptr;
};

TEST_F(PageMultiInfo_UT, PageMultiInfo_UT_updateInfo)
{
    DeviceInput *device = new DeviceInput;
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    device->setInfoFromHwinfo(mapinfo);
    QList<DeviceBaseInfo *> bInfo;
    bInfo.append(device);
    m_pageMultiInfo->updateInfo(bInfo);
    EXPECT_EQ(0, m_pageMultiInfo->mp_Detail->mp_ScrollArea->verticalScrollBar()->value());
    delete device;
}

TEST_F(PageMultiInfo_UT, PageMultiInfo_UT_setLabel)
{
    m_pageMultiInfo->setLabel("/");
    EXPECT_EQ("/", m_pageMultiInfo->mp_Label->text());
    EXPECT_EQ(63, m_pageMultiInfo->mp_Label->font().weight());
    m_pageMultiInfo->clearWidgets();
    EXPECT_TRUE(m_pageMultiInfo->mp_Detail->m_ListTextBrowser.isEmpty());
}

TEST_F(PageMultiInfo_UT, PageMultiInfo_UT_slotItemClicked)
{
    m_pageMultiInfo->slotItemClicked(0);
    EXPECT_EQ(0, m_pageMultiInfo->mp_Detail->mp_ScrollArea->verticalScrollBar()->value());
    m_pageMultiInfo->slotEnableDevice(0, true);
    EXPECT_FALSE(m_pageMultiInfo->isVisible());
}

void ut_pagemulti_setHwinfoMap(QMap<QString, QString> &mapinfo)
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

TEST_F(PageMultiInfo_UT, PageMultiInfo_UT_slotActionUpdateDriver_001)
{
    DeviceInput *device = new DeviceInput;
    QMap<QString, QString> mapinfo;
    ut_pagemulti_setHwinfoMap(mapinfo);
    device->setInfoFromHwinfo(mapinfo);

    m_pageMultiInfo->m_lstDevice.append(device);
    m_pageMultiInfo->slotActionUpdateDriver(0);

    EXPECT_EQ(1, m_pageMultiInfo->m_lstDevice.size());

    delete device;
}

void ut_pagemulti_printsetmap(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("printer-info", "Canon iR-ADV C3720 22.21");
    mapinfo.insert("Name", "Canon-iR-ADV-C3720-UFR");
    mapinfo.insert("device-uri", "socket://10.4.12.241");
    mapinfo.insert("printer-state", "3");
}

TEST_F(PageMultiInfo_UT, PageMultiInfo_UT_slotActionUpdateDriver_002)
{
    DevicePrint *device = new DevicePrint;
    QMap<QString, QString> mapinfo;
    ut_pagemulti_printsetmap(mapinfo);
    device->setInfo(mapinfo);

    m_pageMultiInfo->m_lstDevice.append(device);
    m_pageMultiInfo->slotActionUpdateDriver(0);

    EXPECT_EQ(1, m_pageMultiInfo->m_lstDevice.size());
    delete device;
}

TEST_F(PageMultiInfo_UT, PageMultiInfo_UT_slotActionRemoveDriver)
{
    DevicePrint *device = new DevicePrint;
    QMap<QString, QString> mapinfo;
    ut_pagemulti_printsetmap(mapinfo);
    device->setInfo(mapinfo);

    m_pageMultiInfo->m_lstDevice.append(device);
    m_pageMultiInfo->slotActionRemoveDriver(0);

    EXPECT_EQ(1, m_pageMultiInfo->m_lstDevice.size());
    delete device;
}
