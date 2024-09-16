// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bottombutton.h"

#include <gtest/gtest.h>

class ut_bottombutton_test : public ::testing::Test
{
    // Test interface
protected:
    void SetUp() override;
    void TearDown() override;

    BottomButton *m_testModule = nullptr;
};

void ut_bottombutton_test::SetUp()
{
    m_testModule = new BottomButton();
}

void ut_bottombutton_test::TearDown()
{
    delete m_testModule;
}

TEST_F(ut_bottombutton_test, paintEvent)
{
    ASSERT_TRUE(m_testModule);
    //当前页面的大小对比
    EXPECT_EQ(85,m_testModule->width());
    EXPECT_EQ(30,m_testModule->height());
    QPaintEvent* event;
    m_testModule->paintEvent(event);
}

TEST_F(ut_bottombutton_test, setText)
{
    m_testModule->setText("test bottom button");
    EXPECT_EQ("test bottom button",m_testModule->m_text);
}

TEST_F(ut_bottombutton_test, setRect)
{
    m_testModule->setRect(QRect(0,0,m_testModule->width(),m_testModule->height()));
    EXPECT_EQ(QRect(0,0,m_testModule->width(),m_testModule->height()),m_testModule->m_rect);
}
