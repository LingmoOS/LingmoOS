// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "process/process_controller.h"
//gtest
#include "stub.h"
#include <gtest/gtest.h>
//Qt
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QIODevice>

static QString m_Sresult;
/***************************************STUB begin*********************************************/

void stub_prio_execute_start(){
    m_Sresult = "start";
    return;
}

/***************************************STUB end**********************************************/

class UT_ProcessController : public ::testing::Test
{
public:
    UT_ProcessController() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        pid_t pid = 100000;
        int signal = SIGCONT;
        m_tester = new ProcessController(pid,signal);
    }

    virtual void TearDown()
    {
        if(m_tester){
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    ProcessController *m_tester;
};

TEST_F(UT_ProcessController, initTest)
{

}

TEST_F(UT_ProcessController, test_execute_001)
{
    
}
