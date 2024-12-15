// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "process/process_icon_cache.h"
#include "process/process_icon.h"
#include "process/process.h"
//gtest
#include "stub.h"
#include <gtest/gtest.h>

using namespace core::process;

class UT_ProcessIconCache : public ::testing::Test
{
public:
    UT_ProcessIconCache() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new ProcessIconCache(nullptr);
    }

    virtual void TearDown()
    {
        if(m_tester){
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    ProcessIconCache *m_tester;
};

TEST_F(UT_ProcessIconCache, initTest)
{


}

TEST_F(UT_ProcessIconCache, test_instance_001)
{
    m_tester->instance();

}

TEST_F(UT_ProcessIconCache, test_getProcessIcon_001)
{
    pid_t pid =1000;
    m_tester->getProcessIcon(pid);

}

TEST_F(UT_ProcessIconCache, test_addProcessIcon_001)
{
    pid_t pid =1000;
    ProcessIcon *icon = new ProcessIcon();
    m_tester->addProcessIcon(pid,icon);

}
