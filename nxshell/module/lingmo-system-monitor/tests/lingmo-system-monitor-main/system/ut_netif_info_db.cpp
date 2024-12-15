// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "system/netif_info_db.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

using namespace core::system;

class UT_NetifInfoDB: public ::testing::Test
{
public:
    UT_NetifInfoDB() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new NetifInfoDB();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    NetifInfoDB *m_tester;
};

TEST_F(UT_NetifInfoDB, initTest)
{
}

TEST_F(UT_NetifInfoDB, test_update_addr)
{
    m_tester->update_addr();
    EXPECT_TRUE(m_tester->m_addrIpv4DB.size() > 0);
}

TEST_F(UT_NetifInfoDB, test_update_netif_info)
{
    m_tester->update_netif_info();
}

TEST_F(UT_NetifInfoDB, test_update)
{
    m_tester->update();

}
