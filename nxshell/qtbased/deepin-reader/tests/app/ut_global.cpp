// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Global.h"
#include "stub.h"

//#include <QWidget>
#include <QMimeType>

#include <gtest/gtest.h>
using namespace Dr;
/********测试DBusObject***********/
class TestGlobal : public ::testing::Test
{
public:
    virtual void SetUp();

    virtual void TearDown();
};

void TestGlobal::SetUp()
{

}

void TestGlobal::TearDown()
{

}

/***********桩函数*************/
QString name_stub()
{
    return "application/postscript";
}

QString name_stub2()
{
    return "application/x-ole-storage";
}

/***********测试用例***********/
TEST_F(TestGlobal, UT_Global_fileType_001)
{
    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    EXPECT_TRUE(fileType(strPath) == PDF);
}

TEST_F(TestGlobal, UT_Global_fileType_002)
{
    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.djvu";
    EXPECT_TRUE(fileType(strPath) == DJVU);
}

TEST_F(TestGlobal, UT_Global_fileType_003)
{
    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pptx";
    EXPECT_TRUE(fileType(strPath) == PPTX);
}

TEST_F(TestGlobal, UT_Global_fileType_004)
{
    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.docx";
    EXPECT_TRUE(fileType(strPath) == DOCX);
}

TEST_F(TestGlobal, UT_Global_fileType_005)
{
    Stub s;
    s.set(ADDR(QMimeType, name), name_stub);

    EXPECT_TRUE(fileType("1.ps") == PS);
}

TEST_F(TestGlobal, UT_Global_fileType_006)
{
    Stub s;
    s.set(ADDR(QMimeType, name), name_stub2);

    EXPECT_TRUE(fileType("1.docx") == DOCX);
}
