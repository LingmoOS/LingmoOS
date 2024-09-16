// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "openwithdialog.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>

#include <QCheckBox>
#include <QDir>
#include <QResizeEvent>
#include <QStandardPaths>

/*******************************函数打桩************************************/
/*******************************单元测试************************************/
// 测试OpenWithDialogListItem
class UT_OpenWithDialogListItem : public ::testing::Test
{
public:
    UT_OpenWithDialogListItem(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new OpenWithDialogListItem(QIcon(), "");
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    OpenWithDialogListItem *m_tester;
};

TEST_F(UT_OpenWithDialogListItem, initTest)
{

}

TEST_F(UT_OpenWithDialogListItem, test_setChecked_001)
{
    m_tester->setChecked(true);
    EXPECT_EQ(m_tester->m_pCheckBtn->icon().isNull(), true);
}

TEST_F(UT_OpenWithDialogListItem, test_setChecked_002)
{
    m_tester->setChecked(false);
    EXPECT_EQ(m_tester->m_pCheckBtn->icon().isNull(), true);
}

TEST_F(UT_OpenWithDialogListItem, test_text)
{
    m_tester->m_pTextLbl->setText("123");
    EXPECT_EQ(m_tester->text(), "123");
}

TEST_F(UT_OpenWithDialogListItem, test_resizeEvent)
{
    QResizeEvent *e = new QResizeEvent(QSize(100, 100), QSize(80, 80));
    m_tester->resizeEvent(e);
    delete e;
    qInfo() << m_tester->m_pIconLbl->height();
    EXPECT_EQ(m_tester->m_pIconLbl->height(), 80);
}

TEST_F(UT_OpenWithDialogListItem, test_enterEvent)
{
    QEvent *e = new QEvent(QEvent::Enter);
    m_tester->enterEvent(e);
    delete e;
}

TEST_F(UT_OpenWithDialogListItem, test_leaveEvent)
{
    QEvent *e = new QEvent(QEvent::Leave);
    m_tester->leaveEvent(e);
    delete e;
}


// 测试OpenWithDialogListSparerItem
class UT_OpenWithDialogListSparerItem : public ::testing::Test
{
public:
    UT_OpenWithDialogListSparerItem(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new OpenWithDialogListSparerItem("");
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    OpenWithDialogListSparerItem *m_tester;
};

TEST_F(UT_OpenWithDialogListSparerItem, initTest)
{

}


// 测试OpenWithDialog
class UT_OpenWithDialog : public ::testing::Test
{
public:
    UT_OpenWithDialog(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new OpenWithDialog("");
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    OpenWithDialog *m_tester;
};

TEST_F(UT_OpenWithDialog, initTest)
{

}

TEST_F(UT_OpenWithDialog, test_getOpenStyle)
{
    m_tester->getOpenStyle("1.zip").isEmpty();
}

TEST_F(UT_OpenWithDialog, test_resizeEvent)
{
    QResizeEvent *e = new QResizeEvent(QSize(100, 100), QSize(80, 80));
    m_tester->resizeEvent(e);
    delete e;
    EXPECT_EQ(m_tester->m_titlebar->width(), 100);
}

TEST_F(UT_OpenWithDialog, test_eventFilter)
{
    OpenWithDialogListItem *item = new OpenWithDialogListItem(QIcon(), "", m_tester);

    QMouseEvent *event = new QMouseEvent(QEvent::MouseMove, QPointF(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_tester->eventFilter(item, event);
    delete event;

    event = new QMouseEvent(QEvent::MouseButtonPress, QPointF(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_tester->eventFilter(item, event);
    delete event;
}

TEST_F(UT_OpenWithDialog, test_openWithProgram)
{
    Stub stub;
    CommonStub::stub_ProcessOpenThread_start(stub);
    m_tester->openWithProgram("1.zip");
}

TEST_F(UT_OpenWithDialog, test_showOpenWithDialog_001)
{
    m_tester->m_pCheckedItem = new OpenWithDialogListItem(QIcon(), "", m_tester);
    Stub stub;
    CommonStub::stub_DAbstractDialog_exec(stub, 1);
    CommonStub::stub_ProcessOpenThread_start(stub);
    m_tester->m_bOk = false;
    EXPECT_EQ(m_tester->showOpenWithDialog(OpenWithDialog::ShowType::OpenType).isEmpty(), true);;
}

TEST_F(UT_OpenWithDialog, test_showOpenWithDialog_002)
{
    m_tester->m_pCheckedItem = new OpenWithDialogListItem(QIcon(), "", m_tester);
    Stub stub;
    CommonStub::stub_DAbstractDialog_exec(stub, 1);
    CommonStub::stub_ProcessOpenThread_start(stub);
    m_tester->m_bOk = true;
    m_tester->m_pSetToDefaultCheckBox->setChecked(true);
    EXPECT_EQ(m_tester->showOpenWithDialog(OpenWithDialog::ShowType::OpenType).isEmpty(), true);
}

TEST_F(UT_OpenWithDialog, test_getProgramPathByExec_001)
{
    m_tester->getProgramPathByExec("");
}

TEST_F(UT_OpenWithDialog, test_getProgramPathByExec_002)
{
    EXPECT_EQ(m_tester->getProgramPathByExec("/usr/bin/deepin-compressor"), "/usr/bin/deepin-compressor");
}

TEST_F(UT_OpenWithDialog, test_getProgramPathByExec_003)
{
    EXPECT_EQ(m_tester->getProgramPathByExec("deepin-compressor"), QStandardPaths::findExecutable("/usr/bin/deepin-compressor"));
}

TEST_F(UT_OpenWithDialog, test_checkItem)
{
    m_tester->m_pCheckedItem = new OpenWithDialogListItem(QIcon(), "", m_tester);
    OpenWithDialogListItem *item = new OpenWithDialogListItem(QIcon(), "", m_tester);
    m_tester->checkItem(item);
    EXPECT_EQ(m_tester->m_pCheckedItem, item);
}

TEST_F(UT_OpenWithDialog, testcreateItem)
{
    EXPECT_NE(m_tester->createItem(QIcon(), "", "", ""), nullptr);
}

TEST_F(UT_OpenWithDialog, testslotUseOtherApplication)
{
    QString strPath = _SOURCEDIR;
    strPath += "/src/desktop/deepin-compressor.desktop";
    Stub stub;
    CommonStub::stub_QFileDialog_getOpenFileName(stub, strPath);
    m_tester->slotUseOtherApplication();

    Stub stub1;
    CommonStub::stub_QFileDialog_getOpenFileName(stub1, QStandardPaths::findExecutable("deepin-compressor"));
    m_tester->slotUseOtherApplication();
}

TEST_F(UT_OpenWithDialog, testslotOpenFileByApp)
{
    m_tester->slotOpenFileByApp();
    m_tester->m_pCheckedItem = new OpenWithDialogListItem(QIcon(), "", m_tester);
    m_tester->slotOpenFileByApp();
}
