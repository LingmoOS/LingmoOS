// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//Self
#include "monitor_compact_view.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include "addr_pri.h"

//Qt
#include <QSignalSpy>

/***************************************STUB begin*********************************************/

/***************************************STUB end**********************************************/

class UT_MonitorCompactView : public ::testing::Test
{
public:
    UT_MonitorCompactView() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        static QWidget parent;
        m_tester = new MonitorCompactView(&parent);
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    MonitorCompactView *m_tester;
};

TEST_F(UT_MonitorCompactView, initTest)
{

}


TEST_F(UT_MonitorCompactView, test_setDetailButtonVisible_01)
{
    m_tester->setDetailButtonVisible(true);
}
