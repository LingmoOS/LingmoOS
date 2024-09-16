// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "system/private/netif_p.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

using namespace core::system;

class UT_NetifInfoPrivate: public ::testing::Test
{
public:
    UT_NetifInfoPrivate() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new NetifInfoPrivate();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    NetifInfoPrivate *m_tester;
};

TEST_F(UT_NetifInfoPrivate, initTest)
{
}

TEST_F(UT_NetifInfoPrivate, test_cpoy)
{
    NetifInfoPrivate copy(*m_tester);
}

