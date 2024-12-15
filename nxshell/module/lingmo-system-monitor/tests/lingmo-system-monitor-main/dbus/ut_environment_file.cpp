// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "dbus/environment_file.h"

//qt
#include <QList>
#include <QString>
#include <QtDBus>
#include <QDebug>

//gtest
#include "stub.h"
#include <gtest/gtest.h>

/***************************************STUB begin*********************************************/

/***************************************STUB end**********************************************/

class UT_EnvironmentFile : public ::testing::Test
{
public:
    UT_EnvironmentFile() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new EnvironmentFile();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    EnvironmentFile *m_tester;
};

TEST_F(UT_EnvironmentFile, initTest)
{

}

TEST_F(UT_EnvironmentFile, test_copyContruct_01)
{
    EnvironmentFile file1;
    EnvironmentFile file2(file1);
}


TEST_F(UT_EnvironmentFile, test_Operator_01)
{
    EnvironmentFile aFile;
    aFile.envFile = "a";
    aFile.flag = 0;
    m_tester->envFile = "a";

    EnvironmentFile bFile;
    bFile.envFile = "a";
    bFile.flag = 0;

    EXPECT_EQ(aFile == bFile, true);
}

TEST_F(UT_EnvironmentFile, test_Operator_02)
{
    EnvironmentFile aFile;
    aFile.envFile = "a";
    aFile.flag = 0;

    EnvironmentFile bFile;
    bFile.envFile = "b";
    bFile.flag = 0;

    EXPECT_EQ(aFile == bFile, false);
}

TEST_F(UT_EnvironmentFile, test_Operator_03)
{
    EnvironmentFile aFile;
    aFile.envFile = "a";
    aFile.flag = 0;

    EnvironmentFile bFile;
    bFile.envFile = "a";
    bFile.flag = 1;

    EXPECT_EQ(aFile == bFile, false);
}

TEST_F(UT_EnvironmentFile, test_Operator_04)
{
    EnvironmentFile aFile;
    aFile.envFile = "a";
    aFile.flag = 0;

    EnvironmentFile bFile;
    bFile.envFile = "b";
    bFile.flag = 1;

    EXPECT_EQ(aFile == bFile, false);
}

TEST_F(UT_EnvironmentFile, test_Operator_05)
{
    const EnvironmentFile rhs;
    m_tester->operator=(rhs);
}


TEST_F(UT_EnvironmentFile, test_Operator_06)
{
   QDBusArgument argument;
   const EnvironmentFile file;
   argument << file.envFile << file.flag;
   m_tester->operator==(file);
}


TEST_F(UT_EnvironmentFile, test_Operator_07)
{
    QDataStream stream;
    const EnvironmentFile file;
    stream << file.envFile << file.flag;
    m_tester->operator==(file);
}
