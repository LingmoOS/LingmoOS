// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "system/netlink.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

using namespace core::system;

class UT_Netlink: public ::testing::Test
{
public:
    UT_Netlink() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new Netlink();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    Netlink *m_tester;
};

TEST_F(UT_Netlink, initTest)
{
}

TEST_F(UT_Netlink, test_linkIterator)
{
    EXPECT_TRUE(m_tester->linkIterator().hasNext() == true);
}

TEST_F(UT_Netlink, test_addrIterator)
{
    EXPECT_TRUE(m_tester->addrIterator().hasNext() == true);
}


