// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//Self
#include "main_window.h"
#include "toolbar.h"
#include "application.h"
#include "process_page_widget.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include "addr_pri.h"

//Qt
#include <QSignalSpy>
#include <DSettingsDialog>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QTest>

/***************************************STUB begin*********************************************/
bool stub_popupSettingsDialog_exec(void* object)
{
    DSettingsDialog* obj = static_cast<DSettingsDialog*>(object);
    Q_UNUSED(obj);
    return true;
}

/***************************************STUB end**********************************************/

class UT_MainWindow : public ::testing::Test
{
public:
    UT_MainWindow() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new MainWindow();
        gApp->setMainWindow(m_tester);
        m_tester->initDisplay();

    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    MainWindow *m_tester;
};

TEST_F(UT_MainWindow, initTest)
{

}

TEST_F(UT_MainWindow, test_toolbar_01)
{
    m_tester->toolbar();

    EXPECT_NE(m_tester->m_toolbar, nullptr);
}

TEST_F(UT_MainWindow, test_initDisplay_01)
{
    gApp->setMainWindow(m_tester);
    m_tester->initDisplay();
}

TEST_F(UT_MainWindow, test_onLoadStatusChanged_01)
{
    m_tester->onLoadStatusChanged(true);

    EXPECT_NE(m_tester->m_toolbar, nullptr);
}

TEST_F(UT_MainWindow, test_onStartMonitorJob_01)
{
    m_tester->onStartMonitorJob();
}

TEST_F(UT_MainWindow, test_onDetailInfoByDbus_01)
{
    QString msgCode("MSG_PROCESS");
    m_tester->onDetailInfoByDbus(msgCode);
}

TEST_F(UT_MainWindow, test_onDetailInfoByDbus_02)
{
    QString msgCode("MSG_PROA");
    m_tester->onDetailInfoByDbus(msgCode);
}

TEST_F(UT_MainWindow, test_popupSettingsDialog_01)
{
    typedef int (*fptr)(DSettingsDialog*);
    fptr DSettingsDialog_exec = (fptr)(&DSettingsDialog::exec);   //obtaining an address
    Stub stub;
    stub.set(DSettingsDialog_exec, stub_popupSettingsDialog_exec);
    m_tester->popupSettingsDialog();

}

TEST_F(UT_MainWindow, test_initUI_01)
{
    m_tester->initUI();
    EXPECT_EQ(m_tester->m_tbShadow->isHidden(), false);
}

TEST_F(UT_MainWindow, test_initConnections_01)
{
    m_tester->initConnections();
}

TEST_F(UT_MainWindow, test_resizeEvent_01)
{
    QResizeEvent ev(QSize(10, 10), QSize(20, 20));
    m_tester->resizeEvent(&ev);
}

TEST_F(UT_MainWindow, test_closeEvent_01)
{
    QCloseEvent ev;
    m_tester->closeEvent(&ev);
}

TEST_F(UT_MainWindow, test_eventFilter_01)
{
    QObject obj;
    QKeyEvent event(QEvent::KeyPress, QKeySequence::Quit, Qt::ControlModifier);
    EXPECT_FALSE(m_tester->eventFilter(&obj, &event));
}

TEST_F(UT_MainWindow, test_showEvent_01)
{

    m_tester->m_initLoad = false;
    QShowEvent se;
    m_tester->showEvent(&se);

    EXPECT_TRUE(m_tester->m_initLoad);
}














