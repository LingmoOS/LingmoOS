// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "basewidget.h"

#include <gtest/gtest.h>
#include <QKeyEvent>
#include <QMouseEvent>

class ut_basewidget_test : public ::testing::Test
{
    // Test interface
protected:
    void SetUp() override;
    void TearDown() override;

    BaseWidget *m_testModule = nullptr;
};

void ut_basewidget_test::SetUp()
{
    m_testModule = new BaseWidget();
}

void ut_basewidget_test::TearDown()
{
    delete m_testModule;
}

TEST_F(ut_basewidget_test, setLayoutSpacing)
{
    ASSERT_TRUE(m_testModule);
    ASSERT_TRUE(m_testModule->m_borderWidget);
    ASSERT_TRUE(m_testModule->m_title);
    ASSERT_TRUE(m_testModule->m_layout);
    m_testModule->setLayoutSpacing(10);
    EXPECT_EQ(10,m_testModule->m_layout->spacing());
    m_testModule->setLayoutSpacing(5);
}

TEST_F(ut_basewidget_test, updateInterface)
{
    float f = 1.0;
    m_testModule->updateInterface(f);
    EXPECT_EQ(m_testModule->m_borderWidget->m_size,
              m_testModule->m_borderWidget->m_pixmap.size());
    float f1 = 2.0;
    m_testModule->updateInterface(f);
    EXPECT_EQ(m_testModule->m_borderWidget->m_size,
              m_testModule->m_borderWidget->m_pixmap.size());
}

TEST_F(ut_basewidget_test, setBigPixmap)
{
    m_testModule->setBigPixmap("test icon");
    EXPECT_EQ(m_testModule->m_borderWidget->m_size,
              m_testModule->m_borderWidget->m_pixmap.size());
}

TEST_F(ut_basewidget_test, setSmallPixmap)
{
    m_testModule->setSmallPixmap("test icon");
    EXPECT_EQ(m_testModule->m_borderWidget->m_size,
              m_testModule->m_borderWidget->m_pixmap.size());
}

TEST_F(ut_basewidget_test, setPixmap)
{
    QString testicon = ":/resources/effective_mode_big@3x.png";
    m_testModule->setPixmap(QPixmap(DHiDPIHelper::loadNxPixmap(testicon)));
}

TEST_F(ut_basewidget_test, setTitle)
{
    m_testModule->setTitle("base widget test");
    EXPECT_EQ("base widget test",m_testModule->m_title->text());
}

TEST_F(ut_basewidget_test, setChecked)
{
    m_testModule->setChecked(false);
    EXPECT_FALSE(m_testModule->m_borderWidget->m_checked);
    m_testModule->setChecked(true);
    EXPECT_TRUE(m_testModule->m_borderWidget->m_checked);
}

TEST_F(ut_basewidget_test, mouseReleaseEvent)
{
    QMouseEvent *event = new QMouseEvent(QEvent::Type::MouseButtonPress, QPointF(100, 100),
                                         Qt::MouseButton::LeftButton, Qt::MouseButton::LeftButton,
                                         Qt::KeyboardModifier::NoModifier);
    m_testModule->mouseReleaseEvent(event);
    delete event;
}

TEST_F(ut_basewidget_test, mouseDoubleClickEvent)
{
    QMouseEvent *event;
    m_testModule->mouseDoubleClickEvent(event);
}

TEST_F(ut_basewidget_test, resizeEvent)
{
    QResizeEvent *e;
    m_testModule->resizeEvent(e);
}
