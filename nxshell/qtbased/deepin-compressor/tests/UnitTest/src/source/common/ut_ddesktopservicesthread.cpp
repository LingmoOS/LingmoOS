// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ddesktopservicesthread.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>
#include <QDir>


/*******************************函数打桩************************************/


/*******************************单元测试************************************/
// 测试DDesktopServicesThread
class UT_DDesktopServicesThread : public ::testing::Test
{
public:
    UT_DDesktopServicesThread(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new DDesktopServicesThread();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    DDesktopServicesThread *m_tester;
};

TEST_F(UT_DDesktopServicesThread, initTest)
{

}

TEST_F(UT_DDesktopServicesThread, test_setOpenFiles)
{
    QStringList listFiles = QStringList() << "1/" << "1.txt";
    m_tester->setOpenFiles(listFiles);
    EXPECT_EQ(m_tester->m_listFiles == listFiles, true);
}

TEST_F(UT_DDesktopServicesThread, test_hasFiles)
{
    QStringList listFiles = QStringList() << "1/" << "1.txt";
    m_tester->setOpenFiles(listFiles);
    EXPECT_EQ(m_tester->hasFiles(), true);
}

TEST_F(UT_DDesktopServicesThread, test_run_001)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_isDir(stub, true);
    DDesktopServicestub::stub_DDesktopServicestub_showFolder(stub);
    m_tester->m_listFiles.clear();
    m_tester->m_listFiles << "1/";
    m_tester->run();
    EXPECT_EQ(m_tester->m_listFiles.isEmpty(), false);
}

TEST_F(UT_DDesktopServicesThread, test_run_002)
{
    Stub stub1;
    DDesktopServicestub::stub_DDesktopServicestub_showFileItem(stub1);
    QFileInfoStub::stub_QFileInfo_isDir(stub1, false);
    m_tester->m_listFiles.clear();
    m_tester->m_listFiles << "1.txt";
    m_tester->run();
    EXPECT_EQ(m_tester->m_listFiles.isEmpty(), false);
}

