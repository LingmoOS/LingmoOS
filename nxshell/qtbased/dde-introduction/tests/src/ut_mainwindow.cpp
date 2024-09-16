// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_mainwindow.h"

#ifndef DISABLE_VIDEO
#include "videowidget.h"
#endif
ut_mainwindow_test::ut_mainwindow_test()
{

}

void ut_mainwindow_test::SetUp()
{
    //下面删除文件的代码是为了模拟欢迎第一次启动场景
    QString userName = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    userName = userName.section("/", -1, -1);
    qInfo() << "userNameuserName" << userName;
    QString path = "/home/";
    path  = path + userName + "/.config/deepin/dde-introduction.conf";
    QFile fileTemp(path);
    if(fileTemp.exists()){
        qInfo() << "fileTempfileTemp" << fileTemp.remove();
    }

    m_mainWindow = new MainWindow();
    m_mainWindow->show();
}

void ut_mainwindow_test::TearDown()
{
    if(m_mainWindow) {
        m_mainWindow->deleteLater();
        m_mainWindow = nullptr;
    }

}

/*******************************************************************************
 1. @函数:    keyPressEvent
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-12-18
 4. @说明:    测试keyPressEvent，和MainWindow的构造和析构函数，next(),previous()函数
*******************************************************************************/
TEST_F(ut_mainwindow_test, keyPressEvent)
{
    setlocale(LC_NUMERIC, "C");
    QKeyEvent* event = new QKeyEvent(QEvent::Type::KeyPress, 1, Qt::KeyboardModifier::NoModifier);
    m_mainWindow->m_isFirst = true;
    m_mainWindow->m_index = 1;
    m_mainWindow->keyPressEvent(event);

    m_mainWindow->m_index = 2;
    m_mainWindow->keyPressEvent(event);

    m_mainWindow->m_index = 3;
    m_mainWindow->keyPressEvent(event);

    m_mainWindow->m_index = 4;
    m_mainWindow->keyPressEvent(event);

    m_mainWindow->m_index = 5;
    m_mainWindow->m_currentAni->stop();
    m_mainWindow->keyPressEvent(event);

    m_mainWindow->m_isFirst = false;
    m_mainWindow->keyPressEvent(event);

    delete event;
    event=nullptr;
}

TEST_F(ut_mainwindow_test, closeEvent)
{
    QCloseEvent* event;
    m_mainWindow->closeEvent(event);
}

/*******************************************************************************
 1. @函数:    next
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-12-21
 4. @说明:    已经在keyPressEvent中一起测试
*******************************************************************************/
TEST_F(ut_mainwindow_test, next)
{
    QWidget* testWidget = new QWidget();
    ASSERT_TRUE(testWidget != nullptr);
#ifndef DISABLE_VIDEO
    VideoWidget* m_video = new VideoWidget(false, testWidget);
    ASSERT_TRUE(m_video != nullptr);
    m_mainWindow->m_last = m_mainWindow->m_current;
    m_mainWindow->next();
    m_video->deleteLater();
#endif
    testWidget->deleteLater();
}

/*******************************************************************************
 1. @函数:    previous
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-12-21
 4. @说明:    已经在keyPressEvent中一起测试
*******************************************************************************/
TEST_F(ut_mainwindow_test, previous)
{
    QWidget* testWidget = new QWidget();
    ASSERT_TRUE(testWidget != nullptr);
#ifndef DISABLE_VIDEO
    VideoWidget* m_video = new VideoWidget(false, testWidget);
    ASSERT_TRUE(m_video != nullptr);
    m_mainWindow->m_last = m_mainWindow->m_current;
    m_mainWindow->previous();
    m_video->deleteLater();
#endif
    testWidget->deleteLater();
}

/*******************************************************************************
 1. @函数:    slotTheme
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-12-21
 4. @说明:    主题变化响应
*******************************************************************************/
TEST_F(ut_mainwindow_test, slotTheme)
{
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::LightType);
    m_mainWindow->m_isFirst = true;
    m_mainWindow->slotTheme();

    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::DarkType);
    m_mainWindow->m_isFirst = true;
    m_mainWindow->slotTheme();

    EXPECT_EQ(DPalette::Window,m_mainWindow->backgroundRole());
}

