// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"

#include "interfaces/daemongrandsearchinterface.h"

#include "stubext.h"

#include <gtest/gtest.h>

using namespace testing;
GRANDSEARCH_USE_NAMESPACE

namespace {
class DaemonGrandSearchInterfaceTest : public Test
{
public:
    DaemonGrandSearchInterfaceTest() : Test()
    {
        stu = new stub_ext::StubExt;
        stu->set_lamda(ADDR(QDBusAbstractInterface, asyncCallWithArgumentList), [&](){
            isCall = true;
            QDBusMessage message;
            return QDBusPendingCall::fromCompletedCall(message);
        });

        interface = new DaemonGrandSearchInterface;
    }
    ~DaemonGrandSearchInterfaceTest()
    {
        delete stu;
        delete interface;
    }

    stub_ext::StubExt *stu = nullptr;
    DaemonGrandSearchInterface *interface;
    bool isCall = false;
};

}

TEST_F(DaemonGrandSearchInterfaceTest, init)
{
    ASSERT_TRUE(interface);
}

TEST_F(DaemonGrandSearchInterfaceTest, Configuration)
{
    interface->Configuration();
    EXPECT_TRUE(isCall);
}
TEST_F(DaemonGrandSearchInterfaceTest, Configure)
{
    QVariantMap data;
    data.insert("testKey", "tsetValue");
    interface->Configure(data);
    EXPECT_TRUE(isCall);
}
TEST_F(DaemonGrandSearchInterfaceTest, FeedBackStrategy)
{
    interface->FeedBackStrategy();
    EXPECT_TRUE(isCall);
}
TEST_F(DaemonGrandSearchInterfaceTest, KeepAlive)
{
    QString missionId("testId");
    interface->KeepAlive(missionId);
    EXPECT_TRUE(isCall);
}
TEST_F(DaemonGrandSearchInterfaceTest, MatchedBuffer)
{
    QString missionId("testId");
    interface->MatchedBuffer(missionId);
    EXPECT_TRUE(isCall);
}
TEST_F(DaemonGrandSearchInterfaceTest, MatchedResults)
{
    QString missionId("testId");
    interface->MatchedResults(missionId);
    EXPECT_TRUE(isCall);
}
TEST_F(DaemonGrandSearchInterfaceTest, OpenWithPlugin)
{
    QString searcher("testSearcher");
    QString item("testItem");
    interface->OpenWithPlugin(searcher, item);
    EXPECT_TRUE(isCall);
}
TEST_F(DaemonGrandSearchInterfaceTest, Search)
{
    QString missionId("testId");
    QString key("testKey");
    interface->Search(missionId, key);
    EXPECT_TRUE(isCall);
}
TEST_F(DaemonGrandSearchInterfaceTest, SetFeedBackStrategy)
{
    QVariantMap data;
    data.insert("testKey", "tsetValue");
    interface->SetFeedBackStrategy(data);
    EXPECT_TRUE(isCall);
}
TEST_F(DaemonGrandSearchInterfaceTest, Terminate)
{
    interface->Terminate();
    EXPECT_TRUE(isCall);
}
