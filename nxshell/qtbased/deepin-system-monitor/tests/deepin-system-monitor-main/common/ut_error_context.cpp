// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "common/error_context.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

/***************************************STUB begin*********************************************/

/***************************************STUB end**********************************************/

class UT_ErrorContext : public ::testing::Test
{
public:
    UT_ErrorContext() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new ErrorContext();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    ErrorContext *m_tester;
};

TEST_F(UT_ErrorContext, initTest)
{

}

TEST_F(UT_ErrorContext, test_ErrorContext_01)
{
    ErrorContext();
}

TEST_F(UT_ErrorContext, test_getCode_01)
{
    m_tester->getCode();
}

TEST_F(UT_ErrorContext, test_setCode_01)
{
    m_tester->setCode(1);
}

TEST_F(UT_ErrorContext, test_getSubCode_01)
{
    m_tester->getSubCode();
}

TEST_F(UT_ErrorContext, test_setSubCode_01)
{
    m_tester->setSubCode(1);
}

TEST_F(UT_ErrorContext, test_getErrorName_01)
{
    m_tester->getErrorName();
}

TEST_F(UT_ErrorContext, test_setErrorName_01)
{
    m_tester->setErrorName("1");
}

TEST_F(UT_ErrorContext, test_getErrorMessage_01)
{
    m_tester->getErrorMessage();
}

TEST_F(UT_ErrorContext, test_setErrorMessage_01)
{
    m_tester->setErrorMessage("1");
}

TEST_F(UT_ErrorContext, test_operator_01)
{
    const ErrorContext other;
    m_tester->operator==(other);
}

TEST_F(UT_ErrorContext, test_operator_02)
{
    ErrorContext copy(*m_tester);
}
