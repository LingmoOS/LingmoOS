// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//Self
#include "process_table_view.h"
#include "kill_process_confirm_dialog.h"
#include "process/process_db.h"
#include "process_attribute_dialog.h"
#include "model/process_sort_filter_proxy_model.h"
#include "main_window.h"
#include "toolbar.h"
#include "application.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include "addr_pri.h"

//Qt
#include <QSignalSpy>
#include <QKeyEvent>
#include <QModelIndexList>
#include <QHeaderView>
#include <DDialog>

/***************************************STUB begin*********************************************/

static bool s_dialogResult = false;
static bool s_processdb_iscurrentprocess = false;
static bool s_openFilePathItem = false;
static bool s_showProperties = false;
static bool s_adjustInfoLabelVisibility = false;
bool stub_eventFilter_hasFocus_true()
{
    return true;
}

bool stub_eventFilter_hasFocus_false()
{
    return false;
}

QByteArray stub_openExecDirWithFM_readAllStandardOutput()
{
    return QString("Location:BB\nBBLocation:CC\nCCLocation:DD").toLocal8Bit();
}

QModelIndexList stub_eventFilter_selectedIndexes(void* object)
{
    QModelIndexList list;
    list << QModelIndex() << QModelIndex() <<QModelIndex();
    return  list;
}

bool stub_eventFitler_qwidget_hasFocus_true()
{
    return true;
}

bool stub_eventFitler_qwidget_hasFocus_false()
{
    return false;
}

bool stub_eventFitler_qheaderview_hasFocus_true()
{
    return true;
}

bool stub_endProcess_exec()
{
    return true;
}

bool stub_endProcess_result()
{
    s_dialogResult = true;
    return s_dialogResult;
}

bool stub_pauseProcess_isCurrentProcess_false(pid_t pid)
{
    Q_UNUSED(pid);
    s_processdb_iscurrentprocess = false;
    return s_processdb_iscurrentprocess;
}

void stub_pauseProcess_pauseProcess(pid_t pid)
{
    return;
}

void stub_resumeProcess_resumeProcess(pid_t pid)
{
    return;
}

QString stub_openExecDirWithFM_cmdlineString()
{
    return QString("/usr/bin/");
}

QString stub_openExecDirWithFM_cmdlineString_CDir()
{
    return QString("c:/system32");
}

bool stub_openExecDirWithFM_openFilePathItem()
{
    s_openFilePathItem = true;
    return s_openFilePathItem;
}

QHash<QString, QString> stub_openExecDirWithFM_environ()
{
    QHash<QString, QString> hash;
    hash.insert("FLATPAK_APPID", "11");
    return hash;
}

bool stub_showProperties_show()
{
    s_showProperties = true;
    return s_showProperties;
}

time_t stub_showProperties_startTime()
{
    return 0;
}

bool stub_search_adjustInfoLabelVisibility(void* obj)
{
    s_adjustInfoLabelVisibility = true;
    return s_adjustInfoLabelVisibility;
}

Toolbar* stub_adjustInfoLabelVisibility_toolbar()
{
    static Toolbar toolbar;
    return &toolbar;
}

/***************************************STUB end**********************************************/
ACCESS_PRIVATE_FUN(ProcessTableView, void(void), adjustInfoLabelVisibility);
class UT_ProcessTableView : public ::testing::Test
{
public:
    UT_ProcessTableView() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        static QWidget wid;
        static MainWindow wm;
        static Toolbar toolbar;
        wm.m_toolbar = &toolbar;
        gApp->setMainWindow(&wm);
        m_tester = new ProcessTableView(&wid);
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    ProcessTableView *m_tester;
};

TEST_F(UT_ProcessTableView, initTest)
{

}

TEST_F(UT_ProcessTableView, test_eventFilter_01)
{

    QKeyEvent ev(QEvent::KeyPress, Qt::Key_M, Qt::AltModifier);
    typedef int (*fptr)(QTreeView*);
    fptr QTreeView_selectedIndexes = (fptr)(&QTreeView::selectedIndexes);   //obtaining an address
    Stub stub;
    stub.set(QTreeView_selectedIndexes, stub_eventFilter_selectedIndexes);

    stub.set(ADDR(ProcessTableView, hasFocus), stub_eventFilter_hasFocus_true);
    m_tester->eventFilter(m_tester, &ev);
}

TEST_F(UT_ProcessTableView, test_eventFilter_02)
{
    QKeyEvent ev(QEvent::KeyPress, Qt::Key_M, Qt::AltModifier);
    Stub stub;
    stub.set(ADDR(QWidget, hasFocus), stub_eventFitler_qwidget_hasFocus_false);
    stub.set(ADDR(QHeaderView, hasFocus), stub_eventFitler_qheaderview_hasFocus_true);
    m_tester->eventFilter(m_tester, &ev);
}

