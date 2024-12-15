// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "uistruct.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>
#include <QDir>


/*******************************函数打桩************************************/

/*******************************单元测试************************************/
// 测试CompressParameter
class UT_CompressParameter : public ::testing::Test
{
public:
    UT_CompressParameter(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CompressParameter;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CompressParameter *m_tester;
};

TEST_F(UT_CompressParameter, initTest)
{

}



// 测试UnCompressParameter
class UT_UnCompressParameter : public ::testing::Test
{
public:
    UT_UnCompressParameter(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new UnCompressParameter;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    UnCompressParameter *m_tester;
};

TEST_F(UT_UnCompressParameter, initTest)
{

}
