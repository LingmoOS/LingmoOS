// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"

#include "interfaces/grandsearchinterface.h"

#include "stubext.h"

#include <gtest/gtest.h>

using namespace testing;
GRANDSEARCH_USE_NAMESPACE

namespace {
class GrandSearchInterfaceTest : public Test
{
public:
    GrandSearchInterfaceTest() : Test()
    {
        stu = new stub_ext::StubExt;
        stu->set_lamda(ADDR(QDBusAbstractInterface, asyncCallWithArgumentList), [&](){
            isCall = true;
            QDBusMessage message;
            return QDBusPendingCall::fromCompletedCall(message);
        });

        interface = new GrandSearchInterface;
    }
    ~GrandSearchInterfaceTest()
    {
        delete stu;
        delete interface;
    }

    stub_ext::StubExt *stu = nullptr;
    GrandSearchInterface *interface;
    bool isCall = false;
};

}

TEST_F(GrandSearchInterfaceTest, IsVisible)
{
    interface->IsVisible();

    EXPECT_TRUE(isCall);
}

TEST_F(GrandSearchInterfaceTest, SetVisible)
{
    interface->SetVisible(false);

    EXPECT_TRUE(isCall);
}
