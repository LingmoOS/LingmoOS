// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QEvent>
#include <QDebug>
#include "stub.h"
#include "addr_pri.h"
#include <DApplicationHelper>
#include "../../src/main_window.h"
#include "../../src/widgets/sidebar.h"

//ACCESS_PRIVATE_FUN(SideBarWidget, void(QPaintEvent *e), paintEvent);
ACCESS_PRIVATE_FUN(SideBarWidget, void(QShowEvent *event), showEvent);
using namespace testing;

int toInt_stub2(bool *ok = nullptr)
{
    Q_UNUSED(ok)
    return 2;
}
class SideBarWidgetTest:public testing::Test{

public:
    Stub stub;
    SideBar *m_sideBar;
    SideBarWidget *m_sideBarWidget;
    MainWindow *m_window;
    QString m_tips = QString("");
    virtual void SetUp() override{
        m_window = new MainWindow;
        m_sideBar = new SideBar(m_window);
        m_sideBarWidget = new SideBarWidget(m_sideBar);
        //m_sideBar->initSideBar();
    }

    virtual void TearDown() override{
        if(nullptr != m_sideBar)
            delete m_sideBar;
    }
};

TEST_F(SideBarWidgetTest, changeShotToolWidget)
{
    stub.set(ADDR(QVariant,toInt),toInt_stub2);
    SideBarWidget();
    stub.reset(ADDR(QVariant,toInt));
    m_sideBarWidget->changeShotToolWidget(QString("rectangle"));
    m_sideBarWidget->changeShotToolWidget(QString("oval"));
    m_sideBarWidget->changeShotToolWidget(QString("arrow"));
    m_sideBarWidget->changeShotToolWidget(QString("line"));
    m_sideBarWidget->changeShotToolWidget(QString("text"));
}

//TEST_F(SideBarWidgetTest, paintEvent)
//{
//    QPaintEvent *e = new QPaintEvent(QRect());
//    call_private_fun::SideBarWidgetpaintEvent(*m_sideBarWidget,e);
//    QEventLoop loop;
//    QTimer::singleShot(1000, &loop, SLOT(quit()));
//    loop.exec();
//}

TEST_F(SideBarWidgetTest, showEvent)
{
    QShowEvent *e = new QShowEvent();
    call_private_fun::SideBarWidgetshowEvent(*m_sideBarWidget,e);
    QEventLoop loop;
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

     delete e;
}







