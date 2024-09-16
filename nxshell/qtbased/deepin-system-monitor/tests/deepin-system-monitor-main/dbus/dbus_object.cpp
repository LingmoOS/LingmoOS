// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "dbus/dbus_object.h"
#include "gui/main_window.h"
#include "application.h"

//qt
#include <QDBusConnection>
#include <QDebug>

#define DBUS_SERVER             "com.deepin.systemMonitor"
#define DBUS_SERVER_PATH        "/com/deepin/systemMonitor"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

/***************************************STUB begin*********************************************/

/***************************************STUB end**********************************************/

class UT_DbusObject : public ::testing::Test
{
public:
    UT_DbusObject() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new DBusObject();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    DBusObject *m_tester;
};

TEST_F(UT_DbusObject, initTest)
{

}

TEST_F(UT_DbusObject, test_getInstance_01)
{
    m_tester->getInstance();
}

TEST_F(UT_DbusObject, test_registerOrNotify_01)
{
    m_tester->registerOrNotify();
}

TEST_F(UT_DbusObject, test_unRegister_01)
{
    m_tester->unRegister();
}





