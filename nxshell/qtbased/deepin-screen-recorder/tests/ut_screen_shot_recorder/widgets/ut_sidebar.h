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

ACCESS_PRIVATE_FUN(SideBar, void(QPaintEvent *e), paintEvent);
ACCESS_PRIVATE_FUN(SideBar, void(QEvent *e), enterEvent);
ACCESS_PRIVATE_FUN(SideBar, bool(QObject *,QEvent *), eventFilter);

using namespace testing;
class SideBarTest:public testing::Test{

public:
    Stub stub;
    SideBar *m_sideBar;
    MainWindow *m_window;
    QString m_tips = QString("");
    virtual void SetUp() override{
        m_window = new MainWindow;
        m_sideBar = new SideBar(m_window);
        m_sideBar->initSideBar();
    }

    virtual void TearDown() override{
        if(nullptr != m_sideBar)
            delete m_sideBar;
    }
};

TEST_F(SideBarTest, paintEvent)
{
    QPaintEvent *e = new QPaintEvent(QRect());
    call_private_fun::SideBarpaintEvent(*m_sideBar,e);
    delete e;
}

TEST_F(SideBarTest, enterEvent)
{
    QEvent *e = new QEvent(QEvent::Enter);
    call_private_fun::SideBarenterEvent(*m_sideBar,e);
    delete e;
}

TEST_F(SideBarTest, eventFilter)
{
    QObject *obj = new QObject();
    QEvent *e = new QEvent(QEvent::ApplicationPaletteChange);
    call_private_fun::SideBareventFilter(*m_sideBar,obj,e);

    QEvent *paletteEvent = new QEvent(QEvent::PaletteChange);
    call_private_fun::SideBareventFilter(*m_sideBar,obj,paletteEvent);

    delete obj;
    delete e;
    delete paletteEvent;
}

TEST_F(SideBarTest, showAt)
{
    m_sideBar->showAt(QPoint(10,10));
}