/*******************************************************************************
 1. @函数:    initUI
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-12-21
 4. @说明:    已经在keyPressEvent创建MainWindow时测试了
*******************************************************************************/
TEST_F(ut_mainwindow_test, initUI)
{
    bool first = m_mainWindow->m_settings->value("IsFirst", true).toBool();
    m_mainWindow->initUI();
    ASSERT_TRUE(m_mainWindow->m_fakerWidget != nullptr);
    ASSERT_TRUE(m_mainWindow->m_pCloseBtn != nullptr);
    if(first == false) {
        m_mainWindow->m_settings->setValue("IsFirst", true);
        m_mainWindow->initUI();
    }
    ASSERT_TRUE(m_mainWindow->m_nextBtn != nullptr);
    ASSERT_TRUE(m_mainWindow->m_previousBtn != nullptr);
    ASSERT_TRUE(m_mainWindow->m_doneBtn != nullptr);
}

/*******************************************************************************
 1. @函数:    initConnect
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-12-21
 4. @说明:    已经在keyPressEvent创建MainWindow时测试了
*******************************************************************************/
TEST_F(ut_mainwindow_test, initConnect)
{
    m_mainWindow->m_isFirst = false;
    m_mainWindow->initConnect();
}

/*******************************************************************************
 1. @函数:    bindAnimation
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-12-21
 4. @说明:    已经在keyPressEvent的next()函数调用中测试了
*******************************************************************************/
TEST_F(ut_mainwindow_test, bindAnimation)
{
    m_mainWindow->bindAnimation();
}

/*******************************************************************************
 1. @函数:    bindAnimation
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-12-21
 4. @说明:    已经在keyPressEvent的next()函数调用中测试了
*******************************************************************************/
TEST_F(ut_mainwindow_test, updateModule)
{
    m_mainWindow->m_isFirst = true;
    m_mainWindow->m_useVideo = false;
    m_mainWindow->updateModule(1);

    m_mainWindow->m_useVideo = true;
    m_mainWindow->updateModule(1);

    m_mainWindow->updateModule(2);
    ASSERT_TRUE(m_mainWindow->m_current != nullptr);

    m_mainWindow->m_supportWM = false;
    m_mainWindow->updateModule(3);

    m_mainWindow->m_supportWM = true;
    m_mainWindow->updateModule(3);
    ASSERT_TRUE(m_mainWindow->m_current != nullptr);

    m_mainWindow->updateModule(4);
    ASSERT_TRUE(m_mainWindow->m_current != nullptr);
}

/*******************************************************************************
 1. @函数:    animationHandle
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-12-21
 4. @说明:    单元测试
*******************************************************************************/
TEST_F(ut_mainwindow_test, animationHandle)
{
    m_mainWindow->m_last = new DWidget(m_mainWindow);
    m_mainWindow->animationHandle();
}

/*******************************************************************************
 1. @函数:    initDesktopModeModule
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-12-23
 4. @说明:    单元测试
*******************************************************************************/
TEST_F(ut_mainwindow_test, initDesktopModeModule)
{
    m_mainWindow->initDesktopModeModule();
    ASSERT_TRUE(m_mainWindow->initDesktopModeModule() != nullptr);
}

/*******************************************************************************
 1. @函数:    initWMModeModule
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-12-23
 4. @说明:    单元测试
*******************************************************************************/
TEST_F(ut_mainwindow_test, initWMModeModule)
{
    m_mainWindow->initWMModeModule();
    ASSERT_TRUE(m_mainWindow->initWMModeModule() != nullptr);
}

/*******************************************************************************
 1. @函数:    initIconModule
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-12-23
 4. @说明:    单元测试
*******************************************************************************/
TEST_F(ut_mainwindow_test, initIconModule)
{
    m_mainWindow->initIconModule();
    ASSERT_TRUE(m_mainWindow->initIconModule() != nullptr);
}
