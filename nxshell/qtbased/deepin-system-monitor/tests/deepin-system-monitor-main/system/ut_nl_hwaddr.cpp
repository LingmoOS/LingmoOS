// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "system/nl_hwaddr.h"
#include <net/if_arp.h>
#include <sys/ioctl.h>

//gtest
#include "stub.h"
#include <gtest/gtest.h>

/***************************************STUB end**********************************************/

int stub_ioctl_hwaddr(int __fd, unsigned long int __request, ...)
{
    return 0;
}

/***************************************STUB end**********************************************/

class UT_NLHWAddr: public ::testing::Test
{
public:
    UT_NLHWAddr() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        QByteArray ifname;
        m_tester = new NLHWAddr(ifname);
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    NLHWAddr *m_tester;
};

TEST_F(UT_NLHWAddr, initTest)
{
}

TEST_F(UT_NLHWAddr, test_initData_01)
{
    m_tester->initData();
//    EXPECT_NE(m_tester->m_sa_family, 0);
}

