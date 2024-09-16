// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "system/device_id_cache.h"
#include "system/udev.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

using namespace core::system;

class UT_DeviceIDCache: public ::testing::Test
{
public:
    UT_DeviceIDCache() : m_tester(nullptr),m_udev(nullptr) {}

public:
    virtual void SetUp()
    {
        m_udev = new UDev;
        m_tester = new DeviceIDCache(m_udev);
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
        if (m_udev) {
            delete m_udev;
            m_udev = nullptr;
        }
    }

protected:
    DeviceIDCache *m_tester;
    UDev *m_udev;
};

TEST_F(UT_DeviceIDCache, initTest)
{
}
