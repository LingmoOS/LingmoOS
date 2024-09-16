// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_model.h"
#include <QString>

ut_model_test::ut_model_test()
{

}

void ut_model_test::SetUp()
{
    m_model = new Model();
}

void ut_model_test::TearDown()
{
    delete m_model;
    m_model = nullptr;
}

/*******************************************************************************
 1. @函数:    currentIcon
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-12-22
 4. @说明:    测试获取当前图标
*******************************************************************************/
TEST_F(ut_model_test, currentIcon)
{
    //模拟数据
    IconStruct tmp;
    tmp.Id = "1";
    tmp.Path = "333";
    tmp.Deletable = false;
    tmp.Pixmap = "pix";
    m_model->m_iconList.append(tmp);
    m_model->m_currentIcon = tmp.Id;
    m_model->currentIcon();
    EXPECT_EQ(tmp, m_model->currentIcon());
}

/*******************************************************************************
 1. @函数:    addIcon
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-12-22
 4. @说明:    测试添加图标
*******************************************************************************/
TEST_F(ut_model_test, addIcon)
{
    //模拟数据
    IconStruct tmp;
    tmp.Id = "1";
    tmp.Path = "333";
    tmp.Deletable = false;
    tmp.Pixmap = "pix";

    m_model->m_currentIcon = tmp.Id;

    //测试添加成功流程
    m_model->addIcon(tmp);

    //测试添加已存在图标流程
    m_model->addIcon(tmp);
    EXPECT_TRUE(m_model->m_iconList.size() > 0);
    //测试添加第二个图标，测试排序函数
    //模拟数据二
    IconStruct tmp2;
    tmp2.Id = "2";
    tmp2.Path = "444";
    tmp2.Deletable = false;
    tmp2.Pixmap = "pix2";
    m_model->addIcon(tmp2);
    EXPECT_TRUE(m_model->m_iconList.size() > 1);
}

/*******************************************************************************
 1. @函数:    removeIcon
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-12-22
 4. @说明:    测试删除图标
*******************************************************************************/
TEST_F(ut_model_test, removeIcon)
{
    //模拟数据
    IconStruct tmp;
    tmp.Id = "1";
    tmp.Path = "333";
    tmp.Deletable = false;
    tmp.Pixmap = "pix";
    m_model->m_iconList.append(tmp);
    m_model->m_currentIcon = tmp.Id;

    int dataSize = m_model->m_iconList.size();
    m_model->removeIcon(tmp);
    EXPECT_NE(dataSize, m_model->m_iconList.size());
    EXPECT_TRUE(m_model->m_iconList.size() < dataSize);
}

/*******************************************************************************
 1. @函数:    removeIcon
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-12-22
 4. @说明:    测试删除图标
*******************************************************************************/
TEST_F(ut_model_test, setCurrentIcon)
{
    //模拟数据
    IconStruct tmp;
    tmp.Id = "1";
    tmp.Path = "333";
    tmp.Deletable = false;
    tmp.Pixmap = "pix";
    m_model->m_iconList.append(tmp);

    //测试设置成功流程
    m_model->setCurrentIcon(tmp.Id);
    EXPECT_EQ(m_model->m_currentIcon, tmp.Id);

    //测试已经选择当前图标流程
    m_model->m_currentIcon = tmp.Id;
    m_model->setCurrentIcon(tmp.Id);
    EXPECT_EQ(m_model->m_currentIcon, tmp.Id);

}

/*******************************************************************************
 1. @函数:    setDesktopMode
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-12-23
 4. @说明:    测试设置桌面样式
*******************************************************************************/
TEST_F(ut_model_test, setDesktopMode)
{
    //测试设置成功流程
    m_model->setDesktopMode(Model::EfficientMode);
    EXPECT_EQ(m_model->m_desktopMode, Model::EfficientMode);

    //测试设置成功流程
    m_model->setDesktopMode(Model::FashionMode);
    EXPECT_EQ(m_model->m_desktopMode, Model::FashionMode);

    //测试已经选择当前桌面样式
    m_model->m_desktopMode = Model::EfficientMode;
    m_model->setDesktopMode(Model::EfficientMode);

}

/*******************************************************************************
 1. @函数:    setWmType
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-12-23
 4. @说明:    测试设置运行模式
*******************************************************************************/
TEST_F(ut_model_test, setWmType)
{
    //测试设置成功流程
    m_model->setWmType(Model::WM_3D);
    EXPECT_EQ(m_model->m_wmType, Model::WM_3D);

    m_model->setWmType(Model::WM_2D);
    EXPECT_EQ(m_model->m_wmType, Model::WM_2D);

    m_model->setWmType(Model::WM_NOMAL);
    EXPECT_EQ(m_model->m_wmType, Model::WM_NOMAL);

    //测试已经选择当前运行模式
    m_model->m_wmType = Model::WM_3D;
    m_model->setWmType(Model::WM_3D);
}



