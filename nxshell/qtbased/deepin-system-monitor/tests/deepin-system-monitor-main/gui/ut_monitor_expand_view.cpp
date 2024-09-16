// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//Self
#include "monitor_expand_view.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include "addr_pri.h"

//Qt
#include <QSignalSpy>

/***************************************STUB begin*********************************************/

/***************************************STUB end**********************************************/

class UT_MonitorExpandView : public ::testing::Test
{
public:
    UT_MonitorExpandView() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        static QWidget parent;
        m_tester = new MonitorExpandView(&parent);
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    MonitorExpandView *m_tester;
};

TEST_F(UT_MonitorExpandView, initTest)
{

}

TEST_F(UT_MonitorExpandView, test_setDetailButtonVisible_01)
{
    m_tester->setDetailButtonVisible(true);
}
