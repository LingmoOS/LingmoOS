// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dmimedatabase.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>
#include <QDir>


/*******************************函数打桩************************************/
QString qQMimeType_name_stub()
{
    return "application/zip";
}
/*******************************单元测试************************************/
// 测试DMimeDatabase
class UT_DMimeDatabase : public ::testing::Test
{
public:
    UT_DMimeDatabase(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new DMimeDatabase;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    DMimeDatabase *m_tester;
};

TEST_F(UT_DMimeDatabase, initTest)
{

}

TEST_F(UT_DMimeDatabase, test_mimeTypeForFile_001)
{
    QMimeType mime = m_tester->mimeTypeForFile("1.docx");
    EXPECT_EQ(mime.name().isEmpty(), false);
}

TEST_F(UT_DMimeDatabase, test_mimeTypeForFile_002)
{
    Stub stub;
    stub.set(ADDR(QMimeType, name), qQMimeType_name_stub);
    QMimeType mime = m_tester->mimeTypeForFile("1.docx", QMimeDatabase::MatchExtension);
    qInfo() << mime.name();
    EXPECT_EQ(mime.name(), "application/zip");
}
