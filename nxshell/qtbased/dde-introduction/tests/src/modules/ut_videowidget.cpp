// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_videowidget.h"

#define protected public
#include "videowidget.h"
#undef protected

ut_videowidget_test::ut_videowidget_test()
{

}

TEST_F(ut_videowidget_test, VideoWidget)
{
    VideoWidget* test_module = new VideoWidget(false);
    ASSERT_TRUE(test_module);
    ASSERT_TRUE(test_module->m_video);
    ASSERT_TRUE(test_module->m_control);
    ASSERT_TRUE(test_module->m_clip);
    ASSERT_TRUE(test_module->m_btnAni);
    ASSERT_TRUE(test_module->m_hideAni);
    ASSERT_TRUE(test_module->m_leaveTimer);
    ASSERT_TRUE(test_module->m_pauseTimer);
    ASSERT_TRUE(test_module->m_background);
    ASSERT_TRUE(test_module->m_hideEffect);
    EXPECT_TRUE(test_module->m_strVideoCoverIcon != "");
    EXPECT_EQ(48,test_module->m_control->width());
    EXPECT_EQ(48,test_module->m_control->height());
    EXPECT_EQ(false,test_module->m_load);
    test_module->deleteLater();
}


TEST_F(ut_videowidget_test, updateBigIcon)
{
    VideoWidget* test_module = new VideoWidget(false);
    test_module->updateBigIcon();
    EXPECT_EQ(700,test_module->width());
    EXPECT_EQ(450,test_module->height());
    EXPECT_EQ(700,test_module->m_video->width());
    EXPECT_EQ(450,test_module->m_video->height());
    EXPECT_EQ(700,test_module->m_background->width());
    EXPECT_EQ(450,test_module->m_background->height());

    test_module->deleteLater();
}

TEST_F(ut_videowidget_test, updateSmallIcon)
{
    VideoWidget* test_module = new VideoWidget(false);
    test_module->updateSmallIcon();
    EXPECT_EQ(549,test_module->width());
    EXPECT_EQ(309,test_module->height());
    EXPECT_EQ(549,test_module->m_video->width());
    EXPECT_EQ(309,test_module->m_video->height());
    test_module->deleteLater();
}

TEST_F(ut_videowidget_test, updateSelectBtnPos)
{
    VideoWidget* test_module = new VideoWidget(false);
    test_module->updateSelectBtnPos();

    test_module->deleteLater();
}

TEST_F(ut_videowidget_test, updateControlButton)
{
    VideoWidget* test_module = new VideoWidget(false);
    test_module->updateControlButton();
    test_module->deleteLater();
}

TEST_F(ut_videowidget_test, onControlButtonClicked)
{
    VideoWidget* test_module = new VideoWidget(false);
    test_module->onControlButtonClicked();
    test_module->deleteLater();
}

TEST_F(ut_videowidget_test, PauseResetOrPlay)
{
    VideoWidget* test_module = new VideoWidget(false);
    test_module->PauseResetOrPlay();
    test_module->deleteLater();
}


TEST_F(ut_videowidget_test, stop)
{
    VideoWidget* test_module = new VideoWidget(false);
    test_module->stop();

    test_module->deleteLater();
}

TEST_F(ut_videowidget_test, updateInterface)
{
    VideoWidget* test_module = new VideoWidget(false);
    QSize size(700, 450);
    test_module->updateInterface(size);
    EXPECT_EQ(700,test_module->width());
    EXPECT_EQ(450,test_module->height());
    EXPECT_EQ(700,test_module->m_video->width());
    EXPECT_EQ(450,test_module->m_video->height());
    test_module->deleteLater();
}

TEST_F(ut_videowidget_test, showVideoControlButton)
{
    VideoWidget* test_module = new VideoWidget(false);
    test_module->showVideoControlButton();
    EXPECT_FALSE(test_module->m_leaveTimer->isActive());
    test_module->deleteLater();
}

TEST_F(ut_videowidget_test, deleteBackground)
{
    VideoWidget* test_module = new VideoWidget(false);
    test_module->deleteBackground();
    test_module->deleteLater();
}

TEST_F(ut_videowidget_test, enterEvent)
{
    VideoWidget* test_module = new VideoWidget(false);
    QEvent *e;
    test_module->enterEvent(e);
     
    test_module->deleteLater();
}

TEST_F(ut_videowidget_test, leaveEvent)
{
    VideoWidget* test_module = new VideoWidget(false);
    QEvent *e;
    test_module->leaveEvent(e);
     
    test_module->deleteLater();
}

TEST_F(ut_videowidget_test, resizeEvent)
{
    VideoWidget* test_module = new VideoWidget(false);
    QResizeEvent *e;
    test_module->resizeEvent(e);
     
    test_module->deleteLater();
}

TEST_F(ut_videowidget_test, keyPressEvent)
{
    VideoWidget* test_module = new VideoWidget(false);
    QKeyEvent keyEvent(QEvent::Type::KeyPress, Qt::Key_Enter, Qt::KeyboardModifier::NoModifier);
    test_module->keyPressEvent(&keyEvent);

    QKeyEvent keyEvent1(QEvent::Type::KeyPress, Qt::Key_Return, Qt::KeyboardModifier::NoModifier);
    test_module->keyPressEvent(&keyEvent1);
    test_module->deleteLater();
}

TEST_F(ut_videowidget_test, updateClip)
{
    VideoWidget* test_module = new VideoWidget(false);
    test_module->updateClip();    
    test_module->deleteLater();
}

