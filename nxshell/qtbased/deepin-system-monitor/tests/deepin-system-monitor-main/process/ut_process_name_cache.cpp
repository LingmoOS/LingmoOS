// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "process/process_name_cache.h"
//gtest
#include "stub.h"
#include <gtest/gtest.h>
//Qt
#include <QFileInfo>
#include <QApplication>
#include <QCache>
using namespace core::process;

typedef QCache<pid_t, ProcessName> myCache;
/***************************************STUB begin*********************************************/
void stub_insert_insert(){
    return;
}
/***************************************STUB end**********************************************/
class UT_ProcessNameCache : public ::testing::Test
{
public:
    UT_ProcessNameCache() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new ProcessNameCache(nullptr);
    }

    virtual void TearDown()
    {
        if(m_tester){
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    ProcessNameCache *m_tester;
};

TEST_F(UT_ProcessNameCache, initTest)
{

}

TEST_F(UT_ProcessNameCache, test_instance_001)
{
    m_tester->instance();
}

TEST_F(UT_ProcessNameCache, test_insert_001)
{
    ProcessName *name = new ProcessName;
    Stub b;
    b.set(ADDR(myCache,insert),stub_insert_insert);
    m_tester->insert(1000,name);
    delete name;
}

TEST_F(UT_ProcessNameCache, test_remove_001)
{
    m_tester->remove(1000);
}


TEST_F(UT_ProcessNameCache, test_clear_001)
{
    m_tester->clear();
}

TEST_F(UT_ProcessNameCache, test_processName_001)
{
    m_tester->processName(1000);
}

TEST_F(UT_ProcessNameCache, test_contains_001)
{
    m_tester->contains(1000);
}
