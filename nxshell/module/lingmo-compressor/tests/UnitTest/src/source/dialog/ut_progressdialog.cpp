// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "progressdialog.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>
#include <QDir>


/*******************************函数打桩************************************/

/*******************************单元测试************************************/
// 测试ProgressDialog
class UT_ProgressDialog : public ::testing::Test
{
public:
    UT_ProgressDialog(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new ProgressDialog;
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    ProgressDialog *m_tester;
};

TEST_F(UT_ProgressDialog, initTest)
{

}

TEST_F(UT_ProgressDialog, test_setCurrentTask)
{
    m_tester->setCurrentTask("1.txt");
    EXPECT_EQ(m_tester->m_tasklable->text(), QString("Task:1.txt"));
}

TEST_F(UT_ProgressDialog, test_setCurrentFile)
{
    m_tester->setCurrentFile("1.txt");
    EXPECT_EQ(m_tester->m_filelable->text().contains("Extracting:"), true);
}

TEST_F(UT_ProgressDialog, test_setProcess_001)
{
    m_tester->m_dPerent = 0;
    m_tester->setProcess(0);
    EXPECT_EQ(m_tester->m_dPerent, 0);
}

TEST_F(UT_ProgressDialog, test_setProcess_002)
{
    m_tester->m_dPerent = 0;
    m_tester->setProcess(10);
    EXPECT_EQ(m_tester->m_dPerent, 10);
}

TEST_F(UT_ProgressDialog, test_setFinished)
{
    m_tester->m_dPerent = 100;
    m_tester->setFinished();
    EXPECT_EQ(m_tester->m_dPerent, 0);
}

TEST_F(UT_ProgressDialog, test_clearprocess)
{
    m_tester->m_dPerent = 100;
    m_tester->m_circleprogress->setValue(100);
    m_tester->clearprocess();
    EXPECT_EQ(m_tester->m_circleprogress->value(), 0);
}

TEST_F(UT_ProgressDialog, test_showDialog)
{
    Stub stub;
    CommonStub::stub_QDialog_open(stub);

    m_tester->showDialog();
}

TEST_F(UT_ProgressDialog, test_slotextractpress_001)
{
    Stub stub;
    CommonStub::stub_QDialog_exec(stub, -1);

    m_tester->slotextractpress(1);
}

TEST_F(UT_ProgressDialog, test_slotextractpress_002)
{
    Stub stub;
    CommonStub::stub_QDialog_exec(stub, -1);

    m_tester->slotextractpress(0);
}



// 测试CommentProgressDialog
class UT_CommentProgressDialog : public ::testing::Test
{
public:
    UT_CommentProgressDialog(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CommentProgressDialog;
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CommentProgressDialog *m_tester;
};

TEST_F(UT_CommentProgressDialog, initTest)
{

}

TEST_F(UT_CommentProgressDialog, test_showdialog)
{
    Stub stub;
    CommonStub::stub_QDialog_exec(stub, -1);

    m_tester->showdialog();
}

TEST_F(UT_CommentProgressDialog, test_setProgress)
{
    m_tester->m_progressBar->setValue(0);
    m_tester->setProgress(10);
    EXPECT_EQ(m_tester->m_progressBar->value(), 10);
}

TEST_F(UT_CommentProgressDialog, test_setFinished)
{
    m_tester->m_progressBar->setValue(10);
    m_tester->setFinished();
    EXPECT_EQ(m_tester->m_progressBar->value(), 100);
}
