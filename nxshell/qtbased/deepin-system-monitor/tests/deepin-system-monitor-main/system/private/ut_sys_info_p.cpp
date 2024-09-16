// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "system/private/sys_info_p.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

using namespace core::system;

class UT_SysInfoPrivate: public ::testing::Test
{
public:
    UT_SysInfoPrivate() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new SysInfoPrivate();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    SysInfoPrivate *m_tester;
};

TEST_F(UT_SysInfoPrivate, initTest)
{
}

TEST_F(UT_SysInfoPrivate, test_cpoy)
{
    SysInfoPrivate copy(*m_tester);
}

