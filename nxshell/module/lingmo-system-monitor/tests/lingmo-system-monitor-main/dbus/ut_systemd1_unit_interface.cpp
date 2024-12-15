// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "dbus/systemd1_unit_interface.h"
#include "dbus/dbus_properties_interface.h"
#include "dbus/systemd1_unit_interface.h"
#include "dbus/dbus_common.h"
using namespace dbus::common;

//qt
#include <QByteArray>
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QtDBus>

//gtest
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

/***************************************STUB begin*********************************************/

/***************************************STUB end**********************************************/

class UT_Systemd1UnitInterface : public ::testing::Test
{
public:
    UT_Systemd1UnitInterface() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new Systemd1UnitInterface("org.freedesktop.systemd1", "/org/freedesktop/systemd1/unit/virtlogd_2eservice",
                                             QDBusConnection::sessionBus());
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    Systemd1UnitInterface *m_tester;
};

TEST_F(UT_Systemd1UnitInterface, initTest)
{

}

TEST_F(UT_Systemd1UnitInterface, test_getId_01)
{
    QPair<ErrorContext, QString> reply = m_tester->getId();
}

TEST_F(UT_Systemd1UnitInterface, test_getLoadState_01)
{
    QPair<ErrorContext, QString> reply = m_tester->getLoadState();
}

TEST_F(UT_Systemd1UnitInterface, test_getActiveState_01)
{
    QPair<ErrorContext, QString> reply = m_tester->getActiveState();
}

TEST_F(UT_Systemd1UnitInterface, test_getSubState_01)
{
    QPair<ErrorContext, QString> reply = m_tester->getSubState();
}

TEST_F(UT_Systemd1UnitInterface, test_getDescription_01)
{
    QPair<ErrorContext, QString> reply = m_tester->getDescription();
}

TEST_F(UT_Systemd1UnitInterface, test_canReload_01)
{
    QPair<ErrorContext, bool> reply = m_tester->canReload();
}

TEST_F(UT_Systemd1UnitInterface, test_canStart_001)
{
    QPair<ErrorContext, bool> reply = m_tester->canStart();
}

TEST_F(UT_Systemd1UnitInterface, test_canStop_01)
{
    QPair<ErrorContext, bool> reply = m_tester->canStop();
}
