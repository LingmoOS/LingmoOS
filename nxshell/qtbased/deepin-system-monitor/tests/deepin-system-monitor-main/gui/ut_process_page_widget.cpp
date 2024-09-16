// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//Self
#include "process_page_widget.h"
#include "application.h"
#include "main_window.h"
#include "settings.h"
#include "process/process_set.h"
#include "kill_process_confirm_dialog.h"
#include "monitor_compact_view.h"
#include "monitor_expand_view.h"
#include "toolbar.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include "addr_pri.h"

//Qt
#include <QSignalSpy>
#include <QKeyEvent>
#include <QRect>

/***************************************STUB begin*********************************************/

QVariant stub_initUI_getOption_01()
{
    return kFilterCurrentUser;
}

QVariant stub_initUI_getOption_02()
{
    return kNoFilter;
}

QVariant stub_initUI_getOption_03()
{
    return kFilterApps;
}


int stub_popupKillConfirmDialog_exec(void* object)
{
    Q_UNUSED(object);
    return 1;
}

void stub_showWindowKiller_showMinimized()
{
    return;
}

void stub_paintEvent_isSearchContentEmpty()
{
    return;
}
/***************************************STUB end**********************************************/

class UT_ProcessPageWidget : public ::testing::Test
{
public:
    UT_ProcessPageWidget() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        static QWidget wid;
        static MainWindow mw;
        gApp->setMainWindow(&mw);

        m_tester = new ProcessPageWidget(&wid);
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    ProcessPageWidget *m_tester;
};

TEST_F(UT_ProcessPageWidget, initTest)
{

}

TEST_F(UT_ProcessPageWidget, test_initUI_01)
{
    Stub stub;
    stub.set(ADDR(Settings, getOption), stub_initUI_getOption_01);
    m_tester->initUI();

    EXPECT_NE(m_tester->m_rightStackView, nullptr);
}

TEST_F(UT_ProcessPageWidget, test_initUI_02)
{
    Stub stub;
    stub.set(ADDR(Settings, getOption), stub_initUI_getOption_02);
    m_tester->initUI();

    EXPECT_NE(m_tester->m_rightStackView, nullptr);
}

TEST_F(UT_ProcessPageWidget, test_initUI_03)
{
    Stub stub;
    stub.set(ADDR(Settings, getOption), stub_initUI_getOption_03);
    m_tester->initUI();

    EXPECT_NE(m_tester->m_rightStackView, nullptr);
}

TEST_F(UT_ProcessPageWidget, test_initConnections_01)
{
    m_tester->initConnections();

    EXPECT_NE(m_tester->m_settings, nullptr);
}

TEST_F(UT_ProcessPageWidget, test_eventFilter_01)
{
    static QKeyEvent ev(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    EXPECT_TRUE(m_tester->eventFilter(m_tester->m_appButton, dynamic_cast<QEvent*>(&ev)));
}

TEST_F(UT_ProcessPageWidget, test_eventFilter_02)
{
    static QKeyEvent ev(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    EXPECT_TRUE(m_tester->eventFilter(m_tester->m_myProcButton, dynamic_cast<QEvent*>(&ev)));
}

TEST_F(UT_ProcessPageWidget, test_eventFilter_03)
{
    static QKeyEvent ev(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    EXPECT_TRUE(m_tester->eventFilter(m_tester->m_myProcButton, dynamic_cast<QEvent*>(&ev)));
}

TEST_F(UT_ProcessPageWidget, test_eventFilter_04)
{
    static QKeyEvent ev(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    EXPECT_TRUE(m_tester->eventFilter(m_tester->m_allProcButton, dynamic_cast<QEvent*>(&ev)));
}

TEST_F(UT_ProcessPageWidget, test_switchProcessPage_01)
{
    m_tester->switchProcessPage();

    EXPECT_NE(m_tester->m_rightStackView, nullptr);
}

TEST_F(UT_ProcessPageWidget, test_switchCurrentNoFilterPage_01)
{
    m_tester->switchCurrentNoFilterPage();

    EXPECT_NE(m_tester->m_procTable, nullptr);
    EXPECT_TRUE(m_tester->m_allProcButton->isChecked());
}

TEST_F(UT_ProcessPageWidget, test_switchDisplayMode_01)
{
    int mode = 0;
    m_tester->switchDisplayMode(mode);
}

TEST_F(UT_ProcessPageWidget, test_paintEvent_01)
{
    static Toolbar toolbar;
    gApp->mainWindow()->m_toolbar = &toolbar;
    Stub stub;
    stub.set(ADDR(Toolbar, isSearchContentEmpty), stub_paintEvent_isSearchContentEmpty);
    EXPECT_TRUE(!m_tester->grab().isNull());
}

TEST_F(UT_ProcessPageWidget, test_popupKillConfirmDialog_01)
{
    typedef int (*fptr)(DDialog*);
    fptr DDialog_exec = (fptr)((int(DDialog::*)(void))&DDialog::exec);

    Stub stub;
    stub.set(DDialog_exec, stub_popupKillConfirmDialog_exec);
    m_tester->popupKillConfirmDialog(1000);
}

TEST_F(UT_ProcessPageWidget, test_showWindowKiller_01)
{
    Stub stub;
    stub.set(ADDR(MainWindow, showMinimized), stub_showWindowKiller_showMinimized);
    m_tester->showWindowKiller();
}

TEST_F(UT_ProcessPageWidget, test_changeIconTheme_01)
{
    DApplicationHelper::ColorType type = DApplicationHelper::ColorType::LightType;
    m_tester->changeIconTheme(type);
}

TEST_F(UT_ProcessPageWidget, test_changeIconTheme_02)
{
    DApplicationHelper::ColorType type = DApplicationHelper::ColorType::DarkType;
    m_tester->changeIconTheme(type);
}

TEST_F(UT_ProcessPageWidget, test_onStatInfoUpdated_01)
{
    m_tester->onStatInfoUpdated();
}

TEST_F(UT_ProcessPageWidget, test_onDetailWidgetChanged_01)
{
    static QWidget wid;
    static MonitorCompactView compactView(&wid);
    m_tester->m_compactView = &compactView;
    m_tester->onDetailWidgetChanged(1);
}

TEST_F(UT_ProcessPageWidget, test_onDetailWidgetChanged_02)
{
    static QWidget wid;
    static MonitorExpandView expandView(&wid);
    m_tester->m_expandView = &expandView;
    m_tester->onDetailWidgetChanged(1);
}

TEST_F(UT_ProcessPageWidget, test_onLoadLeftDataWidgetDelay_01)
{
    m_tester->onLoadLeftDataWidgetDelay();

    EXPECT_EQ(m_tester->m_views->count(), 2);
}
