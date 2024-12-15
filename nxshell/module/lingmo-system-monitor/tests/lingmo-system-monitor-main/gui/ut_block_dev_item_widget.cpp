// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//Self
#include "block_dev_item_widget.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include "addr_pri.h"

//Qt
#include <QSignalSpy>
#include <QPainter>
#include <QResizeEvent>

/***************************************STUB begin*********************************************/

/***************************************STUB end**********************************************/

class UT_BlockDevItemWidget : public ::testing::Test
{
public:
    UT_BlockDevItemWidget() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new BlockDevItemWidget();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    BlockDevItemWidget *m_tester;
};

TEST_F(UT_BlockDevItemWidget, initTest)
{

}

TEST_F(UT_BlockDevItemWidget, test_paintEvent_01)
{
    m_tester->m_isActive = true;
    m_tester->m_mode = BlockDevItemWidget::TitleStyle::TITLE_VERTICAL;
    EXPECT_FALSE(m_tester->grab().isNull());
}

TEST_F(UT_BlockDevItemWidget, test_paintEvent_02)
{
    m_tester->m_isActive = false;
    m_tester->m_mode = BlockDevItemWidget::TitleStyle::TITLE_VERTICAL;
    EXPECT_FALSE(m_tester->grab().isNull());
}

TEST_F(UT_BlockDevItemWidget, test_paintEvent_03)
{
    m_tester->m_isActive = true;
    m_tester->m_mode = BlockDevItemWidget::TitleStyle::TITLE_HORIZONTAL;
    EXPECT_FALSE(m_tester->grab().isNull());
}

TEST_F(UT_BlockDevItemWidget, test_paintEvent_04)
{
    m_tester->m_isActive = false;
    m_tester->m_mode = BlockDevItemWidget::TitleStyle::TITLE_HORIZONTAL;
    EXPECT_FALSE(m_tester->grab().isNull());
}

TEST_F(UT_BlockDevItemWidget, test_resizeEvent_01)
{
    static QResizeEvent re(QSize(10, 10), QSize(20, 20));
    m_tester->resizeEvent(&re);
}

TEST_F(UT_BlockDevItemWidget, test_mousePressEvent_01)
{
    static QMouseEvent me(QEvent::MouseButtonPress, QPointF(1.0, 1.0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_tester->mousePressEvent(&me);

    QSignalSpy signalSpy(m_tester, &BlockDevItemWidget::clicked);
    EXPECT_TRUE(signalSpy.count() == 0);

    emit m_tester->clicked(m_tester->m_blokeDeviceInfo.deviceName());
    signalSpy.wait(50);
    EXPECT_TRUE(signalSpy.count() == 1);
}


TEST_F(UT_BlockDevItemWidget, test_fontChanged_01)
{
    QFont font;
    font.setBold(true);
    m_tester->fontChanged(font);
    EXPECT_EQ(m_tester->m_font.bold(), font.bold());
}

TEST_F(UT_BlockDevItemWidget, test_activeItemWidget_01)
{
    m_tester->activeItemWidget(false);

    EXPECT_EQ(m_tester->m_isActive, false);
}

TEST_F(UT_BlockDevItemWidget, test_updateData_01)
{
    BlockDevice info;
    m_tester->updateData(info);

    EXPECT_EQ(m_tester->m_blokeDeviceInfo.readSpeed(), info.readSpeed());
}

TEST_F(UT_BlockDevItemWidget, test_setMode_01)
{
    int mode = 1;
    m_tester->setMode(mode);

    EXPECT_EQ(m_tester->m_mode, mode);
}

TEST_F(UT_BlockDevItemWidget, test_isActiveItem_01)
{
    m_tester->m_isActive = false;
    EXPECT_FALSE(m_tester->isActiveItem());
}

TEST_F(UT_BlockDevItemWidget, test_updateWidgetGeometry_01)
{
    m_tester->m_mode = BlockDevItemWidget::TitleStyle::TITLE_HORIZONTAL;
    m_tester->updateWidgetGeometry();

    EXPECT_EQ(m_tester->m_mode, BlockDevItemWidget::TitleStyle::TITLE_HORIZONTAL);
}

TEST_F(UT_BlockDevItemWidget, test_updateWidgetGeometry_02)
{
    m_tester->m_mode = BlockDevItemWidget::TitleStyle::TITLE_VERTICAL;
    m_tester->updateWidgetGeometry();

    EXPECT_EQ(m_tester->m_mode, BlockDevItemWidget::TitleStyle::TITLE_VERTICAL);
}
