// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PageSearchThread.h"
#include "DocSheet.h"
#include "stub.h"

#include <gtest/gtest.h>

class UT_PageSearchThread : public ::testing::Test
{
public:
    UT_PageSearchThread(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new PageSearchThread();
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    PageSearchThread *m_tester;
};

TEST_F(UT_PageSearchThread, initTest)
{

}

TEST_F(UT_PageSearchThread, UT_PageSearchThread_startSearch)
{
    QString strPath = UTSOURCEDIR;
    strPath += "/files/1.pdf";
    DocSheet *sheet = new DocSheet(Dr::PDF, strPath, nullptr);

    m_tester->startSearch(sheet, "123");
    EXPECT_TRUE(m_tester->m_sheet == sheet);
    EXPECT_TRUE(m_tester->m_searchText == "123");
    delete sheet;
}

TEST_F(UT_PageSearchThread, UT_PageSearchThread_stopSearch)
{
    m_tester->stopSearch();
    EXPECT_TRUE(m_tester->m_quit == true);
}

TEST_F(UT_PageSearchThread, UT_PageSearchThread_run)
{
    QString strPath = UTSOURCEDIR;
    strPath += "/files/1.pdf";
    DocSheet *sheet = new DocSheet(Dr::PDF, strPath, nullptr);

    m_tester->startSearch(sheet, "123");
    m_tester->run();
    delete sheet;
}

TEST_F(UT_PageSearchThread, UT_PageSearchThread_initCJKtoKangxi)
{
    m_tester->initCJKtoKangxi();
}
