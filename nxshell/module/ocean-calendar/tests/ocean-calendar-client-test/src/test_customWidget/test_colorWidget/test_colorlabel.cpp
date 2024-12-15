// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

//#include "test_colorlabel.h"

//test_colorlabel::test_colorlabel(QObject *parent) : QObject(parent)
//{

//}

//TEST_F(test_colorlabel, getColor_001)
//{
//    ColorLabel *colorLabel = new ColorLabel();
//    EXPECT_EQ(colorLabel->getColor(0, 0, 0).name(), "#000000");
//    EXPECT_EQ(colorLabel->getColor(1, 0.5, 0.5).name(), "#3f7e7f");;
//    EXPECT_EQ(colorLabel->getColor(1, 0.2, 0.5).name(), "#667f7f");
//    EXPECT_EQ(colorLabel->getColor(1, 0.1, 0.9).name(), "#cee5e5");
//    EXPECT_EQ(colorLabel->getColor(1, 0.3, 0.4).name(), "#476566");
//    EXPECT_EQ(colorLabel->getColor(2, 0.5, 0.5).name(), "#3f7d7f");
//    EXPECT_EQ(colorLabel->getColor(0.5, 0.5, 0.5).name(), "#3f7e7f");
//    EXPECT_EQ(colorLabel->getColor(5, 0.5, 0.5).name(), "#3f7a7f");
//    EXPECT_EQ(colorLabel->getColor(1, 0.5, 0.5).name(), "#3f7e7f");
//    EXPECT_EQ(colorLabel->getColor(10, 0, 0).name(), "#000000");
//    EXPECT_EQ(colorLabel->getColor(60, 0, 0).name(), "#000000");
//    EXPECT_EQ(colorLabel->getColor(120, 0, 0).name(), "#000000");
//    EXPECT_EQ(colorLabel->getColor(180, 0, 0).name(), "#000000");
//    EXPECT_EQ(colorLabel->getColor(241, 0, 0).name(), "#000000");
//    EXPECT_EQ(colorLabel->getColor(305, 0, 0).name(), "#000000");
//    EXPECT_EQ(colorLabel->getColor(360, 0, 0).name(), "#000000");
//    delete colorLabel;
//}

//TEST_F(test_colorlabel, setHue_001)
//{
//    ColorLabel *colorLabel = new ColorLabel();
//    colorLabel->setHue(1);
//    EXPECT_EQ(1, colorLabel->m_hue);
//    colorLabel->setHue(5);
//    EXPECT_EQ(5, colorLabel->m_hue);
//    colorLabel->setHue(100);
//    EXPECT_EQ(100, colorLabel->m_hue);
//    delete colorLabel;
//}

//TEST_F(test_colorlabel, pickColor)
//{
//    ColorLabel *colorLabel = new ColorLabel();
//    colorLabel->pickColor(QPoint(), 0);
//    delete colorLabel;
//}

//TEST_F(test_colorlabel, paintEvent_001)
//{
//    ColorLabel *colorLabel = new ColorLabel();
//    QPaintEvent *e = new QPaintEvent(QRect());
//    colorLabel->m_entered = true;
//    colorLabel->paintEvent(e);
//    colorLabel->m_entered = false;
//    colorLabel->paintEvent(e);
//    delete e;
//    delete colorLabel;
//}

//TEST_F(test_colorlabel, mousePressEvent_001)
//{
//    ColorLabel *colorLabel = new ColorLabel();
//    QMouseEvent *e = new QMouseEvent(QEvent::MouseMove, QPointF(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
//    colorLabel->mousePressEvent(e);
//    EXPECT_TRUE(colorLabel->m_pressed);
//    delete e;
//    delete colorLabel;
//}

//TEST_F(test_colorlabel, mouseMoveEvent)
//{
//    ColorLabel *colorLabel = new ColorLabel();
//    QMouseEvent *e = new QMouseEvent(QEvent::MouseMove, QPointF(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
//    colorLabel->mouseMoveEvent(e);
//    EXPECT_TRUE(colorLabel->m_entered);
//    delete e;
//    delete colorLabel;
//}

//TEST_F(test_colorlabel, mouseReleaseEvent_001)
//{
//    ColorLabel *colorLabel = new ColorLabel();
//    QMouseEvent *e = new QMouseEvent(QEvent::MouseMove, QPointF(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
//    colorLabel->mouseReleaseEvent(e);
//    EXPECT_FALSE(colorLabel->m_pressed);
//    delete e;
//    delete colorLabel;
//}

//TEST_F(test_colorlabel, pickColorCursor_001)
//{
//    ColorLabel *colorLabel = new ColorLabel();
//    QCursor cursor = colorLabel->pickColorCursor();
//    EXPECT_NE(cursor, QCursor());
//    delete colorLabel;
//}
