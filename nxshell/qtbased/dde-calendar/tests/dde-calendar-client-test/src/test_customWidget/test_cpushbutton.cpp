// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

//#include "test_cpushbutton.h"

//test_cpushbutton::test_cpushbutton(QObject *parent) : QObject(parent)
//{

//}

//TEST_F(test_cpushbutton, setHighlight_001)
//{
//    mWidget->setHighlight(true);
//    EXPECT_TRUE(mWidget->m_Highlighted);
//}

//TEST_F(test_cpushbutton, setHighlight_002)
//{
//    mWidget->setHighlight(false);
//    EXPECT_FALSE(mWidget->m_Highlighted);
//}

//TEST_F(test_cpushbutton, isHighlight_001)
//{
//    mWidget->setHighlight(true);
//    EXPECT_TRUE(mWidget->isHighlight());
//}

//TEST_F(test_cpushbutton, isHighlight_002)
//{
//    mWidget->setHighlight(false);
//    EXPECT_FALSE(mWidget->isHighlight());
//}

//TEST_F(test_cpushbutton, mousePressEvent_001)
//{
//    QMouseEvent *e = new QMouseEvent(QEvent::MouseMove, QPointF(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
//    mWidget->mousePressEvent(e);
//    EXPECT_TRUE(mWidget->m_pressed);
//    delete e;
//}

//TEST_F(test_cpushbutton, mouseReleaseEvent_001)
//{
//    QMouseEvent *e = new QMouseEvent(QEvent::MouseMove, QPointF(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
//    mWidget->mouseReleaseEvent(e);
//    EXPECT_FALSE(mWidget->m_pressed);
//    delete e;
//}

//TEST_F(test_cpushbutton, paintEvent_001)
//{
//    QPaintEvent *e = new QPaintEvent(QRect());
//    mWidget->setHighlight(true);
//    mWidget->paintEvent(e);
//    delete e;
//}

//TEST_F(test_cpushbutton, paintEvent_002)
//{
//    QPaintEvent *e = new QPaintEvent(QRect());
//    mWidget->setHighlight(false);
//    mWidget->paintEvent(e);
//    delete e;
//}
