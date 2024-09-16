// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "system/udev.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

using namespace core::system;

class UT_UDev: public ::testing::Test
{
public:
    UT_UDev() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new UDev();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    UDev *m_tester;
};

TEST_F(UT_UDev, initTest)
{
}
