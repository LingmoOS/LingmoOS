// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QSignalSpy>

#include "ut_Head.h"
#include "stub.h"

#define private public
#include "PageDriverInstallInfo.h"
#include "PageDriverBackupInfo.h"
#include "PageDriverRestoreInfo.h"
#include "PageDriverTableView.h"


class PageDriverInfo_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_dPageDriverInstallInfo = new PageDriverInstallInfo;
        m_dPageDriverBackupInfo = new PageDriverBackupInfo;
        m_dPageDriverRestoreInfo = new PageDriverRestoreInfo;
    }
    void TearDown()
    {
    }
    PageDriverInstallInfo *m_dPageDriverInstallInfo;
    PageDriverBackupInfo *m_dPageDriverBackupInfo;
    PageDriverRestoreInfo *m_dPageDriverRestoreInfo;
};

TEST_F(PageDriverInfo_UT, ut_addDriverInfoToTableView)
{
    DriverInfo *info = new DriverInfo();
    info->m_Name = "Sharp MX-C2622R PS, 1.1";
    info->m_DebVersion = "1.0.0";
    info->m_Type = DR_Tablet;
    info->m_Packages = "com.sharp.griffin2light";
    info->m_Status = ST_DRIVER_IS_NEW;

    DriverInfo *info1 = new DriverInfo();
    info1->m_Name = "Sharp MX-C2622R PS, 1.1";
    info1->m_DebVersion = "1.0.0";
    info1->m_DebBackupVersion = "0.0.9";
    info1->m_Type = DR_Tablet;
    info1->m_Packages = "com.sharp.griffin2light";
    info1->m_Status = ST_CAN_UPDATE;

    DriverInfo *info2 = new DriverInfo();
    info2->m_Name = "GeFore RTX30 Series (Notebooks) 001";
    info2->m_Size = "124.36";
    info2->m_Type = DR_Gpu;
    info2->m_Status = ST_NOT_INSTALL;
    info2->m_Checked = true;
    info2->m_DriverName = "lenovo-image-g-series";
    info2->m_Packages = "lenovo-image-g-series";
    info2->m_DebVersion = "1.0-17";

    m_dPageDriverInstallInfo->addDriverInfoToTableView(info, 0);
    m_dPageDriverInstallInfo->addDriverInfoToTableView(info1, 0);
    m_dPageDriverInstallInfo->addDriverInfoToTableView(info2, 0);
    EXPECT_EQ(1, m_dPageDriverInstallInfo->mp_ViewNotInstall->model()->rowCount());
    EXPECT_EQ(1, m_dPageDriverInstallInfo->mp_ViewCanUpdate->model()->rowCount());
    EXPECT_EQ(1, m_dPageDriverInstallInfo->mp_AllDriverIsNew->model()->rowCount());

    m_dPageDriverBackupInfo->addDriverInfoToTableView(info, 0);
    m_dPageDriverBackupInfo->addDriverInfoToTableView(info1, 0);
    m_dPageDriverBackupInfo->addDriverInfoToTableView(info2, 0);
    EXPECT_EQ(2, m_dPageDriverBackupInfo->mp_ViewBackable->model()->rowCount());
    EXPECT_EQ(1, m_dPageDriverBackupInfo->mp_ViewBackedUp->model()->rowCount());

    m_dPageDriverRestoreInfo->addDriverInfoToTableView(info1, 0);
    EXPECT_EQ(1, m_dPageDriverRestoreInfo->mp_ViewBackable->model()->rowCount());
}

TEST_F(PageDriverInfo_UT, ut_addCurDriverInfo)
{
    DriverInfo *info = new DriverInfo();
    info->m_Name = "Sharp MX-C2622R PS, 1.1";
    info->m_DebVersion = "1.0.0";
    info->m_Type = DR_Tablet;
    info->m_Packages = "com.sharp.griffin2light";
    info->m_Status = ST_DRIVER_IS_NEW;

    m_dPageDriverInstallInfo->addCurDriverInfo(info);
}

TEST_F(PageDriverInfo_UT, ut_showTables)
{
    m_dPageDriverInstallInfo->showTables(1, 1, 1);
    m_dPageDriverBackupInfo->showTables(1, 1);
    m_dPageDriverRestoreInfo->showTables(1);
}

TEST_F(PageDriverInfo_UT, ut_getCheckedDriverIndex)
{
    QList<int> list;
    m_dPageDriverInstallInfo->getCheckedDriverIndex(list);
    EXPECT_EQ(0, list.size());

    m_dPageDriverBackupInfo->getCheckedDriverIndex(list);
    EXPECT_EQ(0, list.size());
}