TEST_F(UT_ProcessTableView, test_endProcess_01)
{
    m_tester->endProcess();
}

TEST_F(UT_ProcessTableView, test_endProcess_02)
{
    typedef int (*fptr)(DDialog*);
    fptr DDialog_exec = (fptr)((int(DDialog::*)(void))&DDialog::exec);

    Stub stub;
    stub.set(DDialog_exec, stub_endProcess_exec);
    m_tester->m_selectedPID = QVariant(100000000);
    m_tester->endProcess();
}

TEST_F(UT_ProcessTableView, test_endProcess_03)
{
    typedef int (*fptr)(DDialog*);
    fptr DDialog_exec = (fptr)((int(DDialog::*)(void))&DDialog::exec);

    Stub stub;
    stub.set(DDialog_exec, stub_endProcess_exec);
    m_tester->m_selectedPID = QVariant(100000000);

    stub.set(ADDR(KillProcessConfirmDialog, result), stub_endProcess_result);

    m_tester->endProcess();
}

TEST_F(UT_ProcessTableView, test_pauseProcess_01)
{
    m_tester->pauseProcess();
}

TEST_F(UT_ProcessTableView, test_pauseProcess_02)
{
    m_tester->m_selectedPID = QVariant(100000000);
    Stub stub;
    stub.set(ADDR(ProcessDB, isCurrentProcess), stub_pauseProcess_isCurrentProcess_false);

    stub.set(ADDR(ProcessDB, pauseProcess), stub_pauseProcess_pauseProcess);
    m_tester->pauseProcess();

    EXPECT_FALSE(s_processdb_iscurrentprocess);
}

TEST_F(UT_ProcessTableView, test_resumeProcess_01)
{
    m_tester->resumeProcess();
}

TEST_F(UT_ProcessTableView, test_resumeProcess_02)
{
    m_tester->m_selectedPID = QVariant(100000000);
    Stub stub;
    stub.set(ADDR(ProcessDB, isCurrentProcess), stub_pauseProcess_isCurrentProcess_false);
    stub.set(ADDR(ProcessDB, resumeProcess), stub_resumeProcess_resumeProcess);

    m_tester->resumeProcess();
}

TEST_F(UT_ProcessTableView, test_openExecDirWithFM_01)
{
    m_tester->m_selectedPID = QVariant(100000000);

    Stub stub;
    stub.set(ADDR(Process, cmdlineString), stub_openExecDirWithFM_cmdlineString);
    stub.set(common::openFilePathItem, stub_openExecDirWithFM_openFilePathItem);

    m_tester->openExecDirWithFM();

    EXPECT_TRUE(s_openFilePathItem);
}

TEST_F(UT_ProcessTableView, test_openExecDirWithFM_02)
{
    m_tester->m_selectedPID = QVariant(100000000);

    Stub stub;
    stub.set(ADDR(Process, cmdlineString), stub_openExecDirWithFM_cmdlineString_CDir);
    stub.set(common::openFilePathItem, stub_openExecDirWithFM_openFilePathItem);

    m_tester->openExecDirWithFM();

    EXPECT_TRUE(s_openFilePathItem);
}

TEST_F(UT_ProcessTableView, test_openExecDirWithFM_03)
{
    m_tester->m_selectedPID = QVariant(100000000);

    Stub stub;
    stub.set(ADDR(Process, cmdlineString), stub_openExecDirWithFM_cmdlineString);
    stub.set(common::openFilePathItem, stub_openExecDirWithFM_openFilePathItem);
    stub.set(ADDR(Process, environ), stub_openExecDirWithFM_environ);
    stub.set(ADDR(QProcess, readAllStandardOutput), stub_openExecDirWithFM_readAllStandardOutput);

    m_tester->openExecDirWithFM();

    EXPECT_TRUE(s_openFilePathItem);
}


TEST_F(UT_ProcessTableView, test_showProperties_01)
{
    m_tester->m_selectedPID = QVariant(100000000);

    Stub stub;
    stub.set(ADDR(ProcessAttributeDialog, show), stub_showProperties_show);

    stub.set(ADDR(Process, startTime), stub_showProperties_startTime);
    m_tester->showProperties();

    EXPECT_TRUE(s_showProperties);
}

//TEST_F(UT_ProcessTableView, test_search_01)
//{
//    m_tester->search("1");
//}

TEST_F(UT_ProcessTableView, test_switchDisplayMode_01)
{
    m_tester->switchDisplayMode(kFilterApps);

    EXPECT_EQ(m_tester->m_proxyModel->m_fileterType, kFilterApps);
}

