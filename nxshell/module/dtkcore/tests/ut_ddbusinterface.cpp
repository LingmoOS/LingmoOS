// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <ddbusinterface.h>

#include "fakedbus/fakedbusservice.h"
#include <QTest>
#include <QSignalSpy>

using Dtk::Core::DDBusInterface;

class ut_DDBusInterface : public testing::Test
{
public:
    void SetUp() override
    {
        m_testservice = new FakeDBusService();
        m_testDBusInterfaceParent = new FakeDBusServiceParent();
        // if parent is nullptr
        m_testInterface = new DDBusInterface(FakeDBusService::get_service(),
                                             FakeDBusService::get_path(),
                                             FakeDBusService::get_interface(),
                                             QDBusConnection::sessionBus());
    }
    void TearDown() override
    {
        delete m_testDBusInterfaceParent;
        delete m_testInterface;
        delete m_testservice;
    }
    DDBusInterface *m_testInterface;
    FakeDBusService *m_testservice;
    FakeDBusServiceParent *m_testDBusInterfaceParent;
};

TEST_F(ut_DDBusInterface, TestProperty)
{
    auto objectpaths = m_testservice->objectpaths();
    EXPECT_EQ(m_testDBusInterfaceParent->objectpaths().length(), objectpaths.length());

    auto strproperty = qvariant_cast<QString>(m_testInterface->property("strProperty"));
    EXPECT_EQ(strproperty, m_testservice->strproperty());
}

TEST_F(ut_DDBusInterface, suffix)
{
    EXPECT_EQ(m_testInterface->suffix(), "");
    m_testInterface->setSuffix("-suffix");
    EXPECT_EQ(m_testInterface->suffix(), "-suffix");
}
