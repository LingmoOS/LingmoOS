// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "loadingpage.h"

#include "gtest/src/stub.h"

#include <gtest/gtest.h>
#include <QTest>

/*******************************函数打桩************************************/
// 对DSpinner的start进行打桩
void dSpinner_start_stub()
{
    return;
}

// 对DSpinner的start进行打桩
void dSpinner_stop_stub()
{
    return;
}
/*******************************函数打桩************************************/

class UT_LoadingPage : public ::testing::Test
{
public:
    UT_LoadingPage(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new LoadingPage;
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    LoadingPage *m_tester;
};

TEST_F(UT_LoadingPage, initTest)
{

}

TEST_F(UT_LoadingPage, test_startLoading)
{
    Stub stub;
    stub.set(ADDR(DSpinner, start), dSpinner_start_stub);
    m_tester->startLoading();
}

TEST_F(UT_LoadingPage, test_stopLoading)
{
    Stub stub;
    stub.set(ADDR(DSpinner, stop), dSpinner_stop_stub);
    m_tester->stopLoading();
}

TEST_F(UT_LoadingPage, test_setDes)
{
    m_tester->setDes("123");
    EXPECT_EQ(m_tester->m_pTextLbl->text(), "123");
}
