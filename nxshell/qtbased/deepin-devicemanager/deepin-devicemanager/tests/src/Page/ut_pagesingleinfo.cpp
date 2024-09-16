// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PageInfo.h"
#include "PageSingleInfo.h"
#include "PageTableWidget.h"
#include "DeviceInput.h"
#include "DeviceInfo.h"
#include "ut_Head.h"
#include "stub.h"

#include <DApplication>

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QSignalSpy>
#include <QClipboard>
#include <QMenu>
#include <QAction>

#include <gtest/gtest.h>

class UT_PageSingleInfo : public UT_HEAD
{
public:
    void SetUp()
    {
        m_PageSingleInfo = new PageSingleInfo;
    }
    void TearDown()
    {
        delete m_PageSingleInfo;
    }
    PageSingleInfo *m_PageSingleInfo = nullptr;
};

TEST_F(UT_PageSingleInfo, UT_PageSingleInfo_updateInfo)
{
    DeviceInput *device = new DeviceInput;
    device->m_Name = "name";
    device->m_Vendor = "vendor";
    device->m_Model = "model";
    QList<DeviceBaseInfo *> bInfo;
    bInfo.append(device);
    m_PageSingleInfo->updateInfo(bInfo);
    DeviceInput *d = dynamic_cast<DeviceInput *>(m_PageSingleInfo->mp_Device);
    EXPECT_STREQ(d->m_Name.toStdString().c_str(), "name");
    EXPECT_STREQ(d->m_Vendor.toStdString().c_str(), "vendor");
    EXPECT_STREQ(d->m_Model.toStdString().c_str(), "model");
    delete device;
}

TEST_F(UT_PageSingleInfo, UT_PageSingleInfo_setLabel)
{
    m_PageSingleInfo->setLabel("test");
    EXPECT_STREQ("test", m_PageSingleInfo->mp_Label->text().toStdString().c_str());
    m_PageSingleInfo->clearWidgets();
}

TEST_F(UT_PageSingleInfo, UT_PageSingleInfo_slotActionCopy)
{
    QString conStr = m_PageSingleInfo->mp_Content->toString();
    m_PageSingleInfo->slotActionCopy();
    QClipboard *clipboard = DApplication::clipboard();
    QString clipStr = clipboard->text();
    EXPECT_STREQ(conStr.toStdString().c_str(), clipStr.toStdString().c_str());
}

void ut_single_exec()
{
    return;
}

TEST_F(UT_PageSingleInfo, UT_PageSingleInfo_slotShowMenu_001)
{
    m_PageSingleInfo->mp_Device = new DeviceInput();
    Stub stub;
    stub.set((QAction * (QMenu::*)(const QPoint &, QAction *))ADDR(QMenu, exec), ut_single_exec);

    m_PageSingleInfo->slotShowMenu(QPoint(0, 0));
    EXPECT_EQ(m_PageSingleInfo->mp_Menu->actions().size(), 9);

    delete m_PageSingleInfo->mp_Device;
}

TEST_F(UT_PageSingleInfo, UT_PageSingleInfo_slotShowMenu_002)
{
    m_PageSingleInfo->mp_Device = new DeviceInput();
    m_PageSingleInfo->mp_Device->m_Enable = true;
    Stub stub;
    stub.set((QAction * (QMenu::*)(const QPoint &, QAction *))ADDR(QMenu, exec), ut_single_exec);

    m_PageSingleInfo->slotShowMenu(QPoint(0, 0));
    EXPECT_EQ(m_PageSingleInfo->mp_Menu->actions().size(), 9);

    delete m_PageSingleInfo->mp_Device;
}

TEST_F(UT_PageSingleInfo, UT_PageSingleInfo_slotActionEnable)
{
    DeviceInput *device = new DeviceInput;
    device->m_Name = "name";
    device->m_Vendor = "vendor";
    device->m_Model = "model";
    m_PageSingleInfo->mp_Device = device;
    m_PageSingleInfo->slotActionEnable();
    EXPECT_TRUE(m_PageSingleInfo->mp_Device->enable());
    delete device;
}

TEST_F(UT_PageSingleInfo, UT_PageSingleInfo_slotActionUpdateDriver)
{
    DeviceInput *device = new DeviceInput;
    device->m_Name = "name";
    device->m_Vendor = "vendor";
    device->m_Model = "model";
    device->m_Driver = "usbhid";
    m_PageSingleInfo->mp_Device = device;
    m_PageSingleInfo->slotActionUpdateDriver();
    EXPECT_TRUE(m_PageSingleInfo->mp_Device->enable());
    delete device;
}

TEST_F(UT_PageSingleInfo, UT_PageSingleInfo_slotActionRemoveDriver)
{
    DeviceInput *device = new DeviceInput;
    device->m_Name = "name";
    device->m_Vendor = "vendor";
    device->m_Model = "model";
    device->m_Driver = "usbhid";
    m_PageSingleInfo->mp_Device = device;
    m_PageSingleInfo->slotActionRemoveDriver();
    EXPECT_TRUE(m_PageSingleInfo->mp_Device->enable());
    delete device;
}
