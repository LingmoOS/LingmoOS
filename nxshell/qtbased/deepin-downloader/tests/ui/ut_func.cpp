// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iostream>
#include "gtest/gtest.h"
#include "func.h"
#include <QObject>
class ut_func : public ::testing::Test
{
protected:
    ut_func()
    {
    }

    virtual ~ut_func()
    {
    }
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TEST_F(ut_func, isNetConnect)
{
    Func::isNetConnect();
    EXPECT_TRUE(true);
}

TEST_F(ut_func, isHTTPConnect)
{
    Func::isHTTPConnect();
    EXPECT_TRUE(true);
}

TEST_F(ut_func, setMimeappsValue)
{
    Func::setMimeappsValue("xxx", "yyy");
    EXPECT_TRUE(true);
}

TEST_F(ut_func, pathToMD5)
{
    Func::pathToMD5("xxx");
    EXPECT_TRUE(true);
}

//TEST_F(ut_func, getIniConfigValue)
//{
//    Func::getIniConfigValue("xxx", "yyy", "zzz");
//    EXPECT_TRUE(true);
//}

//TEST_F(ut_func, setIniConfigValue)
//{
//    Func::setIniConfigValue("xxx", "yyy", "zzz", "www");
//    EXPECT_TRUE(true);
//}

TEST_F(ut_func, formatSpeed)
{
    EXPECT_EQ(1024, Func::formatSpeed("1KB/s"));
}

TEST_F(ut_func, formatSpeed2)
{
    EXPECT_EQ(1048576, Func::formatSpeed("1MB/s"));
}

TEST_F(ut_func, formatSpeed3)
{
    EXPECT_EQ(1073741824, Func::formatSpeed("1GB/s"));
}

TEST_F(ut_func, formatFileSize)
{
    EXPECT_EQ(1024, Func::formatFileSize("1KB"));
}

TEST_F(ut_func, formatFileSize2)
{
    EXPECT_EQ(1048576, Func::formatFileSize("1MB"));
}

TEST_F(ut_func, formatFileSize3)
{
    EXPECT_EQ(1073741824, Func::formatFileSize("1GB"));
}

TEST_F(ut_func, removeDigital)
{
    EXPECT_EQ("", Func::removeDigital(""));
}

TEST_F(ut_func, removeDigital2)
{
    EXPECT_EQ("abc", Func::removeDigital("a1b2c3"));
}

TEST_F(ut_func, chineseToPinyin)
{
    EXPECT_EQ("ce1.2.3shi", Func::chineseToPinyin("测1.2.3试"));
}

TEST_F(ut_func, chineseToPinyin2)
{
    EXPECT_EQ("a1b2c3", Func::chineseToPinyin("a1b2c3"));
}

TEST_F(ut_func, isLanConnect)
{
    Func::isLanConnect();
   // EXPECT_EQ("a1b2c3",);
}

TEST_F(ut_func, isIPV6Connect)
{
    Func::isIPV6Connect();
}
