// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "service/system_service_entry.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

/***************************************STUB begin*********************************************/

/***************************************STUB end**********************************************/

class UT_SystemServiceEntry : public ::testing::Test
{
public:
    UT_SystemServiceEntry() : m_tester(nullptr) , m_tester1(nullptr) {}

public:
    virtual void SetUp()
    {
        QString id("1");
        QString sname("1");
        QString loadState("1");
        QString activeState("1");
        QString subState("1");
        QString state("1");
        QString startupType("1");
        QString unitObjectPath("1");
        QString description("1");
        quint32 mainPID = 1;
        bool canReload {true};
        bool canStart {true};
        bool canStop {true};
        m_tester = new SystemServiceEntry();
        m_tester1 = new SystemServiceEntry(id, sname, loadState, activeState, subState, state,
                                           startupType, unitObjectPath, description, mainPID, canReload, canStart, canStop);
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
        if (m_tester1) {
            delete m_tester1;
            m_tester1 = nullptr;
        }
    }

protected:
    SystemServiceEntry *m_tester;
    SystemServiceEntry *m_tester1;
};

TEST_F(UT_SystemServiceEntry, initTest)
{

}

TEST_F(UT_SystemServiceEntry, test_operator_01)
{
    EXPECT_EQ(m_tester->data,true);
}

TEST_F(UT_SystemServiceEntry, test_operator_02)
{
    SystemServiceEntry copy(*m_tester);
}

