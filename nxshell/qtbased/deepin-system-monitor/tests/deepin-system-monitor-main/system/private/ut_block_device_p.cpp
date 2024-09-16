// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "system/private/block_device_p.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

using namespace core::system;

class UT_BlockDevicePrivate: public ::testing::Test
{
public:
    UT_BlockDevicePrivate() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new BlockDevicePrivate();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    BlockDevicePrivate *m_tester;
};

TEST_F(UT_BlockDevicePrivate, initTest)
{
}

TEST_F(UT_BlockDevicePrivate, test_cpoy)
{
    BlockDevicePrivate copy(*m_tester);
}

