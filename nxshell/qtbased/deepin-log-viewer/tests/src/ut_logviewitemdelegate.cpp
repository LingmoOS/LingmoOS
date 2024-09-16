// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logviewitemdelegate.h"
#include "logtreeview.h"

#include <gtest/gtest.h>
#include <stub.h>

#include <DStyle>

#include <QDebug>
#include <QPainter>

DWIDGET_USE_NAMESPACE
//int stub_pixelMetric001(DStyle::PixelMetric m, const QStyleOption *opt = nullptr, const QWidget *widget = nullptr)
//{
//    return 1;
//}

//void stub_drawPrimitive001(DStyle::PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w = nullptr)
//{
//}

//bool stub_isValid001()
//{
//}

//class LogViewItemDelegate_paint_UT : public testing::Test
//{
//public:
//    //添加日志
//    static void SetUpTestCase()
//    {
//        qDebug() << "SetUpTestCase" << endl;
//    }
//    static void TearDownTestCase()
//    {
//        qDebug() << "TearDownTestCase" << endl;
//    }
//    void SetUp() //TEST跑之前会执行SetUp
//    {
//        m_treeview = new LogTreeView();
//        p = new LogViewItemDelegate(m_treeview);
//        qDebug() << "SetUp" << endl;
//    }
//    void TearDown() //TEST跑完之后会执行TearDown
//    {
//        delete p;
//    }
//    LogViewItemDelegate *p;
//    LogTreeView *m_treeview;
//};

////TEST_F(LogViewItemDelegate_paint_UT, LogViewItemDelegate_paint_UT)
////{
////    Stub stub;
////    stub.set((int (DStyle::*)(DStyle::PixelMetric, const QStyleOption *, const QWidget *) const)ADDR(DStyle, pixelMetric), stub_pixelMetric001);
////    stub.set((void (DStyle::*)(DStyle::PrimitiveElement, const QStyleOption *, QPainter *, const QWidget *) const)ADDR(DStyle, drawPrimitive), stub_drawPrimitive001);
////    stub.set(ADDR(QModelIndex, isValid), stub_isValid001);
////    LogViewItemDelegate *p = new LogViewItemDelegate(nullptr);
////    EXPECT_NE(p, nullptr);
////    p->paint(new QPainter, QStyleOptionViewItem(), QModelIndex());
////}

//TEST_F(LogViewItemDelegate_paint_UT, LogViewItemDelegate_createEditor_UT)
//{
//    EXPECT_NE(p, nullptr);
//    QWidget *w = p->createEditor(nullptr, QStyleOptionViewItem(), QModelIndex());
//    EXPECT_EQ(w, nullptr);
//}

//TEST_F(LogViewItemDelegate_paint_UT, LogViewItemDelegate_sizeHint_UT)
//{
//    EXPECT_NE(p, nullptr);
//    p->sizeHint(QStyleOptionViewItem(), QModelIndex());
//}

//TEST_F(LogViewItemDelegate_paint_UT, LogViewItemDelegate_initStyleOption_UT)
//{
//    EXPECT_NE(p, nullptr);
//    p->initStyleOption(new QStyleOptionViewItem(), QModelIndex());
//    p->deleteLater();
//}
