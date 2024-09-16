// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//Self
#include "chart_view_widget.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include "addr_pri.h"

//Qt
#include <QSignalSpy>
#include <QResizeEvent>
#include <QPainter>

/***************************************STUB begin*********************************************/

/***************************************STUB end**********************************************/

class UT_ChartViewWidget : public ::testing::Test
{
public:
    UT_ChartViewWidget() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new ChartViewWidget(ChartViewWidget::ChartViewTypes::MEM_CHART, nullptr);
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    ChartViewWidget *m_tester;
};

TEST_F(UT_ChartViewWidget, initTest)
{

}

TEST_F(UT_ChartViewWidget, test_setData1Color_01)
{
    QColor color(15, 15, 15);
    m_tester->setData1Color(color);

    EXPECT_EQ(m_tester->m_data1Color, color);
}


TEST_F(UT_ChartViewWidget, test_addData1_01)
{
    QVariant variant(20);
    m_tester->addData1(variant);

    EXPECT_EQ(m_tester->m_maxData1, QVariant(20 * 1.1).toLongLong());
    EXPECT_EQ(m_tester->m_maxData, QVariant(20 * 1.1).toLongLong());
}

TEST_F(UT_ChartViewWidget, test_addData1_02)
{
    QVariant variant(20);
    for (int i = 0; i < 40; i++) {
        m_tester->m_listData1.append(i);
    }
    m_tester->addData1(variant);
    EXPECT_EQ(m_tester->m_maxData1.toLongLong(), QVariant(39 * 1.1).toLongLong());
    EXPECT_EQ(m_tester->m_maxData.toLongLong(), QVariant(39 * 1.1).toLongLong());
}

TEST_F(UT_ChartViewWidget, test_addData1_03)
{
    QVariant variant(20);
    m_tester->m_speedAxis = true;
    for (int i = 0; i < 40; i++) {
        m_tester->m_listData1.append(i);
    }
    m_tester->addData1(variant);
    EXPECT_EQ(m_tester->m_maxData1.toLongLong(), QVariant(39 * 1.1).toLongLong());
    EXPECT_EQ(m_tester->m_maxData.toLongLong(), QVariant(39 * 1.1).toLongLong());
}

TEST_F(UT_ChartViewWidget, test_addData1_04)
{
    QVariant variant(20);
    m_tester->m_speedAxis = true;
    m_tester->m_maxData1 = 0;
    for (int i = 0; i < 40; i++) {
        m_tester->m_listData1.append(i);
    }
    m_tester->addData1(variant);
    EXPECT_EQ(m_tester->m_maxData1.toLongLong(), QVariant(39 * 1.1).toLongLong());
    EXPECT_EQ(m_tester->m_maxData.toLongLong(), QVariant(39 * 1.1).toLongLong());
}

TEST_F(UT_ChartViewWidget, test_setData2Color_01)
{
    QColor color(15, 15, 15);
    m_tester->setData2Color(color);

    EXPECT_EQ(m_tester->m_data2Color, color);
}

TEST_F(UT_ChartViewWidget, test_addData2_01)
{
    QVariant variant(20);
    m_tester->addData2(variant);

    EXPECT_EQ(m_tester->m_maxData2, QVariant(20 * 1.1).toLongLong());
    EXPECT_EQ(m_tester->m_maxData, QVariant(20 * 1.1).toLongLong());
}

TEST_F(UT_ChartViewWidget, test_addData2_02)
{
    QVariant variant(20);
    for (int i = 0; i < 40; i++) {
        m_tester->m_listData2.append(i);
    }
    m_tester->addData2(variant);
    EXPECT_EQ(m_tester->m_maxData2.toLongLong(), QVariant(39 * 1.1).toLongLong());
    EXPECT_EQ(m_tester->m_maxData.toLongLong(), QVariant(39 * 1.1).toLongLong());
}

TEST_F(UT_ChartViewWidget, test_addData2_03)
{
    QVariant variant(20);
    m_tester->m_speedAxis = true;
    for (int i = 0; i < 40; i++) {
        m_tester->m_listData2.append(i);
    }
    m_tester->addData2(variant);
    EXPECT_EQ(m_tester->m_maxData2.toLongLong(), QVariant(39 * 1.1).toLongLong());
    EXPECT_EQ(m_tester->m_maxData.toLongLong(), QVariant(39 * 1.1).toLongLong());
}

