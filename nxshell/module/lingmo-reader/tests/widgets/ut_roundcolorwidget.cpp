// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "RoundColorWidget.h"

#include <QSignalSpy>

#include <gtest/gtest.h>

class UT_RoundColorWidget : public ::testing::Test
{
public:
    UT_RoundColorWidget(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new RoundColorWidget(Qt::red);
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    RoundColorWidget *m_tester;
};

TEST_F(UT_RoundColorWidget, initTest)
{

}

TEST_F(UT_RoundColorWidget, UT_RoundColorWidget_setSelected_001)
{
    m_tester->setSelected(true);
    EXPECT_TRUE(m_tester->m_isSelected == true);
}

TEST_F(UT_RoundColorWidget, UT_RoundColorWidget_setSelected_002)
{
    m_tester->setSelected(false);
    EXPECT_TRUE(m_tester->m_isSelected == false);
}

TEST_F(UT_RoundColorWidget, UT_RoundColorWidget_mousePressEvent)
{
    QSignalSpy spy(m_tester, SIGNAL(clicked()));
    m_tester->m_isSelected = false;
    QMouseEvent *event = new QMouseEvent(QEvent::MouseButtonPress, QPointF(50, 50), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_tester->mousePressEvent(event);
    delete event;
    EXPECT_TRUE(spy.count() == 1);
}

TEST_F(UT_RoundColorWidget, UT_RoundColorWidget_paintEvent)
{
    m_tester->m_isSelected = true;
    QPaintEvent paint(QRect(m_tester->rect()));
    m_tester->paintEvent(&paint);
    EXPECT_FALSE(m_tester->grab().isNull());
}
