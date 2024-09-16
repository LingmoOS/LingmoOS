// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "navigationbutton.h"
#include "widgets/nextbutton.h"

#include <gtest/gtest.h>

class ut_navigationbutton_test : public ::testing::Test
{
    // Test interface
protected:
    void SetUp() override;
    void TearDown() override;

    NavigationButton *m_testModule = nullptr;
};

void ut_navigationbutton_test::SetUp()
{
    m_testModule = new NavigationButton("just a test");
}

void ut_navigationbutton_test::TearDown()
{
    delete m_testModule;
}

TEST_F(ut_navigationbutton_test, initButton)
{
    ASSERT_TRUE(m_testModule);
    ASSERT_TRUE(m_testModule->m_label);
    EXPECT_EQ("just a test",m_testModule->m_label->text());
    EXPECT_EQ(95,m_testModule->m_label->width());
    EXPECT_EQ(30,m_testModule->m_label->height());
    EXPECT_FALSE(m_testModule->isEnter);
    EXPECT_FALSE(m_testModule->needFrame);
    m_testModule->initButton();
}

TEST_F(ut_navigationbutton_test, mousePressEvent)
{
    QMouseEvent* event = new QMouseEvent(QEvent::Type::MouseButtonPress, QPointF(100, 100),
                                        Qt::MouseButton::LeftButton, Qt::MouseButton::LeftButton,
                                        Qt::KeyboardModifier::NoModifier);
    m_testModule->mousePressEvent(event);
    EXPECT_FALSE(m_testModule->isEnter);
    EXPECT_FALSE(m_testModule->needFrame);
    delete event;
}

TEST_F(ut_navigationbutton_test, enterEvent)
{
    QEvent *event;
    m_testModule->enterEvent(event);
    EXPECT_TRUE(m_testModule->isEnter);
}

TEST_F(ut_navigationbutton_test, leaveEvent)
{
    QEvent *event;
    m_testModule->leaveEvent(event);
    EXPECT_FALSE(m_testModule->isEnter);
}

TEST_F(ut_navigationbutton_test, paintEvent)
{
    QPaintEvent *event;
    QWidget* testWidget = new QWidget();
    m_testModule->isEnter = true;
    m_testModule->needFrame = true;
    NextButton *nextBtn = new NextButton("Next", testWidget);
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::LightType);
    m_testModule->paintEvent(event);

    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::DarkType);
    m_testModule->paintEvent(event);

    testWidget->deleteLater();
    nextBtn->deleteLater();
}
