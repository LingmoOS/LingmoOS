// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ScaleMenu.h"
#include "Global.h"
#include "DocSheet.h"
#include "ut_defines.h"
#include "stub.h"

#include <gtest/gtest.h>
#include <QTest>
#include <QList>

class TestScaleMenu : public ::testing::Test
{
public:
    TestScaleMenu(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        QString filePath = QCoreApplication::applicationDirPath() + "/" + "files" + "/" + UT_FILE_PDF;
        if (!QFile(filePath).exists() && QFile(":/files/" + QString(UT_FILE_PDF)).exists()) {
            QDir().mkpath(QCoreApplication::applicationDirPath() + "/" + "files");
            QFile(":/files/" + QString(UT_FILE_PDF)).copy(filePath);
        }

        m_tester = new ScaleMenu();
        m_tester->readCurDocParam(new DocSheet(Dr::FileType::PDF, filePath, m_tester));
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    ScaleMenu *m_tester;
};

int indexOf_stub(void *, const QAction &, int)
{
    return 1;
}
QList<qreal> scaleFactorList_stub()
{
    QList<qreal> factorList = {0.1, 0.25, 0.5, 0.75, 1, 1.25, 1.5, 1.75, 2, 3, 4, 5};
    return  factorList;
}

TEST_F(TestScaleMenu, initTest)
{

}

TEST_F(TestScaleMenu, testonTwoPage)
{
    m_tester->m_sheet->m_operation.layoutMode = Dr::TwoPagesMode;
    m_tester->m_sheet->m_operation.scaleMode = Dr::FitToPageDefaultMode;
    m_tester->onTwoPage();
    EXPECT_TRUE(m_tester->m_sheet->m_operation.layoutMode == Dr::SinglePageMode);
    EXPECT_TRUE(m_tester->m_sheet->m_operation.scaleMode == Dr::FitToPageWidthMode);
}

TEST_F(TestScaleMenu, testonFiteH)
{
    m_tester->m_sheet->m_operation.scaleMode = Dr::FitToPageDefaultMode;
    m_tester->onFiteH();
    EXPECT_TRUE(m_tester->m_sheet->m_operation.scaleMode == Dr::FitToPageHeightMode);
}

TEST_F(TestScaleMenu, testonFiteW)
{
    m_tester->m_sheet->m_operation.scaleMode = Dr::FitToPageDefaultMode;
    m_tester->onFiteW();
    EXPECT_TRUE(m_tester->m_sheet->m_operation.scaleMode == Dr::FitToPageWidthMode);
}

TEST_F(TestScaleMenu, testonDefaultPage)
{
    m_tester->m_sheet->m_operation.scaleMode = Dr::FitToPageWidthMode;
    m_tester->onDefaultPage();
    EXPECT_TRUE(m_tester->m_sheet->m_operation.scaleMode == Dr::FitToPageDefaultMode);
}

TEST_F(TestScaleMenu, testonFitPage)
{
    m_tester->m_sheet->m_operation.scaleMode = Dr::FitToPageDefaultMode;
    m_tester->onFitPage();
    EXPECT_TRUE(m_tester->m_sheet->m_operation.scaleMode == Dr::FitToPageWorHMode);
}

TEST_F(TestScaleMenu, testonScaleFactor)
{
    Stub s;
    s.set(ADDR(QList<QAction *>, indexOf), indexOf_stub);
    s.set(ADDR(DocSheet, scaleFactorList), scaleFactorList_stub);
    m_tester->m_sheet->m_operation.scaleMode = Dr::FitToPageWidthMode;
    m_tester->onScaleFactor();
    EXPECT_TRUE(m_tester->m_sheet->m_operation.scaleMode == Dr::ScaleFactorMode);
}
