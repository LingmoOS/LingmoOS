// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "progresspage.h"
#include "ut_commonstub.h"
#include "customwidget.h"

#include "gtest/src/stub.h"

#include <gtest/gtest.h>
#include <QTest>

class UT_ProgressPage : public ::testing::Test
{
public:
    UT_ProgressPage(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new ProgressPage;
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    ProgressPage *m_tester;
};

TEST_F(UT_ProgressPage, initTest)
{

}

TEST_F(UT_ProgressPage, test_setProgressType_001)
{
    m_tester->setProgressType(PT_Compress);
    EXPECT_EQ(m_tester->m_pSpeedLbl->text(), (QObject::tr("Speed", "compress") + ": " + QObject::tr("Calculating...")));
}

TEST_F(UT_ProgressPage, test_setProgressType_002)
{
    m_tester->setProgressType(PT_Delete);
    EXPECT_EQ(m_tester->m_pSpeedLbl->text(), (QObject::tr("Speed", "delete") + ": " + QObject::tr("Calculating...")));
}

TEST_F(UT_ProgressPage, test_setProgressType_003)
{
    m_tester->setProgressType(PT_Convert);
    EXPECT_EQ(m_tester->m_pSpeedLbl->text(), (QObject::tr("Speed", "convert") + ": " + QObject::tr("Calculating...")));
}

TEST_F(UT_ProgressPage, test_setProgressType_004)
{
    m_tester->setProgressType(PT_Comment);
    EXPECT_EQ(m_tester->m_pSpeedLbl->text(), "");
}

TEST_F(UT_ProgressPage, test_setProgressType_005)
{
    m_tester->setProgressType(PT_UnCompress);
    EXPECT_EQ(m_tester->m_pSpeedLbl->text(), (QObject::tr("Speed", "uncompress") + ": " + QObject::tr("Calculating...")));
}

TEST_F(UT_ProgressPage, test_setTotalSize)
{
    m_tester->m_qTotalSize = 0;
    m_tester->setTotalSize(1);
    EXPECT_EQ(m_tester->m_qTotalSize, 1);
}

TEST_F(UT_ProgressPage, test_setArchiveName)
{
    m_tester->m_strArchiveName.clear();
    m_tester->setArchiveName("123");
    EXPECT_EQ(m_tester->m_strArchiveName, "123");
}

TEST_F(UT_ProgressPage, test_archiveName)
{
    m_tester->m_strArchiveName = "123";
    EXPECT_EQ(m_tester->archiveName(), "123");
}

TEST_F(UT_ProgressPage, test_setProgress_001)
{
    Stub stub;
    QElapsedTimerStub::stub_QElapsedTimer_start(stub);
    QElapsedTimerStub::stub_QElapsedTimer_restart(stub);
    m_tester->m_iPerent = 50;
    m_tester->setProgress(40);
    EXPECT_EQ(m_tester->m_iPerent, 50);
}

TEST_F(UT_ProgressPage, test_setProgress_002)
{
    Stub stub;
    QElapsedTimerStub::stub_QElapsedTimer_start(stub);
    QElapsedTimerStub::stub_QElapsedTimer_restart(stub);
    QElapsedTimerStub::stub_QElapsedTimer_elapsed(stub, 1);
    m_tester->m_iPerent = 50;
    m_tester->setProgress(60);
    EXPECT_EQ(m_tester->m_iPerent, 60);
}

TEST_F(UT_ProgressPage, test_setCurrentFileName_001)
{
    m_tester->m_eType = PT_Compress;
    m_tester->setCurrentFileName("11");
    EXPECT_EQ(m_tester->m_pFileNameLbl->text().startsWith("Compressing"), true);
}

TEST_F(UT_ProgressPage, test_setCurrentFileName_002)
{
    m_tester->m_eType = PT_Delete;
    m_tester->setCurrentFileName("11");
    EXPECT_EQ(m_tester->m_pFileNameLbl->text().startsWith("Deleting"), true);
}

TEST_F(UT_ProgressPage, ttest_setCurrentFileName_003)
{
    m_tester->m_eType = PT_Convert;
    m_tester->setCurrentFileName("11");
    EXPECT_EQ(m_tester->m_pFileNameLbl->text().startsWith("Converting"), true);
}

TEST_F(UT_ProgressPage, test_setCurrentFileName_004)
{
    m_tester->m_eType = PT_Comment;
    m_tester->setCurrentFileName("11");
    EXPECT_EQ(m_tester->m_pFileNameLbl->text().startsWith("Updating the comment"), true);
}

TEST_F(UT_ProgressPage, test_setCurrentFileName_005)
{
    m_tester->m_eType = PT_None;
    m_tester->setCurrentFileName("11");
    EXPECT_EQ(m_tester->m_pFileNameLbl->text().startsWith("Extracting"), true);
}

TEST_F(UT_ProgressPage, test_resetProgress_001)
{
    Stub stub;
    QElapsedTimerStub::stub_QElapsedTimer_elapsed(stub, 1);
    m_tester->m_eType = PT_Comment;
    m_tester->resetProgress();
    EXPECT_EQ(m_tester->m_qConsumeTime, 0);
}

TEST_F(UT_ProgressPage, test_resetProgress_002)
{
    Stub stub;
    QElapsedTimerStub::stub_QElapsedTimer_elapsed(stub, 1);
    m_tester->m_eType = PT_None;
    m_tester->resetProgress();
    EXPECT_EQ(m_tester->m_qConsumeTime, 0);
}

TEST_F(UT_ProgressPage, test_restartTimer)
{
    Stub stub;
    QElapsedTimerStub::stub_QElapsedTimer_restart(stub);
    m_tester->restartTimer();
}

TEST_F(UT_ProgressPage, test_setPushButtonCheckable)
{
    m_tester->setPushButtonCheckable(true, true);
    EXPECT_EQ(m_tester->m_pCancelBtn->isEnabled(), true);
    EXPECT_EQ(m_tester->m_pPauseContinueButton->isEnabled(), true);
}

TEST_F(UT_ProgressPage, test_calSpeedAndRemainingTime_001)
{
    Stub stub;
    QElapsedTimerStub::stub_QElapsedTimer_start(stub);
    QElapsedTimerStub::stub_QElapsedTimer_restart(stub);
    QElapsedTimerStub::stub_QElapsedTimer_elapsed(stub, 0);

    m_tester->m_qConsumeTime = -1;
    m_tester->m_qTotalSize = 0;
    double dSpeed;
    qint64 qRemainingTime;
    m_tester->calSpeedAndRemainingTime(dSpeed, qRemainingTime);
    EXPECT_EQ(dSpeed, 0);
}

TEST_F(UT_ProgressPage, test_calSpeedAndRemainingTime_002)
{
    Stub stub;
    QElapsedTimerStub::stub_QElapsedTimer_start(stub);
    QElapsedTimerStub::stub_QElapsedTimer_restart(stub);
    QElapsedTimerStub::stub_QElapsedTimer_elapsed(stub, 0);

    double dSpeed;
    qint64 qRemainingTime;
    m_tester->calSpeedAndRemainingTime(dSpeed, qRemainingTime);
    EXPECT_EQ(dSpeed, 0);
}

TEST_F(UT_ProgressPage, test_calSpeedAndRemainingTime_003)
{
    Stub stub;
    QElapsedTimerStub::stub_QElapsedTimer_start(stub);
    QElapsedTimerStub::stub_QElapsedTimer_restart(stub);
    QElapsedTimerStub::stub_QElapsedTimer_elapsed(stub, 10);

    m_tester->m_qConsumeTime = 0;
    m_tester->m_qTotalSize = 1024;
    m_tester->m_iPerent = 100;
    m_tester->m_eType = PT_Convert;
    double dSpeed;
    qint64 qRemainingTime;
    m_tester->calSpeedAndRemainingTime(dSpeed, qRemainingTime);
    EXPECT_EQ(dSpeed, 200);
}

TEST_F(UT_ProgressPage, test_calSpeedAndRemainingTime_004)
{
    Stub stub;
    QElapsedTimerStub::stub_QElapsedTimer_start(stub);
    QElapsedTimerStub::stub_QElapsedTimer_restart(stub);
    QElapsedTimerStub::stub_QElapsedTimer_elapsed(stub, 10);

    m_tester->m_qConsumeTime = 0;
    m_tester->m_qTotalSize = 1024;
    m_tester->m_iPerent = 100;
    m_tester->m_eType = PT_Compress;
    double dSpeed;
    qint64 qRemainingTime;
    m_tester->calSpeedAndRemainingTime(dSpeed, qRemainingTime);
    EXPECT_EQ(dSpeed, 100);
}

TEST_F(UT_ProgressPage, test_displaySpeedAndTime_001)
{
    m_tester->m_eType = PT_Compress;
    double dSpeed = 1000;
    qint64 qRemainingTime = 10;
    m_tester->displaySpeedAndTime(dSpeed, qRemainingTime);
    EXPECT_EQ(m_tester->m_pSpeedLbl->text(), QObject::tr("Speed", "compress") + ": " + QString::number(dSpeed, 'f', 2) + "KB/s");
}

TEST_F(UT_ProgressPage, test_displaySpeedAndTime_002)
{
    m_tester->m_eType = PT_Compress;
    double dSpeed = 1024 * 10;
    qint64 qRemainingTime = 10;
    m_tester->displaySpeedAndTime(dSpeed, qRemainingTime);
    EXPECT_EQ(m_tester->m_pSpeedLbl->text(), QObject::tr("Speed", "compress") + ": " + QString::number((dSpeed / 1024), 'f', 2) + "MB/s");
}

TEST_F(UT_ProgressPage, test_displaySpeedAndTime_003)
{
    m_tester->m_eType = PT_Compress;
    double dSpeed = 1024 * 400;
    qint64 qRemainingTime = 10;
    m_tester->displaySpeedAndTime(dSpeed, qRemainingTime);
    EXPECT_EQ(m_tester->m_pSpeedLbl->text(), QObject::tr("Speed", "compress") + ": " + ">300MB/s");
}

TEST_F(UT_ProgressPage, test_displaySpeedAndTime_004)
{
    m_tester->m_eType = PT_Delete;
    double dSpeed = 20;
    qint64 qRemainingTime = 10;
    m_tester->displaySpeedAndTime(dSpeed, qRemainingTime);
    EXPECT_EQ(m_tester->m_pSpeedLbl->text(), QObject::tr("Speed", "delete") + ": " + QString::number(dSpeed, 'f', 2) + "KB/s");
}

TEST_F(UT_ProgressPage, test_displaySpeedAndTime_005)
{
    m_tester->m_eType = PT_Delete;
    double dSpeed = 2048;
    qint64 qRemainingTime = 10;
    m_tester->displaySpeedAndTime(dSpeed, qRemainingTime);
    EXPECT_EQ(m_tester->m_pSpeedLbl->text(), QObject::tr("Speed", "delete") + ": " + QString::number((dSpeed / 1024), 'f', 2) + "MB/s");
}

TEST_F(UT_ProgressPage, test_displaySpeedAndTime_006)
{
    m_tester->m_eType = PT_UnCompress;
    double dSpeed = 20;
    qint64 qRemainingTime = 10;
    m_tester->displaySpeedAndTime(dSpeed, qRemainingTime);
    EXPECT_EQ(m_tester->m_pSpeedLbl->text(), QObject::tr("Speed", "uncompress") + ": " + QString::number(dSpeed, 'f', 2) + "KB/s");
}

TEST_F(UT_ProgressPage, test_displaySpeedAndTime_007)
{
    m_tester->m_eType = PT_UnCompress;
    double dSpeed = 1024 * 20;
    qint64 qRemainingTime = 10;
    m_tester->displaySpeedAndTime(dSpeed, qRemainingTime);
    EXPECT_EQ(m_tester->m_pSpeedLbl->text(), QObject::tr("Speed", "uncompress") + ": " + QString::number((dSpeed / 1024), 'f', 2) + "MB/s");
}

TEST_F(UT_ProgressPage, test_displaySpeedAndTime_008)
{
    m_tester->m_eType = PT_UnCompress;
    double dSpeed = 1024 * 400;
    qint64 qRemainingTime = 10;
    m_tester->displaySpeedAndTime(dSpeed, qRemainingTime);
    EXPECT_EQ(m_tester->m_pSpeedLbl->text(), QObject::tr("Speed", "uncompress") + ": " + ">300MB/s");
}

TEST_F(UT_ProgressPage, test_displaySpeedAndTime_009)
{
    m_tester->m_eType = PT_Convert;
    double dSpeed = 20;
    qint64 qRemainingTime = 10;
    m_tester->displaySpeedAndTime(dSpeed, qRemainingTime);
    EXPECT_EQ(m_tester->m_pSpeedLbl->text(), QObject::tr("Speed", "convert") + ": " + QString::number(dSpeed, 'f', 2) + "KB/s");
}

TEST_F(UT_ProgressPage, test_displaySpeedAndTime_010)
{
    m_tester->m_eType = PT_Convert;
    double dSpeed = 1024 * 20;
    qint64 qRemainingTime = 10;
    m_tester->displaySpeedAndTime(dSpeed, qRemainingTime);
    EXPECT_EQ(m_tester->m_pSpeedLbl->text(), QObject::tr("Speed", "convert") + ": " + QString::number((dSpeed / 1024), 'f', 2) + "MB/s");
}

TEST_F(UT_ProgressPage, test_displaySpeedAndTime_011)
{
    m_tester->m_eType = PT_Convert;
    double dSpeed = 1024 * 400;
    qint64 qRemainingTime = 10;
    m_tester->displaySpeedAndTime(dSpeed, qRemainingTime);
    EXPECT_EQ(m_tester->m_pSpeedLbl->text(), QObject::tr("Speed", "uncompress") + ": " + ">300MB/s");
}

TEST_F(UT_ProgressPage, test_displaySpeedAndTime_012)
{
    m_tester->m_eType = PT_Comment;
    double dSpeed = 20;
    qint64 qRemainingTime = 10;
    m_tester->displaySpeedAndTime(dSpeed, qRemainingTime);
    EXPECT_EQ(m_tester->m_pSpeedLbl->text(), "");
}

TEST_F(UT_ProgressPage, test_slotPauseClicked_001)
{
    m_tester->slotPauseClicked(true);
    EXPECT_EQ(m_tester->m_pPauseContinueButton->text(), "Continue");
}

TEST_F(UT_ProgressPage, test_slotPauseClicked_002)
{
    m_tester->slotPauseClicked(false);
    EXPECT_EQ(m_tester->m_pPauseContinueButton->text(), "Pause");
}

TEST_F(UT_ProgressPage, test_slotCancelClicked_001)
{
    Stub stub;
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 1);
    m_tester->m_pPauseContinueButton->setChecked(false);
    m_tester->m_eType = PT_Compress;
    m_tester->slotCancelClicked();
}

TEST_F(UT_ProgressPage, test_slotCancelClicked_002)
{
    Stub stub;
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 0);
    m_tester->m_pPauseContinueButton->setChecked(false);
    m_tester->m_eType = PT_UnCompress;
    m_tester->slotCancelClicked();
}

TEST_F(UT_ProgressPage, test_slotCancelClicked_003)
{
    Stub stub;
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 0);
    m_tester->m_pPauseContinueButton->setChecked(false);
    m_tester->m_eType = PT_Delete;
    m_tester->slotCancelClicked();
}

TEST_F(UT_ProgressPage, test_slotCancelClicked_004)
{
    Stub stub;
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 0);
    m_tester->m_pPauseContinueButton->setChecked(false);
    m_tester->m_eType = PT_CompressAdd;
    m_tester->slotCancelClicked();
}

TEST_F(UT_ProgressPage, test_slotCancelClicked_005)
{
    Stub stub;
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 0);
    m_tester->m_pPauseContinueButton->setChecked(false);
    m_tester->m_eType = PT_Convert;
    m_tester->slotCancelClicked();
}
