// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "service/service_manager_worker.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

static QString m_Sresult;
/***************************************STUB begin*********************************************/

int stub_readUnitDescriptionFromUnitFile_open1(){
    m_Sresult = "open success";
    return 1;
}

int stub_readUnitDescriptionFromUnitFile_open2(){
    m_Sresult = "open failed";
    return -1;
}

ssize_t stub_readUnitDescriptionFromUnitFile_read1(){
    m_Sresult = "read success";
    return 1;
}

ssize_t stub_readUnitDescriptionFromUnitFile_read2(){
    m_Sresult = "read failed";
    return -1;
}

int stub_readUnitDescriptionFromUnitFile_close(){
    m_Sresult = "close";
    return 1;
}

/***************************************STUB end**********************************************/

class UT_ServiceManagerWorker : public ::testing::Test
{
public:
    UT_ServiceManagerWorker() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new ServiceManagerWorker();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    ServiceManagerWorker *m_tester;
};

TEST_F(UT_ServiceManagerWorker, initTest)
{

}

TEST_F(UT_ServiceManagerWorker, test_startJob_01)
{
    m_tester->startJob();
}
