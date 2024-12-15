// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//Self
#include "system_service_page_widget.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include "addr_pri.h"

//Qt
#include <QSignalSpy>


/***************************************STUB begin*********************************************/

/***************************************STUB end**********************************************/

class UT_SystemServicePageWidget : public ::testing::Test
{
public:
    UT_SystemServicePageWidget() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        static QWidget wid;
        m_tester = new SystemServicePageWidget(&wid);
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    SystemServicePageWidget *m_tester;
};

TEST_F(UT_SystemServicePageWidget, initTest)
{

}

TEST_F(UT_SystemServicePageWidget, test_paintEvent_01)
{
    EXPECT_TRUE(!m_tester->grab().isNull());
}

