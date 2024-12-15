// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "common.h"

#include "gtest/src/stub.h"

#include <gtest/gtest.h>

#include <QTextCodec>
/*******************************函数打桩************************************/

/*******************************函数打桩************************************/


// 测试Common
class UT_Common : public ::testing::Test
{
public:
    UT_Common(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new Common;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    Common *m_tester;
};

TEST_F(UT_Common, initTest)
{

}

TEST_F(UT_Common, test_codecConfidenceForData)
{
    EXPECT_EQ(m_tester->codecConfidenceForData(QTextCodec::codecForName("GBK"), "哈哈", QLocale::China), 1);
}

TEST_F(UT_Common, test_trans2uft8_001)
{
    QByteArray strCode;
    EXPECT_EQ(m_tester->trans2uft8("哈哈", strCode), "哈哈");
}

TEST_F(UT_Common, test_trans2uft8_002)
{
    QByteArray strCode;
    EXPECT_EQ(m_tester->trans2uft8("abc", strCode), "abc");
}

TEST_F(UT_Common, test_detectEncode)
{

}

TEST_F(UT_Common, test_ChartDet_DetectingTextCoding)
{

}

TEST_F(UT_Common, test_textCodecDetect)
{

}
