// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "ddemointerface.h"
#include "ddemotypes.h"
#include "dfakeinterface.h"

#include <gtest/gtest.h>
#include <QDebug>
#include <QDBusObjectPath>

class TestDDemoInterface : public testing::Test
{
public:
    // 在测试套件的第一个测试用例开始前，SetUpTestCase 函数会被调用
    static void SetUpTestCase()
    {
        m_fakeInterface = new DFakeInterface();
        m_demoInterface = new Dtk::Demo::DDemoInterface();
    }
    // 在测试套件中的最后一个测试用例运行结束后，TearDownTestCase 函数会被调用
    static void TearDownTestCase()
    {
        delete m_fakeInterface;
        delete m_demoInterface;
        m_fakeInterface = nullptr;
        m_demoInterface = nullptr;
    }

    // 每个测试用例开始前，SetUp 函数都会被被调用
    void SetUp() override {}
    // 每个测试用例运行结束后，TearDown 函数都会被被调用
    void TearDown() override {}

    static inline DFakeInterface *m_fakeInterface{nullptr};
    static inline Dtk::Demo::DDemoInterface *m_demoInterface{nullptr};
};

TEST_F(TestDDemoInterface, DockedProperty)
{
    EXPECT_EQ(false, m_demoInterface->Docked());

    m_fakeInterface->setProperty("Docked", true);

    EXPECT_EQ(true, m_demoInterface->Docked());
}

TEST_F(TestDDemoInterface, ListUsers)
{
    Dtk::Demo::UserPathList_p list{};
    EXPECT_EQ(list, m_demoInterface->ListUsers().value());

    list.push_back({1000, QDBusObjectPath("/org/freedesktop/login1/user/_1000")});

    m_fakeInterface->setListUsers(list);
    auto reply = m_demoInterface->ListUsers();
    if (!reply.isValid())
        qDebug() << reply.error().message();
    EXPECT_EQ(list, reply.value());
}
