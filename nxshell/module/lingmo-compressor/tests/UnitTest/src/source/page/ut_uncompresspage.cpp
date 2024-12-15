// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "uncompresspage.h"
#include "uistruct.h"
#include "uncompressview.h"
#include "customwidget.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"

#include <gtest/gtest.h>
#include <QTest>


/*******************************函数打桩************************************/
// 对UnCompressView的addNewFiles进行打桩
void unCompressView_refreshDataByCurrentPathChanged_stub()
{
    return;
}

// 对UnCompressView的addNewFiles进行打桩
void unCompressView_addNewFiles_stub(const QStringList &listFiles)
{
    Q_UNUSED(listFiles)
    return;
}

// 对UnCompressView的addNewFiles进行打桩
QString unCompressView_getCurPath_stub()
{
    return "123";
}

// 对UnCompressView的width进行打桩
int unCompressPage_width_stub()
{
    return 50;
}

// 对QFontMetrics的width进行打桩
int qFontMetrics_width_stub(const QString &, int)
{
    return 120;
}
/*******************************函数打桩************************************/

class UT_UnCompressPage : public ::testing::Test
{
public:
    UT_UnCompressPage(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new UnCompressPage;
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    UnCompressPage *m_tester;
};

TEST_F(UT_UnCompressPage, initTest)
{

}

TEST_F(UT_UnCompressPage, test_setArchiveFullPath_001)
{
    UnCompressParameter stUnCompressParameter;
    stUnCompressParameter.eSplitVolume = UnCompressParameter::ST_No;
    stUnCompressParameter.bModifiable = true;
    m_tester->setArchiveFullPath("1.zip", stUnCompressParameter);
    EXPECT_EQ(m_tester->m_strArchiveFullPath, "1.zip");
    EXPECT_EQ(m_tester->getUnCompressView()->isModifiable(), true);
}

TEST_F(UT_UnCompressPage, test_setArchiveFullPath_002)
{
    UnCompressParameter stUnCompressParameter;
    stUnCompressParameter.eSplitVolume = UnCompressParameter::ST_Zip;
    stUnCompressParameter.bModifiable = false;
    m_tester->setArchiveFullPath("1.zip.001", stUnCompressParameter);
    EXPECT_EQ(m_tester->m_strArchiveFullPath, "1.zip.001");
    EXPECT_EQ(m_tester->getUnCompressView()->isModifiable(), false);
}

TEST_F(UT_UnCompressPage, test_archiveFullPath)
{
    UnCompressParameter stUnCompressParameter;
    m_tester->setArchiveFullPath("1.zip.001", stUnCompressParameter);
    EXPECT_EQ(m_tester->archiveFullPath(), "1.zip.001");
}

TEST_F(UT_UnCompressPage, test_setDefaultUncompressPath)
{
    m_tester->setDefaultUncompressPath("/home/Desktop");
    EXPECT_EQ(m_tester->m_strUnCompressPath, "/home/Desktop");
    EXPECT_EQ(m_tester->m_pUncompressPathBtn->toolTip(), "/home/Desktop");
    EXPECT_EQ(m_tester->m_pUncompressPathBtn->text().contains("Extract to:"), true);
}

TEST_F(UT_UnCompressPage, test_setrefreshArchiveData)
{
    m_tester->refreshArchiveData();
    EXPECT_EQ(m_tester->m_pUnCompressView->m_iLevel, 0);
}

TEST_F(UT_UnCompressPage, test_refreshDataByCurrentPathChanged)
{
    Stub stub;
    stub.set(ADDR(UnCompressView, refreshDataByCurrentPathChanged), unCompressView_refreshDataByCurrentPathChanged_stub);
    m_tester->refreshDataByCurrentPathChanged();
    EXPECT_NE(m_tester->m_pUnCompressView, nullptr);
}

TEST_F(UT_UnCompressPage, test_addNewFiles)
{
    Stub stub;
    stub.set(ADDR(UnCompressView, addNewFiles), unCompressView_addNewFiles_stub);
    m_tester->addNewFiles(QStringList());
    EXPECT_NE(m_tester->m_pUnCompressView, nullptr);
}

TEST_F(UT_UnCompressPage, test_getCurPath)
{
    Stub stub;
    stub.set(ADDR(UnCompressView, getCurPath), unCompressView_getCurPath_stub);
    EXPECT_EQ(m_tester->getCurPath(), "123");
}

TEST_F(UT_UnCompressPage, test_clear)
{
    m_tester->clear();
    EXPECT_EQ(m_tester->m_pUnCompressView->m_mapShowEntry.isEmpty(), true);
    EXPECT_EQ(m_tester->m_pUnCompressView->m_strUnCompressPath, "");
    EXPECT_EQ(m_tester->m_pUnCompressView->m_bModifiable, false);
    EXPECT_EQ(m_tester->m_pUnCompressView->m_bMultiplePassword, false);
}

TEST_F(UT_UnCompressPage, test_getUnCompressView)
{
    EXPECT_EQ(m_tester->getUnCompressView(), m_tester->m_pUnCompressView);
}

TEST_F(UT_UnCompressPage, test_getUnCompressBtn)
{
    EXPECT_EQ(m_tester->getUnCompressBtn(), m_tester->m_pUnCompressBtn);
}

TEST_F(UT_UnCompressPage, test_getUncompressPathBtn)
{
    EXPECT_EQ(m_tester->getUncompressPathBtn(), m_tester->m_pUncompressPathBtn);
}

QString elidedText_stub(const QString &, Qt::TextElideMode, int, int)
{
    return "123…456";
}

TEST_F(UT_UnCompressPage, test_elidedExtractPath)
{
    Stub stub;
    stub.set(ADDR(UnCompressPage, width), unCompressPage_width_stub);
    stub.set((int(QFontMetrics::*)(const QString &, int)const)ADDR(QFontMetrics, width), qFontMetrics_width_stub);
    stub.set(ADDR(QFontMetrics, elidedText), elidedText_stub);
    QString str = "12345678912";
    EXPECT_EQ(m_tester->elidedExtractPath(str).isEmpty(), false);
}

TEST_F(UT_UnCompressPage, test_slotUncompressClicked_001)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_isWritable(stub, false);
    QFileInfoStub::stub_QFileInfo_isExecutable(stub, true);
    QFileInfoStub::stub_QFileInfo_exists(stub, false);
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    m_tester->slotUncompressClicked();
}

