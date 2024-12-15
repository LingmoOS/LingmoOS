// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "queries.h"

#include "gtest/src/stub.h"

#include <gtest/gtest.h>

#include <DLabel>

#include <QUrl>

int g_Dialog_exec_result1 = 0;
/*******************************函数打桩************************************/
QUrl qUrl_fromLocalFile_stub(const QString &)
{
    return QUrl("file://file:dsfdsfsdf.txt");
}

int dialog_exec_stub()
{
    return g_Dialog_exec_result1;
}
/*******************************函数打桩************************************/
// 测试CustomDDialog
class UT_CustomDDialog : public ::testing::Test
{
public:
    UT_CustomDDialog(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CustomDDialog;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CustomDDialog *m_tester;
};

TEST_F(UT_CustomDDialog, initTest)
{

}


// 测试OverwriteQuery
class UT_OverwriteQuery : public ::testing::Test
{
public:
    UT_OverwriteQuery(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new OverwriteQuery("");
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    OverwriteQuery *m_tester;
};

TEST_F(UT_OverwriteQuery, initTest)
{

}

TEST_F(UT_OverwriteQuery, test_execute_001)
{
    Stub stub;
    stub.set(ADDR(QUrl, fromLocalFile), qUrl_fromLocalFile_stub);
    typedef int (*fptr)(DDialog *);
    fptr A_foo = (fptr)(&DDialog::exec);   //获取虚函数地址
    stub.set(A_foo, dialog_exec_stub);

    g_Dialog_exec_result1 = -1;
    m_tester->execute();
    EXPECT_EQ(m_tester->m_data.value("response").toInt(), Result_Cancel);
}

TEST_F(UT_OverwriteQuery, test_execute_002)
{
    Stub stub;
    stub.set(ADDR(QUrl, fromLocalFile), qUrl_fromLocalFile_stub);
    typedef int (*fptr)(DDialog *);
    fptr A_foo = (fptr)(&DDialog::exec);   //获取虚函数地址
    stub.set(A_foo, dialog_exec_stub);

    g_Dialog_exec_result1 = 0;
    m_tester->execute();
    EXPECT_EQ(m_tester->m_data.value("response").toInt(), Result_Skip);
}

TEST_F(UT_OverwriteQuery, test_execute_003)
{
    Stub stub;
    stub.set(ADDR(QUrl, fromLocalFile), qUrl_fromLocalFile_stub);
    typedef int (*fptr)(DDialog *);
    fptr A_foo = (fptr)(&DDialog::exec);   //获取虚函数地址
    stub.set(A_foo, dialog_exec_stub);

    g_Dialog_exec_result1 = 1;
    m_tester->execute();
    EXPECT_EQ(m_tester->m_data.value("response").toInt(), Result_Overwrite);
}

TEST_F(UT_OverwriteQuery, test_responseCancelled)
{
    m_tester->m_data[QStringLiteral("response")] = Result_Cancel;
    EXPECT_EQ(m_tester->responseCancelled(), true);
}

TEST_F(UT_OverwriteQuery, test_responseSkip)
{
    m_tester->m_data[QStringLiteral("response")] = Result_Skip;
    EXPECT_EQ(m_tester->responseSkip(), true);
}

TEST_F(UT_OverwriteQuery, test_responseSkipAll)
{
    m_tester->m_data[QStringLiteral("response")] = Result_SkipAll;
    EXPECT_EQ(m_tester->responseSkipAll(), true);
}

TEST_F(UT_OverwriteQuery, test_responseOverwrite)
{
    m_tester->m_data[QStringLiteral("response")] = Result_Overwrite;
    EXPECT_EQ(m_tester->responseOverwrite(), true);
}

TEST_F(UT_OverwriteQuery, test_responseOverwriteAll)
{
    m_tester->m_data[QStringLiteral("response")] = Result_OverwriteAll;
    EXPECT_EQ(m_tester->responseOverwriteAll(), true);
}

TEST_F(UT_OverwriteQuery, test_autoFeed)
{
    DLabel *label1 = new DLabel();
    DLabel *label2 = new DLabel();
    CustomDDialog *dialog = new CustomDDialog();

    m_tester->autoFeed(label1, label2, dialog);

    delete label1;
    delete label2;
    delete dialog;

    EXPECT_EQ(m_tester->m_iLabelOldHeight, 0);
    EXPECT_EQ(m_tester->m_iLabelOld1Height, 0);
    EXPECT_EQ(m_tester->m_iCheckboxOld1Height, 0);
    EXPECT_EQ(m_tester->m_iDialogOldHeight, 100);
}

TEST_F(UT_OverwriteQuery, test_setWidgetColor)
{
    QWidget *pWgt = new QWidget;
    DPalette::ColorRole ct = DPalette::ToolTipText;
    double alphaF = 0.5;
    DPalette palette = DApplicationHelper::instance()->palette(pWgt);
    QColor color = palette.color(ct);

    m_tester->setWidgetColor(pWgt, ct, alphaF);
    EXPECT_EQ(pWgt->palette().color(ct) == color, true);
    delete pWgt;
}

TEST_F(UT_OverwriteQuery, test_setWidgetType)
{
    QWidget *pWgt = new QWidget;
    DPalette::ColorType ct;
    double alphaF = 0.5;
    DPalette palette = DApplicationHelper::instance()->palette(pWgt);
    QColor color = palette.color(ct);

    m_tester->setWidgetType(pWgt, ct, alphaF);
    EXPECT_EQ(DApplicationHelper::instance()->palette(pWgt).color(ct) == color, true);
    delete pWgt;
}


// 测试PasswordNeededQuery
class UT_PasswordNeededQuery : public ::testing::Test
{
public:
    UT_PasswordNeededQuery(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new PasswordNeededQuery("");
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    PasswordNeededQuery *m_tester;
};

TEST_F(UT_PasswordNeededQuery, initTest)
{

}

TEST_F(UT_PasswordNeededQuery, UT_PasswordNeededQuery_execute_001)
{
    g_Dialog_exec_result1 = -1;
    Stub stub;
    typedef int (*fptr)(DDialog *);
    fptr A_foo = (fptr)(&DDialog::exec);   //获取虚函数地址
    stub.set(A_foo, dialog_exec_stub);
    m_tester->execute();
    EXPECT_EQ(m_tester->m_data.value("response").toInt(), Result_Cancel);
}

TEST_F(UT_PasswordNeededQuery, UT_PasswordNeededQuery_execute_002)
{
    g_Dialog_exec_result1 = 1;
    Stub stub;
    typedef int (*fptr)(DDialog *);
    fptr A_foo = (fptr)(&DDialog::exec);   //获取虚函数地址
    stub.set(A_foo, dialog_exec_stub);
    m_tester->execute();
    EXPECT_EQ(m_tester->m_data.value("response").toInt(), Result_Skip);
}

TEST_F(UT_PasswordNeededQuery, UT_PasswordNeededQuery_autoFeed)
{
    DLabel *label1 = new DLabel();
    DLabel *label2 = new DLabel();
    CustomDDialog *dialog = new CustomDDialog();

    m_tester->autoFeed(label1, label2, dialog);

    delete label1;
    delete label2;
    delete dialog;

    EXPECT_EQ(m_tester->m_iLabelOldHeight, 0);
    EXPECT_EQ(m_tester->m_iLabelOld1Height, 0);
    EXPECT_EQ(m_tester->m_iDialogOldHeight, 100);
}

TEST_F(UT_PasswordNeededQuery, UT_PasswordNeededQuery_responseCancelled)
{
    m_tester->m_data[QStringLiteral("response")] = false;
    EXPECT_EQ(m_tester->responseCancelled(), true);
}

TEST_F(UT_PasswordNeededQuery, UT_PasswordNeededQuery_password)
{
    m_tester->m_data[QStringLiteral("password")] = "123";
    EXPECT_EQ(m_tester->password(), "123");
}


// 测试PasswordNeededQuery
class UT_LoadCorruptQuery : public ::testing::Test
{
public:
    UT_LoadCorruptQuery(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new LoadCorruptQuery("");
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    LoadCorruptQuery *m_tester;
};

TEST_F(UT_LoadCorruptQuery, initTest)
{

}

TEST_F(UT_LoadCorruptQuery, UT_LoadCorruptQuery_execute_001)
{
    g_Dialog_exec_result1 = -1;
    Stub stub;
    typedef int (*fptr)(DDialog *);
    fptr A_foo = (fptr)(&DDialog::exec);   //获取虚函数地址
    stub.set(A_foo, dialog_exec_stub);
    m_tester->execute();
    EXPECT_EQ(m_tester->m_data.value("response").toInt(), Result_Cancel);
}

TEST_F(UT_LoadCorruptQuery, UT_LoadCorruptQuery_execute_002)
{
    g_Dialog_exec_result1 = 0;
    Stub stub;
    typedef int (*fptr)(DDialog *);
    fptr A_foo = (fptr)(&DDialog::exec);   //获取虚函数地址
    stub.set(A_foo, dialog_exec_stub);
    m_tester->execute();
    EXPECT_EQ(m_tester->m_data.value("response").toInt(), Result_Readonly);
}

TEST_F(UT_LoadCorruptQuery, UT_LoadCorruptQuery_responseYes)
{
    m_tester->m_data[QStringLiteral("response")] = Result_Readonly;
    EXPECT_EQ(m_tester->responseYes(), true);
}

TEST_F(UT_LoadCorruptQuery, UT_LoadCorruptQuery_autoFeed)
{
    DLabel *label = new DLabel();
    CustomDDialog *dialog = new CustomDDialog();

    m_tester->autoFeed(label, dialog);

    delete label;
    delete dialog;

    EXPECT_EQ(m_tester->m_iLabelOldHeight, 0);
    EXPECT_EQ(m_tester->m_iDialogOldHeight, 100);
}