TEST_F(UT_ChartViewWidget, test_addData2_04)
{
    QVariant variant(20);
    m_tester->m_speedAxis = true;
    m_tester->m_maxData2 = 0;
    for (int i = 0; i < 40; i++) {
        m_tester->m_listData2.append(i);
    }
    m_tester->addData2(variant);
    EXPECT_EQ(m_tester->m_maxData2.toLongLong(), QVariant(39 * 1.1).toLongLong());
    EXPECT_EQ(m_tester->m_maxData.toLongLong(), QVariant(39 * 1.1).toLongLong());
}

TEST_F(UT_ChartViewWidget, test_setSpeedAxis_01)
{
    m_tester->setSpeedAxis(true);
    EXPECT_TRUE(m_tester->m_speedAxis);
}


TEST_F(UT_ChartViewWidget, test_paintEvent_01)
{
    EXPECT_FALSE(m_tester->grab().isNull());
}

TEST_F(UT_ChartViewWidget, test_resizeEvent_01)
{
    static QResizeEvent re(QSize(10, 10), QSize(20, 20));
    m_tester->resizeEvent(&re);
}

TEST_F(UT_ChartViewWidget, test_changeFont_01)
{
    QFont font;
    font.setBold(true);
    font.setPointSizeF(10.0);
    m_tester->changeFont(font);

    EXPECT_EQ(m_tester->m_textfont.bold(), font.bold());
    EXPECT_EQ(m_tester->m_textfont.pointSizeF(), font.pointSizeF() -2);
}

TEST_F(UT_ChartViewWidget, test_changeTheme_01)
{
    m_tester->changeTheme();
}

TEST_F(UT_ChartViewWidget, test_drawBackPixmap_01)
{
    m_tester->drawBackPixmap();

    EXPECT_EQ(m_tester->m_backPixmap.size(), m_tester->size());
}

TEST_F(UT_ChartViewWidget, test_drawBackPixmap_02)
{
    m_tester->setGeometry(0, 0, 0, 0);
    m_tester->drawBackPixmap();
    EXPECT_EQ(m_tester->width(), 0);
}

TEST_F(UT_ChartViewWidget, test_drawData1_01)
{
    QPixmap pixmap(100, 100);
    QPainter painter(&pixmap);
    m_tester->drawData1(&painter);

    EXPECT_EQ(m_tester->m_listData1.size(), 0);
}

TEST_F(UT_ChartViewWidget, test_drawData1_02)
{
    for (int i = 0; i < 2; i++)
    {
        m_tester->m_listData1.append(i);
    }
    QPixmap pixmap(100, 100);
    QPainter painter(&pixmap);

    m_tester->drawData1(&painter);
}

TEST_F(UT_ChartViewWidget, test_drawData2_01)
{
    QPixmap pixmap(100, 100);
    QPainter painter(&pixmap);
    m_tester->drawData2(&painter);

    EXPECT_EQ(m_tester->m_listData2.size(), 0);
}

TEST_F(UT_ChartViewWidget, test_drawData2_02)
{
    for (int i = 0; i < 2; i++)
    {
        m_tester->m_listData2.append(i);
    }
    QPixmap pixmap(100, 100);
    QPainter painter(&pixmap);
    m_tester->drawData2(&painter);
}

TEST_F(UT_ChartViewWidget, test_drawAxisText_01)
{
    QPixmap pixmap(100, 100);
    QPainter painter(&pixmap);
    m_tester->drawAxisText(&painter);
}

TEST_F(UT_ChartViewWidget, test_setAxisTitle_01)
{
    QString title = "hello";
    m_tester->setAxisTitle(title);

    EXPECT_EQ(m_tester->m_axisTitle, title);
}

TEST_F(UT_ChartViewWidget, test_getPainterPathByData_01)
{
    QList<QVariant> listData;

    listData << QVariant(10) << QVariant(20) << QVariant(30);
    QPainterPath path;
    QVariant maxYvalue(10);


    m_tester->getPainterPathByData(listData, path, maxYvalue);
}












