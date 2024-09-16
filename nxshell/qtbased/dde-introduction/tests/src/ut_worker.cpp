// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_worker.h"

ut_worker_test::ut_worker_test()
{

}

void ut_worker_test::SetUp()
{
    m_worker = new Worker;
}

void ut_worker_test::TearDown()
{
    delete m_worker;
    m_worker = nullptr;
}

/*******************************************************************************
 1. @函数:    setDesktopMode
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-12-23
 4. @说明:    测试设置桌面样式
*******************************************************************************/
TEST_F(ut_worker_test, setDesktopMode)
{
    ASSERT_TRUE(m_worker->m_model != nullptr);
    ASSERT_TRUE(m_worker->m_iconInter != nullptr);
    ASSERT_TRUE(m_worker->m_wmInter != nullptr);
    ASSERT_TRUE(m_worker->m_dockInter != nullptr);
    //测试时尚模式
    m_worker->setDesktopMode(Model::DesktopMode::FashionMode);

    //测试高效模式
    m_worker->setDesktopMode(Model::DesktopMode::EfficientMode);
}

/*******************************************************************************
 1. @函数:    setWMMode
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-12-23
 4. @说明:    测试设置运行模式
*******************************************************************************/
TEST_F(ut_worker_test, setWMMode)
{
    m_worker->m_model->m_wmType = Model::WM_2D;
    m_worker->setWMMode(Model::WM_3D);
    EXPECT_EQ(Model::WM_2D,m_worker->m_model->wmType());

    m_worker->setWMMode(Model::WM_2D);
    EXPECT_EQ(Model::WM_2D,m_worker->m_model->wmType());
}

/*******************************************************************************
 1. @函数:    setIcon
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-12-23
 4. @说明:    测试设置图标
*******************************************************************************/
TEST_F(ut_worker_test, setIcon)
{
    //模拟数据
    IconStruct tmp;
    tmp.Id = "1";
    tmp.Path = "333";
    tmp.Deletable = false;
    tmp.Pixmap = "pix";
    m_worker->setIcon(tmp);
}

/*******************************************************************************
 1. @函数:    onWMChanged
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-12-23
 4. @说明:    测试运行模式响应函数
*******************************************************************************/
TEST_F(ut_worker_test,onWMChanged)
{
    m_worker->m_model->m_wmType = Model::WM_2D;
    m_worker->onWMChanged("deepin wm");
    EXPECT_EQ(Model::WM_3D,m_worker->m_model->wmType());

    m_worker->onWMChanged("deepin wm test");
    EXPECT_EQ(Model::WM_2D,m_worker->m_model->wmType());
}

/*******************************************************************************
 1. @函数:    onWMChang
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-12-23
 4. @说明:    目前函数不使用
*******************************************************************************/
TEST_F(ut_worker_test, onWMChang)
{
    //该函数暂时未用到
    m_worker->onWMChang();
}

/*******************************************************************************
 1. @函数:    onDisplayModeChanged
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-12-23
 4. @说明:    测试桌面样式改变响应函数
*******************************************************************************/
TEST_F(ut_worker_test, onDisplayModeChanged)
{
    m_worker->onDisplayModeChanged(Model::DesktopMode::EfficientMode);
    EXPECT_EQ(Model::DesktopMode::EfficientMode,m_worker->m_model->m_desktopMode);
    m_worker->onDisplayModeChanged(Model::DesktopMode::FashionMode);
    EXPECT_EQ(Model::DesktopMode::FashionMode,m_worker->m_model->m_desktopMode);
}

/*******************************************************************************
 1. @函数:    onIconRefreshed
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-12-23
 4. @说明:    测试图标刷新响应函数
*******************************************************************************/
TEST_F(ut_worker_test, onIconRefreshed)
{
    m_worker->onIconRefreshed("icon");
}

/*******************************************************************************
 1. @函数:    onIconListChanged
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-12-23
 4. @说明:    测试图标列表变化响应函数
*******************************************************************************/
TEST_F(ut_worker_test, onIconListChanged)
{
    QString value = "[{\"Id\":\"bloom-dark\",\"Path\":\"/usr/share/icons/bloom-dark\",\"Deletable\":false}]";
    //模拟数据
    IconStruct tmp;
    tmp.Id = "1";
    tmp.Path = "333";
    tmp.Deletable = false;
    tmp.Pixmap = "pix";
    //添加无用的数据，执行onIconListChanged时删除
    m_worker->m_model->m_iconList.append(tmp);
    m_worker->onIconListChanged(value);

}

/*******************************************************************************
 1. @函数:    onIconPixmapFinished
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-12-23
 4. @说明:    测试图标完成响应函数
*******************************************************************************/
TEST_F(ut_worker_test, onIconPixmapFinished)
{
    QString tmp = "[{\"Id\":\"bloom-dark\",\"Path\":\"/usr/share/icons/bloom-dark\",\"Deletable\":false}]";
    const QJsonArray &array = QJsonDocument::fromJson(tmp.toUtf8()).array();
    QDBusPendingReply<QString> icon =
        m_worker->m_iconInter->Thumbnail("icon", array.at(0).toObject()["Id"].toString());
    const QJsonObject &obj = array.at(0).toObject();
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(icon, this);
    ASSERT_TRUE(watcher != nullptr);
    watcher->setProperty("Json", obj);
    m_worker->onIconPixmapFinished(watcher);

    watcher->deleteLater();
}

TEST_F(ut_worker_test, onIconClickChange)
{
    QDBusPendingReply<QString> icon = m_worker->m_iconInter->List("icon");
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(icon, this);
    ASSERT_TRUE(watcher != nullptr);
    m_worker->onIconClickChange(watcher);

    watcher->deleteLater();
}
