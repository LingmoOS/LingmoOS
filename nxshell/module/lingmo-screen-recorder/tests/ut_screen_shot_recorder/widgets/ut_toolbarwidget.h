// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QDebug>
#include "stub.h"
#include "addr_pri.h"
#include <DApplicationHelper>

#include "../../src/main_window.h"
#include "../../src/widgets/toolbar.h"
#include "../../src/utils.h"

using namespace testing;
//ACCESS_PRIVATE_FUN(ToolTips, void(QResizeEvent *ev), resizeEvent);
//ACCESS_PRIVATE_FUN(ToolButton, void(QEvent *e), enterEvent);
//ACCESS_PRIVATE_FUN(ToolButton, void(QEvent *e), leaveEvent);
//ACCESS_PRIVATE_FUN(ToolButton, void(QMouseEvent *e), mousePressEvent);
//ACCESS_PRIVATE_FUN(ToolBarWidget, void(QPaintEvent *), paintEvent);
//ACCESS_PRIVATE_FUN(ToolBarWidget, void(QShowEvent *), showEvent);
//DGuiApplicationHelper::ColorType themeType_default_stub()
//{
//    return DGuiApplicationHelper::ColorType::UnknownType;
//}
//DGuiApplicationHelper::ColorType themeType_Light_stub()
//{
//    return DGuiApplicationHelper::ColorType::LightType;
//}
//DGuiApplicationHelper::ColorType themeType_Dark_stub()
//{
//    return DGuiApplicationHelper::ColorType::DarkType;
//}

//void paintEvent_stub(QPaintEvent* e)
//{
//    Q_UNUSED(e);
//};

int toInt_one_stub(bool *ok = nullptr)
{
    Q_UNUSED(ok);
    return 1;
}

int toInt_two_stub(bool *ok = nullptr)
{
    Q_UNUSED(ok);
    return 2;
}

bool waylandDectected_true_stub()
{
    return true;
}

bool waylandDectected_false_stub()
{
    return false;
}

bool toolBar_toBool_stub()
{
    return true;
}

class ToolBarWidgetTest:public testing::Test{

public:
    Stub stub;
    ToolBar *m_toolBar;
    MainWindow *m_mainWindow;
    ToolBarWidget *m_toolBarWidget;
    ToolBarWidget *m_toolBarWidget_test;
    virtual void SetUp() override{
        m_mainWindow = new MainWindow;
        m_toolBar = new ToolBar(m_mainWindow);
        //stub.set(ADDR(DesktopInfo,waylandDectected),waylandDectected_true_stub);
        stub.set(ADDR(QVariant,toInt),toInt_one_stub);
        m_toolBarWidget = new ToolBarWidget(m_mainWindow,m_toolBar);
        //stub.reset(ADDR(DesktopInfo,waylandDectected));
        stub.reset(ADDR(QVariant,toInt));
        //stub.set(ADDR(DesktopInfo,waylandDectected),waylandDectected_false_stub);
        stub.set(ADDR(QVariant,toInt),toInt_two_stub);
        Utils::is3rdInterfaceStart = true;
        m_toolBarWidget_test = new ToolBarWidget(m_mainWindow,m_toolBar);
    }

    virtual void TearDown() override{
        if(nullptr != m_toolBarWidget)
            delete m_toolBarWidget;
        if(nullptr != m_toolBarWidget_test)
            delete m_toolBarWidget_test;
        if(nullptr != m_toolBar)
            m_toolBar->deleteLater();
        if(nullptr != m_mainWindow)
            m_mainWindow->deleteLater();
        //stub.reset(ADDR(DesktopInfo,waylandDectected));
        stub.reset(ADDR(QVariant,toInt));
    }
};

//TEST_F(ToolBarWidgetTest, paintEvent)
//{
//    QPaintEvent *paintEvent = new QPaintEvent(QRect());
//    call_private_fun::ToolBarWidgetpaintEvent(*m_toolBarWidget,paintEvent);

//    QEventLoop eventloop;
//    QTimer::singleShot(500, &eventloop, SLOT(quit()));
//    eventloop.exec();
//}

//TEST_F(ToolBarWidgetTest, showEvent)
//{
//    stub.set(ADDR(QVariant,toBool),toolBar_toBool_stub);
//    QShowEvent *showEvent = new QShowEvent();
//    call_private_fun::ToolBarWidgetshowEvent(*m_toolBarWidget,showEvent);

//    delete showEvent;
//}

TEST_F(ToolBarWidgetTest, setExpand)
{
    m_toolBarWidget->setExpand(true,QString("circ"));
}

TEST_F(ToolBarWidgetTest, changeArrowAndLineFromBar)
{
    m_toolBarWidget->changeArrowAndLineFromBar(1);
}

TEST_F(ToolBarWidgetTest, setRecordButtonDisableFromMain)
{
    m_toolBarWidget->setRecordButtonDisableFromMain();
}

TEST_F(ToolBarWidgetTest, shapeClickedFromBar)
{
    //m_toolBarWidget->shapeClickedFromBar(QString("circ"));
}

TEST_F(ToolBarWidgetTest, setRecordLaunchFromMain)
{
    m_toolBarWidget->setRecordLaunchFromMain(true);
}

TEST_F(ToolBarWidgetTest, setVideoInitFromMain)
{
    m_toolBarWidget->setVideoInitFromMain();
}

TEST_F(ToolBarWidgetTest, setMicroPhoneEnable)
{
    m_toolBarWidget->setMicroPhoneEnable(true);
}

TEST_F(ToolBarWidgetTest, setSystemAudioEnable)
{
    m_toolBarWidget->setSystemAudioEnable(true);
}

TEST_F(ToolBarWidgetTest, setCameraDeviceEnable)
{
    m_toolBarWidget->setCameraDeviceEnable(false);
}






