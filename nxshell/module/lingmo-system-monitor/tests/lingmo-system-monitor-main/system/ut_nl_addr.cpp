// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

////self
//#include "system/nl_addr.h"
//#include <netlink/netlink.h>
//#include <netlink/cache.h>
//#include <netlink/addr.h>
//#include <netlink/route/link.h>
//#include <netlink/route/addr.h>
//#include <netlink/addr.h>
//#include <libnl3/netlink/route/addr.h>

////gtest
//#include "stub.h"
//#include <gtest/gtest.h>

//using namespace core::system;

//class UT_NLAddr: public ::testing::Test
//{
//public:
//    UT_NLAddr() : m_tester(nullptr) {}

//public:
//    virtual void SetUp()
//    {
//        rtnl_addr addr;
//        memset(&addr, 0, sizeof (addr));
//        m_tester = new NLAddr(&addr);
//    }

//    virtual void TearDown()
//    {
//        if (m_tester) {
//        delete m_tester;
//        m_tester = nullptr;
//            }
//    }

//protected:
//    NLAddr *m_tester;
//};

//TEST_F(UT_NLAddr, initTest)
//{
//}

//TEST_F(UT_NLAddr, test_ifindex)
//{
//    auto ok = m_tester->ifindex();
//    EXPECT_NE(ok, 0);
//}

//TEST_F(UT_NLAddr, test_family)
//{
//    auto ok = m_tester->family();
//    EXPECT_NE(ok, 0);
//}

//TEST_F(UT_NLAddr, test_prefixlen)
//{
//    auto ok = m_tester->prefixlen();
//    EXPECT_NE(ok, 0);
//}

