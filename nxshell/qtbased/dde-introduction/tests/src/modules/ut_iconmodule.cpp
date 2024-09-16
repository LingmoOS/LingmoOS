// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_iconmodule.h"

#define protected public
#include "iconmodule.h"
#undef protected

ut_iconmodule_test::ut_iconmodule_test()
{

}

TEST_F(ut_iconmodule_test, updateBigIcon)
{
    IconModule* test_module = new IconModule();
    ASSERT_TRUE(test_module);
    ASSERT_TRUE(test_module->m_scroll);
    ASSERT_TRUE(test_module->m_layout);
    ASSERT_TRUE(test_module->m_scrollWidget);
    test_module->updateBigIcon();
    EXPECT_EQ(700,test_module->width());
    EXPECT_EQ(278,test_module->height());
    EXPECT_EQ(700,test_module->m_scroll->width());
    EXPECT_EQ(278,test_module->m_scroll->height());
    EXPECT_EQ(700,test_module->m_scrollWidget->width());
    EXPECT_EQ(71,test_module->m_height);

    test_module->deleteLater();
}

TEST_F(ut_iconmodule_test, addIcon)
{
    IconModule* test_module = new IconModule();
    //模拟数据
    IconStruct tmp;
    tmp.Id = "1";
    tmp.Path = "333";
    tmp.Deletable = false;
    tmp.Pixmap = "pix";
    test_module->addIcon(tmp);
    EXPECT_TRUE(test_module->m_iconList.size() > 0);

    //模拟数据二
    IconStruct tmp2;
    tmp2.Id = "2";
    tmp2.Path = "444";
    tmp2.Deletable = false;
    tmp2.Pixmap = "pix2";
    test_module->addIcon(tmp2);
    EXPECT_TRUE(test_module->m_iconList.size() > 1);

    test_module->deleteLater();
}

TEST_F(ut_iconmodule_test, removeIcon)
{
    IconModule* test_module = new IconModule();
    //模拟数据
    IconStruct tmp;
    tmp.Id = "1";
    tmp.Path = "333";
    tmp.Deletable = false;
    tmp.Pixmap = "pix";
    test_module->addIcon(tmp);
    int dataSize = test_module->m_iconList.size();
    test_module->removeIcon(tmp);
    EXPECT_NE(dataSize, test_module->m_iconList.size());
    EXPECT_TRUE(test_module->m_iconList.size() < dataSize);
    test_module->deleteLater();
}

TEST_F(ut_iconmodule_test, updateSmallIcon)
{
    IconModule* test_module = new IconModule();
    test_module->updateSmallIcon();
    EXPECT_EQ(550,test_module->width());
    EXPECT_EQ(270,test_module->height());
    EXPECT_EQ(550,test_module->m_scroll->width());
    EXPECT_EQ(270,test_module->m_scroll->height());
    EXPECT_EQ(550,test_module->m_scrollWidget->width());
    EXPECT_EQ(60,test_module->m_height);

    test_module->deleteLater();
}

TEST_F(ut_iconmodule_test, updateSelectBtnPos)
{
    IconModule* test_module = new IconModule();
    test_module->updateSelectBtnPos();
    test_module->deleteLater();
}

TEST_F(ut_iconmodule_test, keyPressEvent)
{
    IconModule* test_module = new IconModule();
    QKeyEvent keyEvent4(QEvent::KeyPress,Qt::Key_Left,Qt::NoModifier);
    test_module->keyPressEvent(&keyEvent4);

    QKeyEvent keyEvent1(QEvent::KeyPress,Qt::Key_Right,Qt::NoModifier);
    test_module->keyPressEvent(&keyEvent1);

    QKeyEvent keyEvent2(QEvent::KeyPress,Qt::Key_Down,Qt::NoModifier);
    test_module->keyPressEvent(&keyEvent2);

    QKeyEvent keyEvent3(QEvent::KeyPress,Qt::Key_Up,Qt::NoModifier);
    test_module->keyPressEvent(&keyEvent3);
    test_module->deleteLater();
}

TEST_F(ut_iconmodule_test, eventFilter)
{
    IconModule* test_module = new IconModule();
    QWidget* testWidget = new QWidget();
    QEvent event(QEvent::Resize);
    ASSERT_TRUE(testWidget);
    test_module->eventFilter(testWidget, &event);
    test_module->deleteLater();
    testWidget->deleteLater();
}

TEST_F(ut_iconmodule_test, mouseMoveEvent)
{
    IconModule* test_module = new IconModule();
    QMouseEvent* event = new QMouseEvent(QEvent::Type::MouseButtonPress, QPointF(100, 100),
                                         Qt::MouseButton::LeftButton, Qt::MouseButton::LeftButton,
                                         Qt::KeyboardModifier::NoModifier);
    test_module->mouseMoveEvent(event);
    test_module->deleteLater();
    delete event;
}

TEST_F(ut_iconmodule_test, mousePressEvent)
{
    IconModule* test_module = new IconModule();
    QMouseEvent* event = new QMouseEvent(QEvent::Type::MouseButtonPress, QPointF(100, 100),
                                        Qt::MouseButton::LeftButton, Qt::MouseButton::LeftButton,
                                        Qt::KeyboardModifier::NoModifier);
    test_module->mousePressEvent(event);
    EXPECT_EQ(QPointF(100, 100),test_module->m_TempPoint);
    delete event;
    test_module->deleteLater();
}

TEST_F(ut_iconmodule_test, currentIconChanged)
{
    IconModule* test_module = new IconModule();
    for (const IconStruct &icon : test_module->m_model->iconList()) {
        test_module->addIcon(icon);
    }
    IconStruct tmp;
    tmp.Id = "Vintage";
    tmp.Path = "/usr/share/icons/bloom-dark";
    tmp.Deletable = false;
    test_module->m_model->m_iconList.append(tmp);
    test_module->m_model->m_currentIcon = tmp.Id;
    test_module->currentIconChanged(test_module->m_model->currentIcon());
    test_module->deleteLater();
}
