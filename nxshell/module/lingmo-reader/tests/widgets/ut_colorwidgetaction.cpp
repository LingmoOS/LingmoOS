// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ColorWidgetAction.h"

#include "stub.h"

#include <gtest/gtest.h>
#include <QTest>
#include <QSignalSpy>

class TestColorWidgetAction : public ::testing::Test
{
public:
    TestColorWidgetAction(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new ColorWidgetAction();
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    ColorWidgetAction *m_tester;
};

TEST_F(TestColorWidgetAction, initTest)
{

}

TEST_F(TestColorWidgetAction, testslotBtnClicked)
{
    QSignalSpy spy(m_tester, SIGNAL(sigBtnGroupClicked()));
    m_tester->slotBtnClicked(0);
    EXPECT_EQ(spy.count(), 1);
}
