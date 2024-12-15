// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "system/private/mem_p.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

using namespace core::system;

class UT_MemInfoPrivate: public ::testing::Test
{
public:
    UT_MemInfoPrivate() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new MemInfoPrivate();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    MemInfoPrivate *m_tester;
};

TEST_F(UT_MemInfoPrivate, initTest)
{
}

TEST_F(UT_MemInfoPrivate, test_cpoy)
{
    MemInfoPrivate copy(*m_tester);
}

