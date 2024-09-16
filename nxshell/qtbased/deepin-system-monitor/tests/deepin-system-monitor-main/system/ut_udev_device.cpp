// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "system/udev_device.h"
#include "system/udev.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

using namespace core::system;

class UT_UDevDevice: public ::testing::Test
{
public:
    UT_UDevDevice() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        using HANDLE = struct udev_device *;
        HANDLE udevice = nullptr;
        m_tester = new UDevDevice(udevice);
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    UDevDevice *m_tester;
};

TEST_F(UT_UDevDevice, initTest)
{

}

TEST_F(UT_UDevDevice, test_UDevDevice)
{
    UDev *udev = new UDev;
    char *path = nullptr;
    UDevDevice *m_tester2 = new UDevDevice(udev, path);
    delete udev;
    delete m_tester2;
}
