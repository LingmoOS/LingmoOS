// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "GetDriverNameModel.h"
#include "ut_Head.h"
#include "stub.h"

#include <DStackedWidget>
#include <DLabel>

#include <QDir>
#include <QFileIconProvider>

#include <gtest/gtest.h>


class UT_GetDriverNameModel : public UT_HEAD
{
public:
    void SetUp()
    {
        m_GetDriverNameModel = new GetDriverNameModel;
    }
    void TearDown()
    {
        delete m_GetDriverNameModel;
    }

    GetDriverNameModel *m_GetDriverNameModel;
};

TEST_F(UT_GetDriverNameModel, UT_GetDriverNameModel_stopLoadingDrivers)
{
    m_GetDriverNameModel->stopLoadingDrivers();
    EXPECT_TRUE(m_GetDriverNameModel->m_Stop);
}

TEST_F(UT_GetDriverNameModel, UT_GetDriverNameModel_startLoadDrivers_001)
{
    QStandardItemModel *model = new QStandardItemModel();
    m_GetDriverNameModel->startLoadDrivers(model, true, "/home");

    EXPECT_LE(0, m_GetDriverNameModel->mp_driverPathList.size());
    EXPECT_LE(0, m_GetDriverNameModel->mp_driversList.size());

    delete model;
}

TEST_F(UT_GetDriverNameModel, UT_GetDriverNameModel_startLoadDrivers_002)
{
    QStandardItemModel *model = new QStandardItemModel();
    m_GetDriverNameModel->startLoadDrivers(model, false, "/home");

    EXPECT_LE(0, m_GetDriverNameModel->mp_driverPathList.size());
    EXPECT_LE(0, m_GetDriverNameModel->mp_driversList.size());
    delete model;
}

TEST_F(UT_GetDriverNameModel, UT_GetDriverNameModel_traverseFolders_001)
{
    m_GetDriverNameModel->traverseFolders("/home");

    m_GetDriverNameModel->mp_driversList << "a.ko" << "b.ko";

    EXPECT_LE(2, m_GetDriverNameModel->mp_driversList.size());
}

TEST_F(UT_GetDriverNameModel, UT_GetDriverNameModel_traverseFolders_002)
{
    m_GetDriverNameModel->traverseFolders("/home/tets");

    EXPECT_LE(0, m_GetDriverNameModel->mp_driversList.size());
}
