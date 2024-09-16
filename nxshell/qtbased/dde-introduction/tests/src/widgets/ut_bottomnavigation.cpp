// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bottomnavigation.h"

#include <gtest/gtest.h>

class ut_bottomnavigation_test : public ::testing::Test
{
    // Test interface
protected:
    void SetUp() override;
    void TearDown() override;

    BottomNavigation *m_testModule = nullptr;
};

void ut_bottomnavigation_test::SetUp()
{
    m_testModule = new BottomNavigation();
}

void ut_bottomnavigation_test::TearDown()
{
    delete m_testModule;
}

TEST_F(ut_bottomnavigation_test, onButtonClicked)
{
    ASSERT_TRUE(m_testModule);
    EXPECT_TRUE(m_testModule->m_buttons.size() > 0);
    m_testModule->onButtonClicked();
}


TEST_F(ut_bottomnavigation_test, eventFilter)
{
    QObject *watched;
    QEvent event(QEvent::Enter);
    m_testModule->eventFilter(watched,&event);
    EXPECT_EQ(Qt::PointingHandCursor,m_testModule->cursor());

    QEvent event1(QEvent::Leave);
    m_testModule->eventFilter(watched,&event1);
    EXPECT_EQ(Qt::ArrowCursor,m_testModule->cursor());

}