TEST_F(UT_UnCompressPage, test_slotUncompressClicked_002)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_isWritable(stub, false);
    QFileInfoStub::stub_QFileInfo_isExecutable(stub, true);
    QFileInfoStub::stub_QFileInfo_exists(stub, true);
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    m_tester->slotUncompressClicked();
}

TEST_F(UT_UnCompressPage, test_slotUncompressClicked_003)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_isWritable(stub, true);
    QFileInfoStub::stub_QFileInfo_isExecutable(stub, true);
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    m_tester->slotUncompressClicked();
}

TEST_F(UT_UnCompressPage, test_slotUnCompressPathClicked_001)
{
    QList<QUrl> listUrl;
    listUrl << QUrl("/home/Desktop");
    Stub stub;
    DFileDialogStub::stub_DFileDialog_exec(stub, 0);
    DFileDialogStub::stub_DFileDialog_selectedUrls(stub, listUrl);
    m_tester->slotUnCompressPathClicked();
}

TEST_F(UT_UnCompressPage, test_slotUnCompressPathClicked_002)
{
    QList<QUrl> listUrl;
    listUrl << QUrl("/home/Desktop");
    Stub stub;
    DFileDialogStub::stub_DFileDialog_exec(stub, 1);
    DFileDialogStub::stub_DFileDialog_selectedUrls(stub, listUrl);
    m_tester->slotUnCompressPathClicked();
}

TEST_F(UT_UnCompressPage, test_slotFileChoose)
{
    m_tester->getUnCompressView()->m_bModifiable = true;
    m_tester->slotFileChoose();
    EXPECT_EQ(m_tester->m_pUnCompressView->isModifiable(), true);
}
