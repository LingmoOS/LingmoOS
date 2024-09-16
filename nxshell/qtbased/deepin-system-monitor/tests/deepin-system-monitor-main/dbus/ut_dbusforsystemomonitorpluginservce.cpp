// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "dbus/dbusforsystemomonitorpluginservce.h"
#include "application.h"
//qt
#include <QList>
#include <QString>
#include <QtDBus>
#include <QDebug>

//gtest
#include "stub.h"
#include <gtest/gtest.h>

/***************************************STUB begin*********************************************/
Application::TaskState stub_getCurrentTaskState()
{
    return Application::TaskState::kTaskFinished;
}
/***************************************STUB end**********************************************/

class UT_DBusForSystemoMonitorPluginServce : public ::testing::Test
{
public:
    UT_DBusForSystemoMonitorPluginServce() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        static MainWindow mainwindow;
        gApp->setMainWindow(&mainwindow);
        m_tester = new DBusForSystemoMonitorPluginServce();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    DBusForSystemoMonitorPluginServce *m_tester;
};

TEST_F(UT_DBusForSystemoMonitorPluginServce, initTest)
{

}

TEST_F(UT_DBusForSystemoMonitorPluginServce, test_slotJumpDetailWidget_01)
{
    Stub stub;
    stub.set(ADDR(Application, getCurrentTaskState), stub_getCurrentTaskState);
    m_tester->slotJumpDetailWidget("MSG_CPU");
}

TEST_F(UT_DBusForSystemoMonitorPluginServce, test_slotJumpProcessWidget_01)
{
    Stub stub;
    stub.set(ADDR(Application, getCurrentTaskState), stub_getCurrentTaskState);
    m_tester->slotJumpProcessWidget("MSG_PROCESS");
}

TEST_F(UT_DBusForSystemoMonitorPluginServce, test_slotRaiseWindow_01)
{
    Stub stub;
    stub.set(ADDR(Application, getCurrentTaskState), stub_getCurrentTaskState);
    m_tester->slotRaiseWindow();
}
