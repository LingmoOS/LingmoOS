// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "detailwidgetmanager.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

/***************************************STUB begin*********************************************/
/***************************************STUB end**********************************************/

class UT_DetailWidgetManager: public ::testing::Test
{
public:
    UT_DetailWidgetManager() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new DetailWidgetManager();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    DetailWidgetManager *m_tester;
};

TEST_F(UT_DetailWidgetManager, initTest)
{
}

TEST_F(UT_DetailWidgetManager, test_getInstance)
{
    m_tester->getInstance();
}

TEST_F(UT_DetailWidgetManager, test_jumpDetailWidget)
{
    m_tester->jumpDetailWidget("MSG_PROCESS");
}

TEST_F(UT_DetailWidgetManager, test_jumpProcessWidget)
{
    m_tester->jumpProcessWidget("MSG_PROCESS");
}

