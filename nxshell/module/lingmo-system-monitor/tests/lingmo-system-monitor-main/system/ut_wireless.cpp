// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "system/wireless.h"
#include<sys/ioctl.h>

//gtest
#include "stub.h"
#include <gtest/gtest.h>

using namespace core::system;

/***************************************STUB begin*********************************************/

int stub_socket_wireless (int __domain, int __type, int __protocol)
{
    return -1;
}
int stub_ioctl_01 (int num, unsigned long int __request, ...)
{
    return -1;
}

int stub_ioctl_02 (int num, unsigned long int __request, ...)
{
    return 0;
}


/***************************************STUB end**********************************************/

class UT_wireless: public ::testing::Test
{
public:
    UT_wireless() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new wireless();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    wireless *m_tester;
};

TEST_F(UT_wireless, initTest)
{
}

TEST_F(UT_wireless, test_essid)
{
    m_tester->essid();
}

TEST_F(UT_wireless, test_link_quality)
{
    m_tester->link_quality();
}

TEST_F(UT_wireless, test_signal_levle)
{
    m_tester->signal_levle();
}

TEST_F(UT_wireless, test_noise_level)
{
    m_tester->noise_level();
}

TEST_F(UT_wireless, test_is_wireless)
{
    m_tester->m_bwireless = true;
    EXPECT_EQ(m_tester->is_wireless(), true);
}

TEST_F(UT_wireless, test_read_wireless_info_01)
{
    m_tester->read_wireless_info();
}

TEST_F(UT_wireless, test_read_wireless_info_02)
{
    Stub stub;
    stub.set(socket, stub_socket_wireless);
    m_tester->m_ifname = "enp3s0";
    m_tester->read_wireless_info();
}

TEST_F(UT_wireless, test_read_wireless_info_03)
{
    Stub stub;
    stub.set(ioctl, stub_ioctl_01);
    m_tester->m_ifname = "enp3s0";
    m_tester->read_wireless_info();
}

TEST_F(UT_wireless, test_read_wireless_info_04)
{
    Stub stub;
    stub.set(ioctl, stub_ioctl_02);
    m_tester->m_ifname = "enp3s0";
    m_tester->read_wireless_info();
}
