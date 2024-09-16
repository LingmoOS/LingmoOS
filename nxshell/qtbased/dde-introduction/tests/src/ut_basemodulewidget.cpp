// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_basemodulewidget.h"

#define protected public
#include "basemodulewidget.h"
#undef protected

ut_basemodulewidget_test::ut_basemodulewidget_test()
{
  
}

TEST_F(ut_basemodulewidget_test, BaseModuleWidget)
{
    QWidget* testWidget = new QWidget();
    ASSERT_TRUE(testWidget != nullptr);

    BaseModuleWidget *m_testModule = new BaseModuleWidget(testWidget);
    ASSERT_TRUE(m_testModule != nullptr);
    ASSERT_TRUE(m_testModule->m_layout != nullptr);
    ASSERT_TRUE(m_testModule->m_titleLbl != nullptr);
    ASSERT_TRUE(m_testModule->m_describeLbl != nullptr);
    m_testModule->deleteLater();
    testWidget->deleteLater();
}

TEST_F(ut_basemodulewidget_test, setTitle)
{
    QWidget* testWidget = new QWidget();
    BaseModuleWidget *m_testModule = new BaseModuleWidget(testWidget);
    QString Title = "test one";
    m_testModule->setTitle(Title);
    EXPECT_EQ(Title,m_testModule->m_titleLbl->text());

    m_testModule->setTitle("test two");
    EXPECT_NE(Title,m_testModule->m_titleLbl->text());
      
    m_testModule->deleteLater();
    testWidget->deleteLater();
}

TEST_F(ut_basemodulewidget_test, setDescribe)
{
    QWidget* testWidget = new QWidget();
    BaseModuleWidget *m_testModule = new BaseModuleWidget(testWidget);
    QString Describe = "test three";
    m_testModule->setDescribe(Describe);
    EXPECT_EQ(Describe,m_testModule->m_describeLbl->text());

    m_testModule->setDescribe("test four");
    EXPECT_NE(Describe,m_testModule->m_describeLbl->text());

    m_testModule->deleteLater();
    testWidget->deleteLater();
}


TEST_F(ut_basemodulewidget_test, getModel)
{
    QWidget* testWidget = new QWidget();
    BaseModuleWidget *m_testModule = new BaseModuleWidget(testWidget);
    m_testModule->getModel();
    ASSERT_TRUE(m_testModule->m_content != nullptr);
    m_testModule->deleteLater();
    testWidget->deleteLater();
}