TEST_F(UT_ProcessTableView, test_changeProcessPriority_01)
{
    m_tester->m_selectedPID = QVariant(100000000);
    m_tester->changeProcessPriority(0);

    EXPECT_TRUE(m_tester->m_selectedPID.isValid());
}

TEST_F(UT_ProcessTableView, test_onThemeTypeChanged_01)
{
    m_tester->onThemeTypeChanged();
}

TEST_F(UT_ProcessTableView, test_loadSettings_01)
{
    m_tester->loadSettings();
}


TEST_F(UT_ProcessTableView, test_saveSettings_01)
{
    m_tester->saveSettings();
}

TEST_F(UT_ProcessTableView, test_initUI_01)
{
    m_tester->initUI(false);

    EXPECT_EQ(m_tester->accessibleName(), "ProcessTableView");
}

TEST_F(UT_ProcessTableView, test_initUI_02)
{
    m_tester->initUI(true);

    EXPECT_EQ(m_tester->accessibleName(), "ProcessTableView");
}

TEST_F(UT_ProcessTableView, test_initConnections_01)
{
    m_tester->initConnections(false);

    EXPECT_NE(m_tester->m_endProcKP, nullptr);
}

TEST_F(UT_ProcessTableView, test_initConnections_02)
{
    m_tester->initConnections(true);

    EXPECT_NE(m_tester->m_endProcKP, nullptr);
}

TEST_F(UT_ProcessTableView, test_displayProcessTableContextMenu_01)
{
    typedef int (*fptr)(QTreeView*);
    fptr QTreeView_selectedIndexes = (fptr)(&QTreeView::selectedIndexes);   //obtaining an address
    Stub stub;
    stub.set(QTreeView_selectedIndexes, stub_eventFilter_selectedIndexes);

    m_tester->displayProcessTableContextMenu(QPoint(0, 0));
}

TEST_F(UT_ProcessTableView, test_displayProcessTableContextMenu_02)
{
    m_tester->displayProcessTableContextMenu(QPoint(0, 0));
}


TEST_F(UT_ProcessTableView, test_displayProcessTableHeaderContextMenu_01)
{
    m_tester->displayProcessTableHeaderContextMenu(QPoint(0, 0));
}

TEST_F(UT_ProcessTableView, test_resizeEvent_01)
{
    Stub stub;
    auto ProcessTable_adjustInfoLabelVisibility = get_private_fun::ProcessTableViewadjustInfoLabelVisibility();

    stub.set(ProcessTable_adjustInfoLabelVisibility, stub_search_adjustInfoLabelVisibility);

    static QResizeEvent ev(QSize(0, 0), QSize(10, 10));
    m_tester->resizeEvent(&ev);
}

TEST_F(UT_ProcessTableView, test_showEvent_01)
{
    static QShowEvent se;
    m_tester->showEvent(&se);

    EXPECT_TRUE(m_tester->m_notFoundLabel);
}

TEST_F(UT_ProcessTableView, test_selectionChanged_01)
{
    QItemSelection select;
    QItemSelection dselect;
    m_tester->selectionChanged(select, dselect);

    EXPECT_EQ(select.size(), 0);
}

TEST_F(UT_ProcessTableView, test_sizeHintForColumn_01)
{
    int value = m_tester->sizeHintForColumn(0);

    EXPECT_EQ(value, 20);
}

TEST_F(UT_ProcessTableView, test_adjustInfoLabelVisibility_01)
{
    Stub stub;
    stub.set(ADDR(MainWindow, toolbar), stub_adjustInfoLabelVisibility_toolbar);
    m_tester->adjustInfoLabelVisibility();

    EXPECT_FALSE(m_tester->m_notFoundLabel->isVisible());
}


TEST_F(UT_ProcessTableView, test_customizeProcessPriority_01)
{
    typedef int (*fptr)(DDialog*);
    fptr DDialog_exec = (fptr)((int(DDialog::*)(void))&DDialog::exec);

    Stub stub;
    stub.set(DDialog_exec, stub_endProcess_exec);
    m_tester->customizeProcessPriority();

    EXPECT_FALSE(m_tester->m_selectedPID.isValid());
}

TEST_F(UT_ProcessTableView, test_customizeProcessPriority_02)
{
    typedef int (*fptr)(DDialog*);
    fptr DDialog_exec = (fptr)((int(DDialog::*)(void))&DDialog::exec);

    Stub stub;
    stub.set(DDialog_exec, stub_endProcess_exec);
    m_tester->m_selectedPID = QVariant(100000000);
    m_tester->customizeProcessPriority();

    EXPECT_TRUE(m_tester->m_selectedPID.isValid());
}



















