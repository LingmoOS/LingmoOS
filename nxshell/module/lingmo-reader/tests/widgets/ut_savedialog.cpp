// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "SaveDialog.h"

#include "stub.h"

#include <DDialog>

#include <gtest/gtest.h>
#include <QTest>

DWIDGET_USE_NAMESPACE

class TestSaveDialog : public ::testing::Test
{
public:
    TestSaveDialog(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new SaveDialog();
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    SaveDialog *m_tester;
};
static QString g_funcname;
int dialog_exec_stub()
{
    g_funcname = __FUNCTION__;
    return 0;
}

TEST_F(TestSaveDialog, initTest)
{

}

TEST_F(TestSaveDialog, testshowExitDialog)
{
    Stub stub;
    typedef int (*fptr)(DDialog *);
    fptr A_foo = (fptr)(&DDialog::exec);   //获取虚函数地址
    stub.set(A_foo, dialog_exec_stub);

    EXPECT_EQ(m_tester->showExitDialog("1.pdf"), 0);
    EXPECT_TRUE(g_funcname == "dialog_exec_stub");
}

TEST_F(TestSaveDialog, testshowTipDialog)
{
    Stub stub;
    typedef int (*fptr)(DDialog *);
    fptr A_foo = (fptr)(&DDialog::exec);   //获取虚函数地址
    stub.set(A_foo, dialog_exec_stub);

    EXPECT_EQ(m_tester->showTipDialog("1.pdf"), 0);
    EXPECT_TRUE(g_funcname == "dialog_exec_stub");
}
