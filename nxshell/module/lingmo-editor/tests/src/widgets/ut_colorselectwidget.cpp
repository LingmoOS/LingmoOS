// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_colorselectwidget.h"

#include <QSignalSpy>

// 测试函数ColorLabel::setColorSelected
TEST_F(test_colorlabel, checkSetColorSelected)
{


    auto colorLabel = new ColorLabel(QColor("#FF0000"));
    colorLabel->setColorSelected(true);
    colorLabel->setColorSelected(false);

    EXPECT_NE(colorLabel->m_bSelected,true);
    EXPECT_NE(colorLabel,nullptr);
    colorLabel->deleteLater();

}

// 测试函数 ColorLabel::isSelected
TEST_F(test_colorlabel, checkIsSelected)
{
    auto colorLabel = new ColorLabel(QColor("#FF0000"));
    colorLabel->setColorSelected(true);
    bool result = colorLabel->isSelected();
    EXPECT_TRUE(result);



    colorLabel->setColorSelected(false);
    result = colorLabel->isSelected();
    EXPECT_FALSE(result);
    EXPECT_NE(colorLabel,nullptr);
    colorLabel->deleteLater();

}

// 测试函数 ColorLabel::getColor
TEST_F(test_colorlabel, checkGetColor)
{

    auto colorLabel = new ColorLabel(QColor("#FF0000"));
    QColor colorSet("red");
    QColor colorGet = colorLabel->getColor();
    EXPECT_EQ(colorSet, colorGet);

    EXPECT_NE(colorLabel,nullptr);
    colorLabel->deleteLater();

}

// 测试函数 ColorLabel::paintEvent
TEST_F(test_colorlabel, checkPaintEvent)
{

    auto colorLabel = new ColorLabel(QColor("#FF0000"));
    colorLabel->setColorSelected(true);
    QPaintEvent event(colorLabel->rect());
    colorLabel->paintEvent(&event);
    EXPECT_NE(colorLabel,nullptr);
    colorLabel->deleteLater();



    colorLabel = new ColorLabel(QColor("#FF0000"));
    colorLabel->setColorSelected(false);
    QPaintEvent event2(colorLabel->rect());
    colorLabel->paintEvent(&event2);
    EXPECT_NE(colorLabel,nullptr);
    colorLabel->deleteLater();

}

// 测试函数 ColorLabel::mousePressEvent
TEST_F(test_colorlabel, checkMousePressEvent)
{

    // 场景1: LeftButtonPress
    auto colorLabel = new ColorLabel(QColor("#FF0000"));
    QSignalSpy spy(colorLabel, &ColorLabel::sigColorClicked);
    QMouseEvent *event = new QMouseEvent(QEvent::MouseButtonPress, QPointF(),
                                         Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    colorLabel->mousePressEvent(event);
    EXPECT_EQ(spy.count(), 1);

    EXPECT_NE(colorLabel,nullptr);
    colorLabel->deleteLater();
    EXPECT_NE(event,nullptr);
    delete event;event=nullptr;



    // 场景2: 非LeftButtonPress
    colorLabel = new ColorLabel(QColor("#FF0000"));
    QSignalSpy spy2(colorLabel, &ColorLabel::sigColorClicked);
    event = new QMouseEvent(QEvent::MouseButtonPress, QPointF(),
                            Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    colorLabel->mousePressEvent(event);
    EXPECT_EQ(spy2.count(), 0);

    EXPECT_NE(colorLabel,nullptr);
    colorLabel->deleteLater();
    EXPECT_NE(event,nullptr);
    delete event;event=nullptr;

}

// 测试函数 ColorSelectWdg::initWidget
TEST_F(test_colorselectwidget, checkInitWidget)
{

    // 场景1: 字符串非空
    ColorSelectWdg *colorSelctWidget = new ColorSelectWdg("this is a test");
    colorSelctWidget->initWidget();
    EXPECT_NE(colorSelctWidget,nullptr);
    EXPECT_NE(colorSelctWidget->m_pMainLayout,nullptr);
    colorSelctWidget->deleteLater();



    // 场景2: 字符串为空
    colorSelctWidget = new ColorSelectWdg(QString());
    colorSelctWidget->initWidget();
    EXPECT_NE(colorSelctWidget->m_pHLayout2,nullptr);
    EXPECT_NE(colorSelctWidget,nullptr);
    colorSelctWidget->deleteLater();

}

// 测试函数  ColorSelectWdg::setTheme
TEST_F(test_colorselectwidget, checkSetTheme)
{

    ColorSelectWdg *colorSelctWidget = new ColorSelectWdg("this is a test");
    colorSelctWidget->setTheme("light");
    EXPECT_NE(colorSelctWidget,nullptr);
    colorSelctWidget->deleteLater();



    colorSelctWidget = new ColorSelectWdg("this is a test");
    colorSelctWidget->setTheme("dark");
    EXPECT_NE(colorSelctWidget,nullptr);
    colorSelctWidget->deleteLater();

}

// 测试函数  ColorSelectWdg::getDefaultColor
TEST_F(test_colorselectwidget, checkGetDefaultColor)
{

    ColorSelectWdg *colorSelctWidget = new ColorSelectWdg("this is a test");
    QColor defaultColor = colorSelctWidget->getDefaultColor();
    EXPECT_TRUE(defaultColor.isValid());
    EXPECT_NE(colorSelctWidget,nullptr);
    colorSelctWidget->deleteLater();

}

// 测试函数  ColorSelectWdg::eventFilter
TEST_F(test_colorselectwidget, checkEventFilter)
{

    // 场景2: 过滤m_pLabel的LeftButton事件
    ColorSelectWdg *colorSelctWidget = new ColorSelectWdg("this is a test");
    QMouseEvent *event = new QMouseEvent(QEvent::MouseButtonPress, QPointF(),
                                         Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    bool result = colorSelctWidget->eventFilter(colorSelctWidget->m_pLabel, event);
    EXPECT_TRUE(result);
    EXPECT_NE(colorSelctWidget,nullptr);
    colorSelctWidget->deleteLater();
    delete event;event=nullptr;



    // 场景2: 过滤非m_pLabel的LeftButton事件
    colorSelctWidget = new ColorSelectWdg("this is a test");
    event = new QMouseEvent(QEvent::MouseButtonPress, QPointF(),
                            Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    result = colorSelctWidget->eventFilter(colorSelctWidget->m_pButton, event);
    EXPECT_FALSE(result);
    EXPECT_NE(colorSelctWidget,nullptr);
    colorSelctWidget->deleteLater();
    delete event;event=nullptr;



    // 场景3: 过滤m_pLabel的非LeftButton事件
    colorSelctWidget = new ColorSelectWdg("this is a test");
    event = new QMouseEvent(QEvent::MouseButtonPress, QPointF(),
                            Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    result = colorSelctWidget->eventFilter(colorSelctWidget->m_pLabel, event);
    EXPECT_FALSE(result);
    EXPECT_NE(colorSelctWidget,nullptr);
    colorSelctWidget->deleteLater();
    delete event;event=nullptr;

}
