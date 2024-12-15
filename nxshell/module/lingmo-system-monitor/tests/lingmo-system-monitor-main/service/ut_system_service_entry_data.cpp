// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "service/system_service_entry_data.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

/***************************************STUB begin*********************************************/

/***************************************STUB end**********************************************/

class UT_SystemServiceEntryData : public ::testing::Test
{
public:
    UT_SystemServiceEntryData() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new SystemServiceEntryData();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    SystemServiceEntryData *m_tester;
};

TEST_F(UT_SystemServiceEntryData, initTest)
{

}

TEST_F(UT_SystemServiceEntryData, test_operator_01)
{
    EXPECT_EQ(m_tester->m_id.isEmpty(), true);
}

TEST_F(UT_SystemServiceEntryData, test_operator_02)
{
    EXPECT_EQ(m_tester->m_sname.isEmpty(), true);
}

TEST_F(UT_SystemServiceEntryData, test_operator_03)
{
    EXPECT_EQ(m_tester->m_loadState.isEmpty(), true);
}

TEST_F(UT_SystemServiceEntryData, test_operator_04)
{
    EXPECT_EQ(m_tester->m_activeState.isEmpty(), true);
}

TEST_F(UT_SystemServiceEntryData, test_operator_05)
{
    EXPECT_EQ(m_tester->m_subState.isEmpty(), true);
}

TEST_F(UT_SystemServiceEntryData, test_operator_06)
{
    EXPECT_EQ(m_tester->m_state.isEmpty(), true);
}

TEST_F(UT_SystemServiceEntryData, test_operator_07)
{
    EXPECT_EQ(m_tester->m_startupType.isEmpty(), true);
}

TEST_F(UT_SystemServiceEntryData, test_operator_08)
{
    EXPECT_EQ(m_tester->m_unitObjectPath.isEmpty(), true);
}

TEST_F(UT_SystemServiceEntryData, test_operator_09)
{
    EXPECT_EQ(m_tester->m_description.isEmpty(), true);
}

TEST_F(UT_SystemServiceEntryData, test_operator_10)
{
    EXPECT_EQ(m_tester->m_canReload, false);
}

TEST_F(UT_SystemServiceEntryData, test_operator_11)
{
    EXPECT_EQ(m_tester->m_canStart, false);
}

TEST_F(UT_SystemServiceEntryData, test_operator_12)
{
    EXPECT_EQ(m_tester->m_canStop, false);
}

TEST_F(UT_SystemServiceEntryData, test_operator_13)
{
    const SystemServiceEntryData rhs;
    m_tester->operator=(rhs);
}

TEST_F(UT_SystemServiceEntryData, test_operator_14)
{
    SystemServiceEntryData copy(*m_tester);
}
