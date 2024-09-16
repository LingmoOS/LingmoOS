// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_normalmodule.h"

#define protected public
#include "normalmodule.h"
#undef protected

ut_normalmodule_test::ut_normalmodule_test()
{
  setlocale(LC_NUMERIC, "C");
}

TEST_F(ut_normalmodule_test, NormalModule)
{
    NormalModule* test_module = new NormalModule();
    ASSERT_TRUE(test_module);
    ASSERT_TRUE(test_module->m_leftNavigationLayout);
    ASSERT_TRUE(test_module->m_rightContentLayout);
    ASSERT_TRUE(test_module->m_buttonGrp);
    ASSERT_FALSE(test_module->m_currentWidget);
    ASSERT_TRUE(test_module->m_wmSwitcher);
    ASSERT_TRUE(test_module->m_titleLabel);
    ASSERT_TRUE(test_module->m_describe);
    ASSERT_TRUE(test_module->m_content);
    ASSERT_TRUE(test_module->m_closeFrame);
    EXPECT_EQ(false,test_module->tab_be_press);
    test_module->deleteLater();
}

TEST_F(ut_normalmodule_test, keyPressEvent)
{
//    NormalModule* test_module = new NormalModule();
//    test_module->m_closeFrame->beFocused = false;
//    test_module->m_index = 2;
//    qint64 tempTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
//    QKeyEvent keyEvent0(QEvent::KeyPress,Qt::Key_Escape,Qt::NoModifier);
//    test_module->keyPressEvent(&keyEvent0);
//    EXPECT_EQ(tempTime,test_module->m_keypressTime);

//    test_module->m_index = 3;
//    test_module->keyPressEvent(&keyEvent0);
//    test_module->m_index = 4;
//    test_module->keyPressEvent(&keyEvent0);

//    test_module->m_index = 1;
//    QKeyEvent keyEvent(QEvent::KeyPress,Qt::Key_Up,Qt::NoModifier);
//    test_module->keyPressEvent(&keyEvent);

//    test_module->m_index = 2;
//    test_module->keyPressEvent(&keyEvent);

//    QKeyEvent keyEvent1(QEvent::KeyPress,Qt::Key_Down,Qt::NoModifier);
//    test_module->m_index = 3;
//    test_module->keyPressEvent(&keyEvent1);

//    test_module->m_index = 4;
//    test_module->keyPressEvent(&keyEvent1);

//    QKeyEvent keyEvent2(QEvent::KeyPress,Qt::Key_Tab,Qt::NoModifier);
//    test_module->keyPressEvent(&keyEvent2);

//    QKeyEvent keyEvent3(QEvent::KeyPress,Qt::Key_Return,Qt::NoModifier);
//    test_module->keyPressEvent(&keyEvent3);
//    test_module->deleteLater();
}

TEST_F(ut_normalmodule_test, mousePressEvent)
{
    NormalModule* test_module = new NormalModule();
    QMouseEvent* event = new QMouseEvent(QEvent::Type::MouseButtonPress, QPointF(100, 100),
                                         Qt::MouseButton::LeftButton, Qt::MouseButton::LeftButton,
                                         Qt::KeyboardModifier::NoModifier);
    test_module->mousePressEvent(event);
    EXPECT_EQ(false,test_module->m_closeFrame->beFocused);
    test_module->deleteLater();
    delete event;
}

TEST_F(ut_normalmodule_test, eventFilter)
{
    NormalModule* test_module = new NormalModule();
    QWidget* testWidget = new QWidget();
    QEvent event(QEvent::KeyPress);
    ASSERT_TRUE(testWidget);
    test_module->eventFilter(testWidget, &event);
    test_module->deleteLater();
    testWidget->deleteLater();
}

TEST_F(ut_normalmodule_test, updateCurrentWidget)
{
    NormalModule* test_module = new NormalModule();
    test_module->updateCurrentWidget(1);
    EXPECT_EQ(1,test_module->m_index);
    test_module->deleteLater();
}

TEST_F(ut_normalmodule_test, RefeshQwidget)
{
    NormalModule* test_module = new NormalModule();
    test_module->m_index = 5;
    test_module->updateCurrentWidget(1);
    test_module->RefeshQwidget();
    EXPECT_EQ(1,test_module->m_index);
    ASSERT_TRUE(test_module->m_currentWidget);
    EXPECT_EQ(549,test_module->m_currentWidget->width());
    EXPECT_EQ(309,test_module->m_currentWidget->height());
    test_module->deleteLater();
}

TEST_F(ut_normalmodule_test, initTheme)
{
    NormalModule* test_module = new NormalModule();
    test_module->initTheme(0);
    test_module->initTheme(2);
    test_module->deleteLater();
}

TEST_F(ut_normalmodule_test, updataButton)
{
    NormalModule* test_module = new NormalModule();
    QPushButton *btn = new QPushButton();
    ASSERT_TRUE(btn);
    ASSERT_TRUE(test_module->m_button);
    test_module->updataButton(btn);
    btn->deleteLater();
    test_module->deleteLater();
}

TEST_F(ut_normalmodule_test, updateLabel)
{
    NormalModule* test_module = new NormalModule();
    test_module->updateLabel();
    test_module->deleteLater();
}

TEST_F(ut_normalmodule_test, updateInterface)
{
    NormalModule* test_module = new NormalModule();
    test_module->m_index = 1;
    test_module->updateInterface(100);

    test_module->m_index = 2;
    test_module->updateInterface(100);

    test_module->m_index = 1;
    test_module->updateInterface(120);

    test_module->m_index = 2;
    test_module->updateInterface(120);
    test_module->deleteLater();
}

TEST_F(ut_normalmodule_test, cancelCloseFrame)
{
    NormalModule* test_module = new NormalModule();
    test_module->cancelCloseFrame();
    EXPECT_EQ(false,test_module->m_closeFrame->beFocused);
    test_module->deleteLater();
}

TEST_F(ut_normalmodule_test, ClickShow)
{
    NormalModule* test_module = new NormalModule();
    QPushButton *btn = new QPushButton();
    test_module->ClickShow(btn);
    ASSERT_TRUE(btn);
    EXPECT_EQ(false,test_module->m_closeFrame->beFocused);
    btn->deleteLater();
    test_module->deleteLater();
}


