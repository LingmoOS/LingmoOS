// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//Self
#include "base/base_commandlink_button.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include "addr_pri.h"

//Qt
#include <QSignalSpy>
#include <QKeyEvent>
#include <QMouseEvent>

/***************************************STUB begin*********************************************/

/***************************************STUB end**********************************************/


class UT_BaseCommandLinkButton : public ::testing::Test
{
public:
    UT_BaseCommandLinkButton() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        static QWidget wid;
        m_tester = new BaseCommandLinkButton("1", &wid);
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    BaseCommandLinkButton *m_tester;
};

TEST_F(UT_BaseCommandLinkButton, initTest)
{

}

TEST_F(UT_BaseCommandLinkButton, test_eventFilter_01)
{
    static QKeyEvent ev(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);


    EXPECT_TRUE(m_tester->eventFilter(m_tester, &ev));
}

TEST_F(UT_BaseCommandLinkButton, test_eventFilter_02)
{
    static QMouseEvent ev(QEvent::MouseButtonPress, QPointF(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);


    EXPECT_FALSE(m_tester->eventFilter(m_tester, &ev));
}
