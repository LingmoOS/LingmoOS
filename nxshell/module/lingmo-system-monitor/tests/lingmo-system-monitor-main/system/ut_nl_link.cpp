// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "system/nl_link.h"
#include <netlink/route/link.h>
#include <libnl3/netlink/route/link.h>

//gtest
#include "stub.h"
#include <gtest/gtest.h>

using namespace core::system;

class UT_NLLink: public ::testing::Test
{
public:
    UT_NLLink() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        struct rtnl_link *link;
        m_tester = new NLLink(link);
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    NLLink *m_tester;
};

TEST_F(UT_NLLink, initTest)
{
}

//TEST_F(UT_NLLink, test_link)
//{
//    rtnl_link *link = nullptr;
//    bool hold = true;
//    NLLink NLLink(link,hold);
//}
