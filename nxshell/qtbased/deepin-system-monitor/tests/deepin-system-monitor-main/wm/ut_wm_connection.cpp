// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "wm/wm_connection.h"
//gtest
#include "stub.h"
#include <gtest/gtest.h>
//Qt

using namespace core::wm;

class UT_WMConnection: public ::testing::Test
{
public:
    UT_WMConnection() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new WMConnection();
    }

    virtual void TearDown()
    {
        if(m_tester){
            delete m_tester;
            m_tester=nullptr;
        }
    }

protected:
    WMConnection *m_tester;
};

TEST_F(UT_WMConnection, initTest)
{

}

