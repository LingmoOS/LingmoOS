// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "process/process_name.h"
#include "process/process.h"
#include "process/private/process_p.h"
#include "process/process_db.h"
#include "process/process_name_cache.h"
#include "process/desktop_entry_cache.h"
#include "common/common.h"
#include "common/thread_manager.h"
#include "system/system_monitor_thread.h"
#include "system/system_monitor.h"
#include "wm/wm_window_list.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
//Qt
#include <QFileInfo>
#include <QApplication>
#include <QList>
using namespace core::process;
/***************************************STUB begin*********************************************/
bool stub_getDisplayName_isEmpty(){
    return false;
}

bool stub_getDisplayName_isTrayApp(){
    return true;
}
bool stub_getDisplayName_isGuiApp(){
    return true;
}

bool stub_getDisplayName_StringisEmpty(){
    return false;
}
/***************************************STUB end**********************************************/
class UT_ProcessName : public ::testing::Test
{
public:
    UT_ProcessName() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new ProcessName();
    }

    virtual void TearDown()
    {
        if(m_tester){
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    ProcessName *m_tester;
};

TEST_F(UT_ProcessName, initTest)
{

}

TEST_F(UT_ProcessName, test_refreashProcessName_001)
{
    Process *proc = new Process;
    m_tester->refreashProcessName(proc);
    delete proc;
}

TEST_F(UT_ProcessName, test_normalizeProcessName_001)
{
    QString source = "aaaaa";
    QByteArrayList cmdline {};
    QString expect = m_tester->normalizeProcessName(source,cmdline);
    EXPECT_EQ(expect,source);
}

TEST_F(UT_ProcessName, test_normalizeProcessName_002)
{
    QString source = "aaaaaaaaaaaaaaaaaaaaaa";
    QByteArrayList cmdline {};
    cmdline << "/usr/bin/deepin-system-monitor";
    QString expect = m_tester->normalizeProcessName(source,cmdline);
}

TEST_F(UT_ProcessName, test_getDisplayName_001)
{
    Process *proc = new Process;
    m_tester->getDisplayName(proc);
    delete proc;
}

TEST_F(UT_ProcessName, test_getDisplayName_002)
{
    Process *proc = new Process;
    Stub b1;
    b1.set(ADDR(QByteArrayList,isEmpty),stub_getDisplayName_isEmpty);
    Stub b2;
    b2.set(ADDR(WMWindowList,isTrayApp),stub_getDisplayName_isTrayApp);
    Stub b3;
    b3.set(ADDR(QString,isEmpty),stub_getDisplayName_StringisEmpty);
    m_tester->getDisplayName(proc);

    delete proc;
}

TEST_F(UT_ProcessName, test_getDisplayName_003)
{
    Process *proc = new Process;
    Stub b1;
    b1.set(ADDR(QByteArrayList,isEmpty),stub_getDisplayName_isEmpty);
    Stub b2;
    b2.set(ADDR(WMWindowList,isTrayApp),stub_getDisplayName_isTrayApp);

    m_tester->getDisplayName(proc);

    delete proc;
}

TEST_F(UT_ProcessName, test_getDisplayName_004)
{
    Process *proc = new Process;
    Stub b1;
    b1.set(ADDR(QByteArrayList,isEmpty),stub_getDisplayName_isEmpty);
    Stub b2;
    b2.set(ADDR(WMWindowList,isGuiApp),stub_getDisplayName_isGuiApp);
    Stub b3;
    b3.set(ADDR(QString,isEmpty),stub_getDisplayName_StringisEmpty);
    m_tester->getDisplayName(proc);

    delete proc;
}

TEST_F(UT_ProcessName, test_getDisplayName_005)
{
    Process *proc = new Process;
    Stub b1;
    b1.set(ADDR(QByteArrayList,isEmpty),stub_getDisplayName_isEmpty);
    Stub b2;
    b2.set(ADDR(WMWindowList,isGuiApp),stub_getDisplayName_isGuiApp);
    Stub b3;
    b3.set(ADDR(QString,isEmpty),stub_getDisplayName_StringisEmpty);
    m_tester->getDisplayName(proc);

    delete proc;
}
