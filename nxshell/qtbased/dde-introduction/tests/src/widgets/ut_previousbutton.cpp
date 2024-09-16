// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "previousbutton.h"

#include <gtest/gtest.h>

class ut_previousbutton_test : public ::testing::Test
{
    // Test interface
protected:
    void SetUp() override;
    void TearDown() override;

    previousButton *m_testModule = nullptr;
};

void ut_previousbutton_test::SetUp()
{
    m_testModule = new previousButton();
}

void ut_previousbutton_test::TearDown()
{
    delete m_testModule;
}

TEST_F(ut_previousbutton_test, keyPressEvent)
{
    ASSERT_TRUE(m_testModule);
    EXPECT_EQ(36,m_testModule->width());
    EXPECT_EQ(36,m_testModule->height());

    QKeyEvent keyEvent(QEvent::KeyPress,Qt::Key_Left,Qt::NoModifier);
    m_testModule->keyPressEvent(&keyEvent);

    QKeyEvent keyEvent1(QEvent::KeyPress,Qt::Key_Right,Qt::NoModifier);
    m_testModule->keyPressEvent(&keyEvent1);

    QKeyEvent keyEvent2(QEvent::KeyPress,Qt::Key_Down,Qt::NoModifier);
    m_testModule->keyPressEvent(&keyEvent2);

    QKeyEvent keyEvent3(QEvent::KeyPress,Qt::Key_End,Qt::NoModifier);
    m_testModule->keyPressEvent(&keyEvent3);
}
