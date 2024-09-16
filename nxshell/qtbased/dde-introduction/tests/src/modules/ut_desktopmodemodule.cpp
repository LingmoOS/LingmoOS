// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_desktopmodemodule.h"

#define protected public
#include "desktopmodemodule.h"
#include "model.h"
#undef protected

ut_desktopmodemodule_test::ut_desktopmodemodule_test()
{

}

TEST_F(ut_desktopmodemodule_test, DesktopModeModule)
{
    DesktopModeModule* test_module = new DesktopModeModule();
    ASSERT_TRUE(test_module);
    ASSERT_TRUE(test_module->m_layout);
    ASSERT_TRUE(test_module->m_efficientWidget);
    ASSERT_TRUE(test_module->m_fashionWidget);

    EXPECT_TRUE(test_module->isfirst);
    test_module->deleteLater();
}

TEST_F(ut_desktopmodemodule_test, onDesktopTypeChanged)
{
    DesktopModeModule* test_module = new DesktopModeModule();

    test_module->onDesktopTypeChanged(Model::DesktopMode::EfficientMode);
    EXPECT_TRUE(test_module->m_efficientWidget->m_borderWidget->m_checked);
    EXPECT_FALSE(test_module->m_fashionWidget->m_borderWidget->m_checked);

    test_module->onDesktopTypeChanged(Model::DesktopMode::FashionMode);
    EXPECT_FALSE(test_module->m_efficientWidget->m_borderWidget->m_checked);
    EXPECT_TRUE(test_module->m_fashionWidget->m_borderWidget->m_checked);
     
    test_module->deleteLater();
}

TEST_F(ut_desktopmodemodule_test, updateBigIcon)
{
    DesktopModeModule* test_module = new DesktopModeModule();
    test_module->updateBigIcon();   
    EXPECT_EQ(330,test_module->m_size.width());
    EXPECT_EQ(210,test_module->m_size.height());
    test_module->deleteLater();
}

TEST_F(ut_desktopmodemodule_test, updateSmallIcon)
{
    DesktopModeModule* test_module = new DesktopModeModule();
    test_module->updateSmallIcon();
    EXPECT_EQ(250,test_module->m_size.width());
    EXPECT_EQ(160,test_module->m_size.height());
    test_module->deleteLater();
}

TEST_F(ut_desktopmodemodule_test, updateSelectBtnPos)
{
    DesktopModeModule* test_module = new DesktopModeModule();
    test_module->m_model->setDesktopMode(Model::EfficientMode);
    test_module->updateSelectBtnPos();
    EXPECT_TRUE(test_module->m_efficientWidget->m_borderWidget->m_checked);
    EXPECT_FALSE(test_module->m_fashionWidget->m_borderWidget->m_checked);

    test_module->m_model->setDesktopMode(Model::FashionMode);
    test_module->updateSelectBtnPos();
    EXPECT_FALSE(test_module->m_efficientWidget->m_borderWidget->m_checked);
    EXPECT_TRUE(test_module->m_fashionWidget->m_borderWidget->m_checked);

    test_module->deleteLater();
}

TEST_F(ut_desktopmodemodule_test, setFirst)
{
    DesktopModeModule* test_module = new DesktopModeModule();
    test_module->setFirst(true);
    EXPECT_TRUE(test_module->isfirst);

    test_module->setFirst(false);
    EXPECT_FALSE(test_module->isfirst);
     
    test_module->deleteLater();
}

TEST_F(ut_desktopmodemodule_test, keyPressEvent)
{
    DesktopModeModule* test_module = new DesktopModeModule();
    QKeyEvent keyEvent(QEvent::KeyPress,Qt::Key_Left,Qt::NoModifier);
    test_module->m_model->setDesktopMode(Model::EfficientMode);
    test_module->keyPressEvent(&keyEvent);

    QKeyEvent keyEvent1(QEvent::KeyPress,Qt::Key_Right,Qt::NoModifier);
    test_module->m_model->setDesktopMode(Model::FashionMode);
    test_module->keyPressEvent(&keyEvent1);

    test_module->deleteLater();
}

TEST_F(ut_desktopmodemodule_test, paintEvent)
{
    DesktopModeModule* test_module = new DesktopModeModule();
    QPaintEvent *event;
    test_module->setFirst(true);
    test_module->m_model->setDesktopMode(Model::EfficientMode);
    test_module->paintEvent(event);
    EXPECT_TRUE(test_module->isfirst);

    test_module->setFirst(true);
    test_module->m_model->setDesktopMode(Model::FashionMode);
    test_module->paintEvent(event);
    EXPECT_TRUE(test_module->isfirst);

    test_module->setFirst(false);
    test_module->m_model->setDesktopMode(Model::EfficientMode);
    test_module->paintEvent(event);
    EXPECT_FALSE(test_module->isfirst);

    test_module->setFirst(false);
    test_module->m_model->setDesktopMode(Model::FashionMode);
    test_module->paintEvent(event);
    EXPECT_FALSE(test_module->isfirst);

    test_module->deleteLater();
}

TEST_F(ut_desktopmodemodule_test, updateInterface)
{
    DesktopModeModule* test_module = new DesktopModeModule();
    float f = 1.0;
    test_module->updateInterface(f);
    EXPECT_EQ(test_module->m_fashionWidget->m_borderWidget->m_pixmap.size(),
              test_module->m_fashionWidget->m_borderWidget->m_size);

    float f1 = 2.0;
    test_module->updateInterface(f1);
    EXPECT_NE(test_module->m_fashionWidget->m_borderWidget->m_pixmap.size(),
              test_module->m_fashionWidget->m_borderWidget->m_size);
    test_module->deleteLater();
}

TEST_F(ut_desktopmodemodule_test, onSetModeFashion)
{
    DesktopModeModule* test_module = new DesktopModeModule();
    test_module->onSetModeFashion();
    test_module->deleteLater();
}

TEST_F(ut_desktopmodemodule_test, onSetModeEfficient)
{
    DesktopModeModule* test_module = new DesktopModeModule();
    test_module->onSetModeEfficient();
    test_module->deleteLater();
}
