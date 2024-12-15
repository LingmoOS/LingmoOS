// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HandleMenu.h"
#include "Global.h"
#include "DocSheet.h"
#include "ut_defines.h"
#include "stub.h"

#include <gtest/gtest.h>
#include <QTest>
#include <QSignalSpy>

class TestHandleMenu : public ::testing::Test
{
public:
    TestHandleMenu(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new HandleMenu();
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    HandleMenu *m_tester;
};

TEST_F(TestHandleMenu, initTest)
{

}

TEST_F(TestHandleMenu, testonHandTool)
{
    QString filePath = QCoreApplication::applicationDirPath() + "/" + "files" + "/" + UT_FILE_PDF;
    if (!QFile(filePath).exists() && QFile(":/files/" + QString(UT_FILE_PDF)).exists()) {
        QDir().mkpath(QCoreApplication::applicationDirPath() + "/" + "files");
        QFile(":/files/" + QString(UT_FILE_PDF)).copy(filePath);
    }
    m_tester->m_docSheet = new DocSheet(Dr::FileType::PDF, filePath, m_tester);
    QSignalSpy spy(m_tester->m_docSheet, SIGNAL(sigOperationChanged(DocSheet *)));
    m_tester->onHandTool();
    EXPECT_TRUE(m_tester->m_docSheet->m_operation.mouseShape == Dr::MouseShapeHand);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(TestHandleMenu, testonSelectText)
{
    QString filePath = QCoreApplication::applicationDirPath() + "/" + "files" + "/" + UT_FILE_PDF;
    if (!QFile(filePath).exists() && QFile(":/files/" + QString(UT_FILE_PDF)).exists()) {
        QDir().mkpath(QCoreApplication::applicationDirPath() + "/" + "files");
        QFile(":/files/" + QString(UT_FILE_PDF)).copy(filePath);
    }
    m_tester->m_docSheet = new DocSheet(Dr::FileType::PDF, filePath, m_tester);
    QSignalSpy spy(m_tester->m_docSheet, SIGNAL(sigOperationChanged(DocSheet *)));
    m_tester->onSelectText();
    EXPECT_TRUE(m_tester->m_docSheet->m_operation.mouseShape == Dr::MouseShapeNormal);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(TestHandleMenu, testreadCurDocParam)
{
    QString filePath = QCoreApplication::applicationDirPath() + "/" + "files" + "/" + UT_FILE_PDF;
    if (!QFile(filePath).exists() && QFile(":/files/" + QString(UT_FILE_PDF)).exists()) {
        QDir().mkpath(QCoreApplication::applicationDirPath() + "/" + "files");
        QFile(":/files/" + QString(UT_FILE_PDF)).copy(filePath);
    }
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, filePath, m_tester);
    m_tester->readCurDocParam(sheet);
    EXPECT_TRUE(m_tester->m_docSheet == sheet);
    EXPECT_TRUE(m_tester->m_textAction->isChecked());
    delete sheet;
}
