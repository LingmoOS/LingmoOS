// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "process/process_icon.h"
#include "process/private/process_p.h"
#include "process/process.h"
#include "process/desktop_entry_cache.h"
#include "wm/wm_window_list.h"
//gtest
#include "stub.h"
#include <gtest/gtest.h>
//Qt
#include <QFileInfo>

using namespace core::process;
using namespace core::wm;
/***************************************STUB begin**********************************************/
bool stub_getIcon_isEmpty(){
    return false;
}

bool stub_str_isEmpty(){
    return false;
}

bool stub_getIcon_isTrayApp(){
    return true;
}
/***************************************STUB end**********************************************/
class UT_ProcessIcon : public ::testing::Test
{
public:
    UT_ProcessIcon() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new ProcessIcon();
    }

    virtual void TearDown()
    {
        if(m_tester){
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    ProcessIcon *m_tester;
};

TEST_F(UT_ProcessIcon, initTest)
{

}

TEST_F(UT_ProcessIcon, test_refreashProcessIcon_001)
{
    Process *proc = new Process();
    m_tester->refreashProcessIcon(proc);
    delete proc;
}

TEST_F(UT_ProcessIcon, test_icon_001)
{
    m_tester->icon();

}

TEST_F(UT_ProcessIcon, test_defaultIconData_001)
{
    QString proc = "111";
     m_tester->defaultIconData(proc);
}

TEST_F(UT_ProcessIcon, test_terminalIconData_001)
{
    QString proc = "111";
     m_tester->terminalIconData(proc);
}

TEST_F(UT_ProcessIcon, test_getIcon_001)
{
    Process *proc = new Process();
    m_tester->getIcon(proc);
    delete proc;
}

TEST_F(UT_ProcessIcon, test_getIcon_002)
{
    Process *proc = new Process();
    Stub b;
    b.set(ADDR(QByteArrayList,isEmpty),stub_getIcon_isEmpty);
    QByteArrayList cmdline;
    cmdline << "/opt/null";
    proc->d->cmdline = cmdline;
    m_tester->getIcon(proc);
    delete proc;
}

TEST_F(UT_ProcessIcon, test_getIcon_003)
{
    Process *proc = new Process();
    Stub b;
    b.set(ADDR(QByteArrayList,isEmpty),stub_getIcon_isEmpty);
    Stub b1;
    b1.set(ADDR(QString,isEmpty),stub_str_isEmpty);
    QByteArrayList cmdline;
    cmdline << "/opt/null";
    proc->d->cmdline = cmdline;
    m_tester->getIcon(proc);
    delete proc;
}

TEST_F(UT_ProcessIcon, test_getIcon_004)
{
    Process *proc = new Process();
    Stub b;
    b.set(ADDR(QByteArrayList,isEmpty),stub_getIcon_isEmpty);
    Stub b1;
    b1.set(ADDR(WMWindowList,isTrayApp),stub_getIcon_isTrayApp);
    proc->d->environ.insert("GIO_LAUNCHED_DESKTOP_FILE","1");
    QByteArrayList cmdline;
    cmdline << "/opt/null";
    proc->d->cmdline = cmdline;
    m_tester->getIcon(proc);
    delete proc;
}

TEST_F(UT_ProcessIcon, test_getIcon_005)
{
    Process *proc = new Process();
    Stub b;
    b.set(ADDR(QByteArrayList,isEmpty),stub_getIcon_isEmpty);
    Stub b1;
    b1.set(ADDR(WMWindowList,isGuiApp),stub_getIcon_isTrayApp);
    proc->d->environ.insert("GIO_LAUNCHED_DESKTOP_FILE","1");
    QByteArrayList cmdline;
    cmdline << "/opt/null";
    proc->d->cmdline = cmdline;
    m_tester->getIcon(proc);
    delete proc;
}


TEST_F(UT_ProcessIcon, test_getIcon_006)
{
    Process *proc = new Process();
    Stub b;
    b.set(ADDR(QByteArrayList,isEmpty),stub_getIcon_isEmpty);
    Stub b1;
    b1.set(ADDR(DesktopEntryCache,contains),stub_getIcon_isTrayApp);
    proc->d->environ.insert("GIO_LAUNCHED_DESKTOP_FILE","1");
    QByteArrayList cmdline;
    cmdline << "/opt/null";
    proc->d->cmdline = cmdline;
    m_tester->getIcon(proc);
    delete proc;
}

TEST_F(UT_ProcessIcon, test_getIcon_007)
{
    Process *proc = new Process();
    Stub b;
    b.set(ADDR(QByteArrayList,isEmpty),stub_getIcon_isEmpty);
    Stub b1;
    b1.set(ADDR(DesktopEntryCache,contains),stub_getIcon_isTrayApp);
    proc->d->environ.insert("GIO_LAUNCHED_DESKTOP_FILE","1");
     proc->d->environ.insert("GIO_LAUNCHED_DESKTOP_FILE_PID","1000");
     proc->d->pid = 1000;
    QByteArrayList cmdline;
    cmdline << "/opt/null";
    proc->d->cmdline = cmdline;
    m_tester->getIcon(proc);
    delete proc;
}

TEST_F(UT_ProcessIcon, test_getIcon_008)
{
    Process *proc = new Process();
    Stub b;
    b.set(ADDR(QByteArrayList,isEmpty),stub_getIcon_isEmpty);
    Stub b1;
    b1.set(ADDR(QList<QString> ,contains),stub_getIcon_isTrayApp);
    m_tester->getIcon(proc);
    delete proc;
}
