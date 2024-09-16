// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "closebutton.h"

#include <gtest/gtest.h>

class ut_closebutton_test : public ::testing::Test
{
    // Test interface
protected:
    void SetUp() override;
    void TearDown() override;

    CloseButton *m_testModule = nullptr;
};

void ut_closebutton_test::SetUp()
{
    m_testModule = new CloseButton();
}

void ut_closebutton_test::TearDown()
{
    delete m_testModule;
}

TEST_F(ut_closebutton_test, keyPressEvent)
{
    ASSERT_TRUE(m_testModule);
    EXPECT_EQ(51,m_testModule->width());
    EXPECT_EQ(51,m_testModule->height());
    EXPECT_FALSE(m_testModule->beFocused);
    QKeyEvent keyEvent(QEvent::KeyPress,Qt::Key_Return,Qt::NoModifier);
    m_testModule->keyPressEvent(&keyEvent);
}

TEST_F(ut_closebutton_test, paintEvent)
{
    QPaintEvent event(QRect(0,0,m_testModule->width(),m_testModule->height()));
    m_testModule->beFocused = true;
    m_testModule->paintEvent(&event);
}

TEST_F(ut_closebutton_test, mousePressEvent)
{
    QMouseEvent *event = new QMouseEvent(QEvent::Type::MouseButtonPress, QPointF(100, 100),
                                         Qt::MouseButton::LeftButton, Qt::MouseButton::LeftButton,
                                         Qt::KeyboardModifier::NoModifier);
    m_testModule->mousePressEvent(event);
    delete event;
}
