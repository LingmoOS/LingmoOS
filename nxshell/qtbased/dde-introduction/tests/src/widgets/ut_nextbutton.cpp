// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "nextbutton.h"
#define DEFAULT_BG_COLOR  255, 255, 255, int(1.0 * 255)
#define HOVER_BG_COLOR    255, 255, 255, int(0.3 * 255)
#define PRESS_BG_COLOR    255, 255, 255, int(0.1 * 255)
class ut_nextbutton_test : public ::testing::Test
{
    // Test interface
protected:
    void SetUp() override;
    void TearDown() override;

public:
    NextButton *m_testModule = nullptr;
};

void ut_nextbutton_test::SetUp()
{
    m_testModule = new NextButton("just a test");
}

void ut_nextbutton_test::TearDown()
{
    delete m_testModule;
}

TEST_F(ut_nextbutton_test, setMode)
{
    ASSERT_TRUE(m_testModule);
    EXPECT_EQ("just a test",m_testModule->m_text);
    EXPECT_EQ(QColor(DEFAULT_BG_COLOR),m_testModule->m_currentColor);
    m_testModule->setMode(NextButton::Mode::Normal);
    EXPECT_EQ(NextButton::Mode::Normal,m_testModule->m_mode);
}

TEST_F(ut_nextbutton_test, mousePressEvent)
{
    QMouseEvent *event = new QMouseEvent(QEvent::Type::MouseButtonPress, QPointF(100, 100),
                                         Qt::MouseButton::LeftButton, Qt::MouseButton::LeftButton,
                                         Qt::KeyboardModifier::NoModifier);
    m_testModule->mousePressEvent(event);
    EXPECT_EQ(QColor(PRESS_BG_COLOR),m_testModule->m_currentColor);
    delete event;
}

TEST_F(ut_nextbutton_test, enterEvent)
{
    QEvent *event;
    m_testModule->enterEvent(event);
    EXPECT_EQ(QColor(HOVER_BG_COLOR),m_testModule->m_currentColor);
}

TEST_F(ut_nextbutton_test, leaveEvent)
{
    QEvent *event;
    m_testModule->leaveEvent(event);
    EXPECT_EQ(QColor(DEFAULT_BG_COLOR),m_testModule->m_currentColor);
}

TEST_F(ut_nextbutton_test, keyPressEvent)
{
    QKeyEvent keyEvent(QEvent::KeyPress,Qt::Key_Left,Qt::NoModifier);
    m_testModule->keyPressEvent(&keyEvent);

    QKeyEvent keyEvent1(QEvent::KeyPress,Qt::Key_Return,Qt::NoModifier);
    m_testModule->keyPressEvent(&keyEvent1);
}
