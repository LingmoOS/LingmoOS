// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"
#include "uitools.h"
#include "compresspage.h"
#include "uncompresspage.h"
#include "archivejob.h"
#include "commonstruct.h"
#include "progressdialog.h"
#include "progresspage.h"
#include "queries.h"
#include "settingdialog.h"
#include "datamanager.h"
#include "successpage.h"
#include "failurepage.h"
#include "compresssettingpage.h"
#include "ddesktopservicesthread.h"
#include "openFileWatcher.h"
#include "calculatesizethread.h"
#include "uncompressview.h"
#include "compressview.h"
#include "treeheaderview.h"
#include "progressdialog.h"
#include "progresspage.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"

#include <DWindowCloseButton>

#include <QTest>
#include <QAction>
#include <QSettings>
#include <QStackedWidget>
#include <DTitlebar>

#include <gtest/gtest.h>

DWIDGET_USE_NAMESPACE

/*******************************函数打桩************************************/
bool handleArguments_Open_stub(const QStringList &)
{
    return false;
}

bool handleArguments_RightMenu_stub(const QStringList &)
{
    return false;
}

bool handleArguments_Append_stub(const QStringList &)
{
    return false;
}

void slotChoosefiles_stub()
{
    return;
}

void loadArchive_stub(const QString &)
{
    return;
}

void compressPage_addCompressFiles_stub(const QStringList &)
{
    return;
}

QStringList compressPage_compressFiles_stub()
{
    return QStringList() << "1.txt";
}

void UnCompressPage_addNewFiles_stub(const QStringList &)
{
    return;
}

void handleJobNormalFinished_stub(ArchiveJob::JobType)
{
    return ;
}

void handleJobCancelFinished_stub(ArchiveJob::JobType)
{
    return ;
}

void handleJobErrorFinished_stub(ArchiveJob::JobType, ErrorType)
{
    return ;
}

void progressdialog_showDialog_stub()
{
    return ;
}

bool settingDialog_isAutoOpen_stub()
{
    return true;
}

bool settingDialog_isAutoCreatDir_stub()
{
    return true;
}

bool settingDialog_isAutoDeleteFile_stub()
{
    return true;
}

bool calculateSizeThread_isRunning_stub()
{
    return true;
}

bool calculateSizeThread_wait_stub()
{
    return true;
}
/*******************************函数打桩************************************/


class UT_MainWindow : public ::testing::Test
{
public:
    UT_MainWindow(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new MainWindow;
        m_tester->initUI();
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
//        if (nullptr != m_tester->m_mywork) {
//            m_tester->m_mywork->deleteLater();
//            m_tester->m_mywork = nullptr;
//        }
        delete m_tester;
    }

protected:
    MainWindow *m_tester;
};

TEST_F(UT_MainWindow, initTest)
{

}

TEST_F(UT_MainWindow, test_MainWindow_destruct)
{
    Stub stub;
    CommonStub::stub_QThread_isRunning(stub, true);
    CommonStub::stub_QThread_wait(stub, true);

    QStringList listfiles;
    QString strArchiveFullPath;
    QList<FileEntry> listAddEntry;
    CompressOptions stOptions;
    m_tester->m_mywork = new CalculateSizeThread(listfiles, strArchiveFullPath, listAddEntry, stOptions);
    EXPECT_NE(m_tester->m_mywork, nullptr);
}

TEST_F(UT_MainWindow, test_checkHerePath_001)
{
    Stub stub;
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    QFileInfoStub::stub_QFileInfo_isWritable(stub, false);
    QFileInfoStub::stub_QFileInfo_isExecutable(stub, false);
    EXPECT_EQ(m_tester->checkHerePath("/home/Desktop"), false);
}

TEST_F(UT_MainWindow, test_checkHerePath_002)
{
    Stub stub;
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    QFileInfoStub::stub_QFileInfo_isWritable(stub, true);
    QFileInfoStub::stub_QFileInfo_isExecutable(stub, true);
    EXPECT_EQ(m_tester->checkHerePath("/home/Desktop"), true);
}

TEST_F(UT_MainWindow, test_refreshPage_001)
{
    m_tester->m_ePageID = PI_Home;
    m_tester->refreshPage();
    EXPECT_EQ(m_tester->m_pMainWidget->currentIndex(), 0);
}

TEST_F(UT_MainWindow, test_refreshPage_002)
{
    m_tester->m_ePageID = PI_Compress;
    m_tester->refreshPage();
    EXPECT_EQ(m_tester->m_pMainWidget->currentIndex(), 1);
}

TEST_F(UT_MainWindow, test_refreshPage_003)
{
    m_tester->m_ePageID = PI_CompressSetting;
    m_tester->refreshPage();
    EXPECT_EQ(m_tester->m_pMainWidget->currentIndex(), 2);
}

TEST_F(UT_MainWindow, test_refreshPage_004)
{
    m_tester->m_ePageID = PI_UnCompress;
    m_tester->refreshPage();
    EXPECT_EQ(m_tester->m_pMainWidget->currentIndex(), 3);
}

TEST_F(UT_MainWindow, test_refreshPage_005)
{
    m_tester->m_ePageID = PI_AddCompressProgress;
    m_tester->refreshPage();
    EXPECT_EQ(m_tester->m_pMainWidget->currentIndex(), 4);
}

TEST_F(UT_MainWindow, test_refreshPage_006)
{
    m_tester->m_ePageID = PI_CompressProgress;
    m_tester->refreshPage();
    EXPECT_EQ(m_tester->m_pMainWidget->currentIndex(), 4);
}

TEST_F(UT_MainWindow, test_refreshPage_007)
{
    m_tester->m_ePageID = PI_UnCompressProgress;
    m_tester->refreshPage();
    EXPECT_EQ(m_tester->m_pMainWidget->currentIndex(), 4);
}

TEST_F(UT_MainWindow, test_refreshPage_008)
{
    m_tester->m_ePageID = PI_DeleteProgress;
    m_tester->refreshPage();
    EXPECT_EQ(m_tester->m_pMainWidget->currentIndex(), 4);
}

TEST_F(UT_MainWindow, test_refreshPage_009)
{
    m_tester->m_ePageID = PI_ConvertProgress;
    m_tester->refreshPage();
    EXPECT_EQ(m_tester->m_pMainWidget->currentIndex(), 4);
}

TEST_F(UT_MainWindow, test_refreshPage_010)
{
    m_tester->m_ePageID = PI_CommentProgress;
    m_tester->refreshPage();
    EXPECT_EQ(m_tester->m_pMainWidget->currentIndex(), 4);
}

TEST_F(UT_MainWindow, test_refreshPage_012)
{
    m_tester->m_ePageID = PI_Success;
    m_tester->refreshPage();
    EXPECT_EQ(m_tester->m_pMainWidget->currentIndex(), 5);
}

TEST_F(UT_MainWindow, test_refreshPage_013)
{
    m_tester->m_ePageID = PI_Failure;
    m_tester->refreshPage();
    EXPECT_EQ(m_tester->m_pMainWidget->currentIndex(), 6);
}

TEST_F(UT_MainWindow, test_refreshPage_014)
{
    m_tester->m_ePageID = PI_Loading;
    m_tester->refreshPage();
    EXPECT_EQ(m_tester->m_pMainWidget->currentIndex(), 7);
}

TEST_F(UT_MainWindow, test_setTitleButtonStyle_001)
{
    m_tester->setTitleButtonStyle(true, true, DStyle::SP_IncreaseElement);
    EXPECT_EQ(m_tester->m_pTitleWidget->m_pTitleButton->toolTip(), "Open file");
}

TEST_F(UT_MainWindow, test_setTitleButtonStyle_002)
{
    m_tester->setTitleButtonStyle(true, true, DStyle::SP_ArrowLeave);
    EXPECT_EQ(m_tester->m_pTitleWidget->m_pTitleButton->toolTip(), "Back");
}

TEST_F(UT_MainWindow, test_setTitleButtonStyle_003)
{
    m_tester->setTitleButtonStyle(true, false, DStyle::SP_EditElement);
    EXPECT_EQ(m_tester->m_pTitleWidget->m_pTitleCommentButton->isVisible(), false);
}

TEST_F(UT_MainWindow, test_loadArchive_001)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_isReadable(stub, false);
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    m_tester->m_operationtype = Operation_NULL;
    m_tester->loadArchive("1.zip");
    EXPECT_EQ(m_tester->m_operationtype != Operation_Load, true);
}

TEST_F(UT_MainWindow, test_loadArchive_002)
{
//    Stub stub;
//    QFileInfoStub::stub_QFileInfo_isReadable(stub, true);
//    QFileInfoStub::stub_QFileInfo_exists(stub, false);
//    m_tester->loadArchive("1.zip");
//    EXPECT_EQ(m_tester->m_ePageID == PI_Failure, true);
}

TEST_F(UT_MainWindow, test_loadArchive_003)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_isReadable(stub, true);
    QFileInfoStub::stub_QFileInfo_exists(stub, true);
    ArchiveManagerStub::stub_ArchiveManager_loadArchive(stub, true);
    m_tester->loadArchive("1.zip");
    EXPECT_EQ(m_tester->m_ePageID == PI_Loading, true);
}

TEST_F(UT_MainWindow, test_loadArchive_004)
{
    Stub stub;
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    QFileInfoStub::stub_QFileInfo_isReadable(stub, true);
    QFileInfoStub::stub_QFileInfo_exists(stub, true);
    ArchiveManagerStub::stub_ArchiveManager_loadArchive(stub, false);
    m_tester->loadArchive("1.zip");
    EXPECT_EQ(m_tester->m_ePageID == PI_Home, true);
}

TEST_F(UT_MainWindow, test_timerEvent_001)
{
    QTimerEvent *event = new QTimerEvent(m_tester->m_iInitUITimer);
    m_tester->timerEvent(event);
    delete event;
    EXPECT_EQ(m_tester->m_initFlag, true);
}

TEST_F(UT_MainWindow, test_timerEvent_002)
{
//    Stub stub;
//    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
//    stub.set(ADDR(CompressPage, compressFiles), compressPage_compressFiles_stub);
//    QFileInfoStub::stub_QFileInfo_exists(stub, false);

//    QTimerEvent *event = new QTimerEvent(m_tester->m_iCompressedWatchTimerID);
//    m_tester->timerEvent(event);
//    delete event;
//    EXPECT_EQ(m_tester->m_initFlag, false);
//    EXPECT_EQ(m_tester->m_ePageID, PI_Compress);
}

TEST_F(UT_MainWindow, test_timerEvent_003)
{
    Stub stub;
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    QFileInfoStub::stub_QFileInfo_exists(stub, false);

    QTimerEvent *event = new QTimerEvent(m_tester->m_iCompressedWatchTimerID);
    m_tester->timerEvent(event);
    delete event;
    EXPECT_EQ(m_tester->m_initFlag, false);
    EXPECT_EQ(m_tester->m_ePageID, PI_Home);
}

TEST_F(UT_MainWindow, test_closeEvent_001)
{
    Stub stub;
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 0);
    QCloseEvent *event = new QCloseEvent;
    m_tester->m_operationtype = Operation_Load;
    m_tester->closeEvent(event);
    delete event;
}

TEST_F(UT_MainWindow, test_closeEvent_002)
{
    Stub stub;
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 0);
    QCloseEvent *event = new QCloseEvent;
    m_tester->m_operationtype = Operation_Load;
    m_tester->closeEvent(event);
    delete event;
}

TEST_F(UT_MainWindow, test_closeEvent_003)
{
    Stub stub;
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 0);
    QCloseEvent *event = new QCloseEvent;
    m_tester->m_operationtype = Operation_NULL;
    m_tester->closeEvent(event);
    delete event;
}

TEST_F(UT_MainWindow, test_checkSettings_001)
{
    Stub stub;
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    QFileInfoStub::stub_QFileInfo_exists(stub, false);
    EXPECT_EQ(m_tester->checkSettings("1.zip"), false);
}

TEST_F(UT_MainWindow, test_checkSettings_002)
{
    Stub stub;
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    QFileInfoStub::stub_QFileInfo_exists(stub, true);
    QFileInfoStub::stub_QFileInfo_isReadable(stub, false);
    EXPECT_EQ(m_tester->checkSettings("1.zip"), false);
}

TEST_F(UT_MainWindow, test_checkSettings_003)
{
    Stub stub;
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    QFileInfoStub::stub_QFileInfo_exists(stub, true);
    QFileInfoStub::stub_QFileInfo_isReadable(stub, true);
    QFileInfoStub::stub_QFileInfo_isDir(stub, true);
    EXPECT_EQ(m_tester->checkSettings("1/"), false);
}

TEST_F(UT_MainWindow, test_checkSettings_004)
{
//    Stub stub;
//    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
//    QFileInfoStub::stub_QFileInfo_exists(stub, true);
//    QFileInfoStub::stub_QFileInfo_isReadable(stub, true);
//    QFileInfoStub::stub_QFileInfo_isDir(stub, false);
//    EXPECT_EQ(m_tester->checkSettings(""), true);
}

CustomMimeType determineMimeType_stub(const QString &)
{
    CustomMimeType type;
    type.m_bUnKnown = true;
    type.m_strTypeName = "application/zip";
    return type;
}

bool isExistMimeType_stub()
{
    return false;
}

TEST_F(UT_MainWindow, test_checkSettings_005)
{
    Stub stub;
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    QFileInfoStub::stub_QFileInfo_exists(stub, true);
    QFileInfoStub::stub_QFileInfo_isReadable(stub, true);
    QFileInfoStub::stub_QFileInfo_isDir(stub, false);
    stub.set(determineMimeType, determineMimeType_stub);
    stub.set(ADDR(UiTools, isExistMimeType), isExistMimeType_stub);
    EXPECT_EQ(m_tester->checkSettings("1.zip"), false);
}

TEST_F(UT_MainWindow, test_handleApplicationTabEventNotify_001)
{
    QKeyEvent *evt = new QKeyEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    bool bResult = m_tester->handleApplicationTabEventNotify(m_tester->titlebar(), evt);
    delete evt;
    EXPECT_EQ(bResult, false);
}

TEST_F(UT_MainWindow, test_handleApplicationTabEventNotify_002)
{
    QKeyEvent *evt = new QKeyEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    bool bResult = m_tester->handleApplicationTabEventNotify(m_tester->m_pTitleWidget->m_pTitleCommentButton, evt);
    delete evt;
    EXPECT_EQ(bResult, false);
}

TEST_F(UT_MainWindow, test_handleApplicationTabEventNotify_003)
{
    QKeyEvent *evt = new QKeyEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    bool bResult = m_tester->handleApplicationTabEventNotify(m_tester->m_pTitleWidget->m_pTitleButton, evt);
    delete evt;
    EXPECT_EQ(bResult, false);
}

TEST_F(UT_MainWindow, test_handleApplicationTabEventNotify_004)
{
    QKeyEvent *evt = new QKeyEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    m_tester->m_ePageID = PI_UnCompress;
    bool bResult = m_tester->handleApplicationTabEventNotify(m_tester->m_pUnCompressPage->getUnCompressView()->m_pHeaderView->m_pPreLbl, evt);
    delete evt;
    EXPECT_EQ(bResult, false);
}

TEST_F(UT_MainWindow, test_handleApplicationTabEventNotify_005)
{
    QKeyEvent *evt = new QKeyEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    m_tester->m_ePageID = PI_Compress;
    bool bResult = m_tester->handleApplicationTabEventNotify(m_tester->m_pCompressPage->getCompressView()->m_pHeaderView->m_pPreLbl, evt);
    delete evt;
    EXPECT_EQ(bResult, false);
}

TEST_F(UT_MainWindow, test_handleApplicationTabEventNotify_006)
{
    QKeyEvent *evt = new QKeyEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    bool bResult = m_tester->handleApplicationTabEventNotify(m_tester->m_pUnCompressPage->getUnCompressView(), evt);
    delete evt;
    EXPECT_EQ(bResult, false);
}

TEST_F(UT_MainWindow, test_handleApplicationTabEventNotify_007)
{
    QKeyEvent *evt = new QKeyEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    bool bResult = m_tester->handleApplicationTabEventNotify(m_tester->m_pCompressPage->getCompressView(), evt);
    delete evt;
    EXPECT_EQ(bResult, false);
}

TEST_F(UT_MainWindow, test_handleApplicationTabEventNotify_008)
{
    QKeyEvent *evt = new QKeyEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    m_tester->m_ePageID = PI_UnCompress;
    bool bResult = m_tester->handleApplicationTabEventNotify(m_tester->titlebar()->findChild<DWindowCloseButton *>("DTitlebarDWindowCloseButton"), evt);
    delete evt;
    EXPECT_EQ(bResult, false);
}

TEST_F(UT_MainWindow, test_handleApplicationTabEventNotify_009)
{
    QKeyEvent *evt = new QKeyEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    m_tester->m_ePageID = PI_Compress;
    bool bResult = m_tester->handleApplicationTabEventNotify(m_tester->titlebar()->findChild<DWindowCloseButton *>("DTitlebarDWindowCloseButton"), evt);
    delete evt;
    EXPECT_EQ(bResult, false);
}

TEST_F(UT_MainWindow, test_handleApplicationTabEventNotify_010)
{
    QKeyEvent *evt = new QKeyEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    m_tester->m_ePageID = PI_CompressSetting;
    bool bResult = m_tester->handleApplicationTabEventNotify(m_tester->titlebar()->findChild<DWindowCloseButton *>("DTitlebarDWindowCloseButton"), evt);
    delete evt;
    EXPECT_EQ(bResult, false);
}

TEST_F(UT_MainWindow, test_handleApplicationTabEventNotify_011)
{
    QKeyEvent *evt = new QKeyEvent(QEvent::KeyPress, Qt::Key_Backtab, Qt::NoModifier);
    bool bResult = m_tester->handleApplicationTabEventNotify(m_tester->titlebar(), evt);
    delete evt;
    EXPECT_EQ(bResult, false);
}

TEST_F(UT_MainWindow, test_handleApplicationTabEventNotify_012)
{
    QKeyEvent *evt = new QKeyEvent(QEvent::KeyPress, Qt::Key_Backtab, Qt::NoModifier);
    bool bResult = m_tester->handleApplicationTabEventNotify(m_tester->m_pTitleWidget->m_pTitleCommentButton, evt);
    delete evt;
    EXPECT_EQ(bResult, false);
}

TEST_F(UT_MainWindow, test_handleApplicationTabEventNotify_013)
{
    QKeyEvent *evt = new QKeyEvent(QEvent::KeyPress, Qt::Key_Backtab, Qt::NoModifier);
    bool bResult = m_tester->handleApplicationTabEventNotify(m_tester->m_pTitleWidget->m_pTitleButton, evt);
    delete evt;
    EXPECT_EQ(bResult, false);
}

TEST_F(UT_MainWindow, test_handleApplicationTabEventNotify_014)
{
    QKeyEvent *evt = new QKeyEvent(QEvent::KeyPress, Qt::Key_Backtab, Qt::NoModifier);
    m_tester->m_ePageID = PI_UnCompress;
    bool bResult = m_tester->handleApplicationTabEventNotify(m_tester->m_pUnCompressPage->getUnCompressView()->m_pHeaderView->m_pPreLbl, evt);
    delete evt;
    EXPECT_EQ(bResult, false);
}

TEST_F(UT_MainWindow, test_handleApplicationTabEventNotify_015)
{
    QKeyEvent *evt = new QKeyEvent(QEvent::KeyPress, Qt::Key_Backtab, Qt::NoModifier);
    m_tester->m_ePageID = PI_Compress;
    bool bResult = m_tester->handleApplicationTabEventNotify(m_tester->m_pCompressPage->getCompressView()->m_pHeaderView->m_pPreLbl, evt);
    delete evt;
    EXPECT_EQ(bResult, false);
}

TEST_F(UT_MainWindow, test_handleApplicationTabEventNotify_016)
{
    QKeyEvent *evt = new QKeyEvent(QEvent::KeyPress, Qt::Key_Backtab, Qt::NoModifier);
    bool bResult = m_tester->handleApplicationTabEventNotify(m_tester->m_pUnCompressPage->getUnCompressView(), evt);
    delete evt;
    EXPECT_EQ(bResult, false);
}

TEST_F(UT_MainWindow, test_handleApplicationTabEventNotify_017)
{
    QKeyEvent *evt = new QKeyEvent(QEvent::KeyPress, Qt::Key_Backtab, Qt::NoModifier);
    bool bResult = m_tester->handleApplicationTabEventNotify(m_tester->m_pCompressPage->getCompressView(), evt);
    delete evt;
    EXPECT_EQ(bResult, false);
}

TEST_F(UT_MainWindow, test_handleApplicationTabEventNotify_018)
{
    QKeyEvent *evt = new QKeyEvent(QEvent::KeyPress, Qt::Key_Backtab, Qt::NoModifier);
    m_tester->m_ePageID = PI_UnCompress;
    bool bResult = m_tester->handleApplicationTabEventNotify(m_tester->titlebar()->findChild<DWindowCloseButton *>("DTitlebarDWindowCloseButton"), evt);
    delete evt;
    EXPECT_EQ(bResult, false);
}

TEST_F(UT_MainWindow, test_handleApplicationTabEventNotify_019)
{
    QKeyEvent *evt = new QKeyEvent(QEvent::KeyPress, Qt::Key_Backtab, Qt::NoModifier);
    m_tester->m_ePageID = PI_Compress;
    bool bResult = m_tester->handleApplicationTabEventNotify(m_tester->titlebar()->findChild<DWindowCloseButton *>("DTitlebarDWindowCloseButton"), evt);
    delete evt;
    EXPECT_EQ(bResult, false);
}

TEST_F(UT_MainWindow, test_handleApplicationTabEventNotify_020)
{
    QKeyEvent *evt = new QKeyEvent(QEvent::KeyPress, Qt::Key_Backtab, Qt::NoModifier);
    m_tester->m_ePageID = PI_CompressSetting;
    bool bResult = m_tester->handleApplicationTabEventNotify(m_tester->titlebar()->findChild<DWindowCloseButton *>("DTitlebarDWindowCloseButton"), evt);
    delete evt;
    EXPECT_EQ(bResult, false);
}

TEST_F(UT_MainWindow, testhandleQuit)
{
    m_tester->handleQuit();
}

TEST_F(UT_MainWindow, test_slotHandleArguments_001)
{
    Stub stub;
    stub.set(ADDR(MainWindow, handleArguments_Open), handleArguments_Open_stub);
    stub.set(ADDR(MainWindow, handleArguments_RightMenu), handleArguments_RightMenu_stub);
    stub.set(ADDR(MainWindow, handleArguments_Append), handleArguments_Append_stub);

    m_tester->slotHandleArguments(QStringList(), MainWindow::ArgumentType::AT_Open);
}

TEST_F(UT_MainWindow, test_slotHandleArguments_002)
{
    Stub stub;
    stub.set(ADDR(MainWindow, handleArguments_Open), handleArguments_Open_stub);
    stub.set(ADDR(MainWindow, handleArguments_RightMenu), handleArguments_RightMenu_stub);
    stub.set(ADDR(MainWindow, handleArguments_Append), handleArguments_Append_stub);
    m_tester->slotHandleArguments(QStringList() << "1.zip", MainWindow::ArgumentType::AT_Open);
}

TEST_F(UT_MainWindow, test_slotHandleArguments_003)
{
    Stub stub;
    stub.set(ADDR(MainWindow, handleArguments_Open), handleArguments_Open_stub);
    stub.set(ADDR(MainWindow, handleArguments_RightMenu), handleArguments_RightMenu_stub);
    stub.set(ADDR(MainWindow, handleArguments_Append), handleArguments_Append_stub);
    m_tester->slotHandleArguments(QStringList() << "1.zip", MainWindow::ArgumentType::AT_RightMenu);
}

TEST_F(UT_MainWindow, test_slotHandleArguments_004)
{
    Stub stub;
    stub.set(ADDR(MainWindow, handleArguments_Open), handleArguments_Open_stub);
    stub.set(ADDR(MainWindow, handleArguments_RightMenu), handleArguments_RightMenu_stub);
    stub.set(ADDR(MainWindow, handleArguments_Append), handleArguments_Append_stub);
    m_tester->slotHandleArguments(QStringList() << "1.zip", MainWindow::ArgumentType::AT_DragDropAdd);
}

TEST_F(UT_MainWindow, test_slotTitleBtnClicked_001)
{
    Stub stub;
    stub.set(ADDR(MainWindow, slotChoosefiles), slotChoosefiles_stub);

    m_tester->m_ePageID = PI_Home;
    m_tester->slotTitleBtnClicked();
    EXPECT_EQ(m_tester->m_ePageID, PI_Home);
}

TEST_F(UT_MainWindow, test_slotTitleBtnClicked_002)
{
    m_tester->m_ePageID = PI_CompressSetting;
    m_tester->slotTitleBtnClicked();
    EXPECT_EQ(m_tester->m_ePageID, PI_Compress);
}

TEST_F(UT_MainWindow, test_slotChoosefiles_001)
{
    Stub stub;
    DFileDialogStub::stub_DFileDialog_exec(stub, 0);
    stub.set(ADDR(MainWindow, loadArchive), loadArchive_stub);
    stub.set(ADDR(CompressPage, addCompressFiles), compressPage_addCompressFiles_stub);
    stub.set(ADDR(UnCompressPage, addNewFiles), UnCompressPage_addNewFiles_stub);
    m_tester->slotChoosefiles();
}

TEST_F(UT_MainWindow, test_slotChoosefiles_002)
{
    Stub stub;
    stub.set(ADDR(MainWindow, loadArchive), loadArchive_stub);
    DFileDialogStub::stub_DFileDialog_exec(stub, 1);
    DFileDialogStub::stub_DFileDialog_selectedFiles(stub, QStringList() << "1.txt");
    CommonStub::stub_UiTools_isLocalDeviceFile(stub, true);
    CommonStub::stub_UiTools_isArchiveFile(stub, true);
    m_tester->m_ePageID = PI_Home;
    m_tester->slotChoosefiles();
    EXPECT_EQ(m_tester->m_ePageID, PI_Home);
}

TEST_F(UT_MainWindow, test_slotChoosefiles_003)
{
    Stub stub;
    stub.set(ADDR(MainWindow, loadArchive), loadArchive_stub);
    DFileDialogStub::stub_DFileDialog_exec(stub, 1);
    DFileDialogStub::stub_DFileDialog_selectedFiles(stub, QStringList() << "1.txt");
    CommonStub::stub_UiTools_isLocalDeviceFile(stub, true);
    CommonStub::stub_UiTools_isArchiveFile(stub, false);
    m_tester->m_ePageID = PI_Home;
    m_tester->slotChoosefiles();
    EXPECT_EQ(m_tester->m_ePageID, PI_Compress);
}

TEST_F(UT_MainWindow, test_slotChoosefiles_004)
{
    Stub stub;
    stub.set(ADDR(MainWindow, loadArchive), loadArchive_stub);
    DFileDialogStub::stub_DFileDialog_exec(stub, 1);
    DFileDialogStub::stub_DFileDialog_selectedFiles(stub, QStringList() << "1.txt");
    CommonStub::stub_UiTools_isLocalDeviceFile(stub, true);
    CommonStub::stub_UiTools_isArchiveFile(stub, false);

    m_tester->m_ePageID = PI_Compress;
    m_tester->slotChoosefiles();
    EXPECT_EQ(m_tester->m_ePageID, PI_Compress);
}

TEST_F(UT_MainWindow, test_slotChoosefiles_005)
{
    Stub stub;
    stub.set(ADDR(MainWindow, loadArchive), loadArchive_stub);
    DFileDialogStub::stub_DFileDialog_exec(stub, 1);
    DFileDialogStub::stub_DFileDialog_selectedFiles(stub, QStringList() << "1.txt");
    CommonStub::stub_UiTools_isLocalDeviceFile(stub, true);
    stub.set(ADDR(UnCompressPage, addNewFiles), UnCompressPage_addNewFiles_stub);
    CommonStub::stub_UiTools_isArchiveFile(stub, false);

    m_tester->m_ePageID = PI_UnCompress;
    m_tester->slotChoosefiles();
    EXPECT_EQ(m_tester->m_ePageID, PI_UnCompress);
}

TEST_F(UT_MainWindow, test_slotChoosefiles_006)
{
    Stub stub;
    stub.set(ADDR(MainWindow, loadArchive), loadArchive_stub);
    DFileDialogStub::stub_DFileDialog_exec(stub, 1);
    DFileDialogStub::stub_DFileDialog_selectedFiles(stub, QStringList() << "1.txt");
    CommonStub::stub_UiTools_isLocalDeviceFile(stub, true);
    CommonStub::stub_UiTools_isArchiveFile(stub, false);

    m_tester->m_ePageID = PI_CompressSetting;
    m_tester->slotChoosefiles();
    EXPECT_EQ(m_tester->m_ePageID, PI_Compress);
}

TEST_F(UT_MainWindow, test_slotDragSelectedFiles_001)
{
    Stub stub;
    stub.set(ADDR(MainWindow, loadArchive), loadArchive_stub);
    stub.set(ADDR(CompressPage, addCompressFiles), compressPage_addCompressFiles_stub);

    m_tester->slotDragSelectedFiles(QStringList());
}

TEST_F(UT_MainWindow, test_slotDragSelectedFiles_002)
{
    Stub stub;
    stub.set(ADDR(MainWindow, loadArchive), loadArchive_stub);
    stub.set(ADDR(CompressPage, addCompressFiles), compressPage_addCompressFiles_stub);

    m_tester->slotDragSelectedFiles(QStringList() << "1.txt" << "2.txt");
    EXPECT_EQ(m_tester->m_ePageID, PI_Compress);
}

TEST_F(UT_MainWindow, test_slotDragSelectedFiles_003)
{
    Stub stub;
    stub.set(ADDR(MainWindow, loadArchive), loadArchive_stub);
    stub.set(ADDR(CompressPage, addCompressFiles), compressPage_addCompressFiles_stub);
    CommonStub::stub_UiTools_isArchiveFile(stub, true);
    m_tester->slotDragSelectedFiles(QStringList() << "1.zip");
    EXPECT_EQ(m_tester->m_ePageID, PI_Home);
}

TEST_F(UT_MainWindow, test_slotDragSelectedFiles_004)
{
    Stub stub;
    stub.set(ADDR(MainWindow, loadArchive), loadArchive_stub);
    stub.set(ADDR(CompressPage, addCompressFiles), compressPage_addCompressFiles_stub);
    CommonStub::stub_UiTools_isArchiveFile(stub, false);
    m_tester->slotDragSelectedFiles(QStringList() << "1.txt");
    EXPECT_EQ(m_tester->m_ePageID, PI_Compress);
}

TEST_F(UT_MainWindow, test_slotCompressLevelChanged)
{
    m_tester->slotCompressLevelChanged(true);
    EXPECT_EQ(m_tester->m_pOpenAction->isEnabled(), true);
}

TEST_F(UT_MainWindow, test_slotCompressNext)
{
    m_tester->slotCompressNext();
    EXPECT_EQ(m_tester->m_ePageID, PI_CompressSetting);
}

TEST_F(UT_MainWindow, test_slotCompress_001)
{
    Stub stub;
    ArchiveManagerStub::stub_ArchiveManager_createArchive(stub, true);
    stub.set(ADDR(CompressPage, compressFiles), compressPage_compressFiles_stub);
    m_tester->slotCompress(QVariant());
    EXPECT_EQ(m_tester->m_ePageID, PI_CompressProgress);
    EXPECT_EQ(m_tester->m_operationtype, Operation_Create);
}

TEST_F(UT_MainWindow, test_slotCompress_002)
{
    Stub stub;
    stub.set(ADDR(CompressPage, compressFiles), compressPage_compressFiles_stub);
    ArchiveManagerStub::stub_ArchiveManager_createArchive(stub, false);
    m_tester->slotCompress(QVariant());
    EXPECT_EQ(m_tester->m_operationtype, Operation_NULL);
}

TEST_F(UT_MainWindow, test_slotJobFinished_001)
{
    Stub stub;
    stub.set(ADDR(MainWindow, handleJobNormalFinished), handleJobNormalFinished_stub);
    stub.set(ADDR(MainWindow, handleJobCancelFinished), handleJobCancelFinished_stub);
    stub.set(ADDR(MainWindow, handleJobErrorFinished), handleJobErrorFinished_stub);

    m_tester->m_eStartupType = StartupType::ST_Compress;
    m_tester->slotJobFinished(ArchiveJob::JT_Create, PFT_Nomral, ET_NoError);
    EXPECT_EQ(m_tester->m_operationtype, Operation_NULL);
    EXPECT_EQ(m_tester->m_fileWriteErrorName.isEmpty(), true);
}

TEST_F(UT_MainWindow, test_slotJobFinished_002)
{
    Stub stub;
    stub.set(ADDR(MainWindow, handleJobNormalFinished), handleJobNormalFinished_stub);
    stub.set(ADDR(MainWindow, handleJobCancelFinished), handleJobCancelFinished_stub);
    stub.set(ADDR(MainWindow, handleJobErrorFinished), handleJobErrorFinished_stub);

    m_tester->m_eStartupType = StartupType::ST_DragDropAdd;
    m_tester->slotJobFinished(ArchiveJob::JT_Add, PFT_Cancel, ET_UserCancelOpertion);
    EXPECT_EQ(m_tester->m_operationtype, Operation_NULL);
    EXPECT_EQ(m_tester->m_fileWriteErrorName.isEmpty(), true);
}

TEST_F(UT_MainWindow, test_slotJobFinished_003)
{
    Stub stub;
    stub.set(ADDR(MainWindow, handleJobNormalFinished), handleJobNormalFinished_stub);
    stub.set(ADDR(MainWindow, handleJobCancelFinished), handleJobCancelFinished_stub);
    stub.set(ADDR(MainWindow, handleJobErrorFinished), handleJobErrorFinished_stub);

    m_tester->slotJobFinished(ArchiveJob::JT_Create, PFT_Error, ET_FileWriteError);
    EXPECT_EQ(m_tester->m_operationtype, Operation_NULL);
    EXPECT_EQ(m_tester->m_fileWriteErrorName.isEmpty(), true);
}

CustomMimeType determineMimeType_stub_7z(const QString &)
{
    CustomMimeType type;
    type.m_bUnKnown = true;
    type.m_strTypeName = "application/x-7z-compressed";
    return type;
}

QString archiveFullPath_stub_7z()
{
    return "1.tar.7z";
}

TEST_F(UT_MainWindow, test_slotUncompressClicked)
{
    Stub stub;
    stub.set(ADDR(UnCompressPage, archiveFullPath), archiveFullPath_stub_7z);
    stub.set(determineMimeType, determineMimeType_stub_7z);
    ArchiveManagerStub::stub_ArchiveManager_extractFiles(stub, true);

    FileEntry entry;
    entry.strFileName = "1.tar";
    entry.strFullPath = "1.tar";
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;

    m_tester->slotUncompressClicked("/home/Desktop");
    EXPECT_EQ(m_tester->m_operationtype, Operation_Extract);
    EXPECT_EQ(m_tester->m_stUnCompressParameter.strExtractPath.isEmpty(), false);
    EXPECT_EQ(m_tester->m_ePageID, PI_UnCompressProgress);
}

TEST_F(UT_MainWindow, test_slotReceiveProgress_001)
{
    Stub stub;
    stub.set(ADDR(ProgressDialog, showDialog), progressdialog_showDialog_stub);
    CommonStub::stub_QDialog_exec(stub, 0);

    m_tester->m_operationtype = Operation_SingleExtract;
    m_tester->slotReceiveProgress(50);
    EXPECT_EQ(m_tester->m_pProgressdialog->m_dPerent, 50);
}

TEST_F(UT_MainWindow, test_slotReceiveProgress_002)
{
    Stub stub;
    stub.set(ADDR(ProgressDialog, showDialog), progressdialog_showDialog_stub);
    CommonStub::stub_QDialog_exec(stub, 0);

    m_tester->m_operationtype = Operation_Update_Comment;
    m_tester->slotReceiveProgress(50);
    EXPECT_EQ(m_tester->m_commentProgressDialog->m_progressBar->value(), 50);
}

TEST_F(UT_MainWindow, test_slotReceiveProgress_003)
{
    Stub stub;
    stub.set(ADDR(ProgressDialog, showDialog), progressdialog_showDialog_stub);
    CommonStub::stub_QDialog_exec(stub, 0);

    m_tester->m_operationtype = Operation_Extract;
    m_tester->slotReceiveProgress(50);
    EXPECT_EQ(m_tester->m_pProgressPage->m_iPerent, 50);
}

TEST_F(UT_MainWindow, test_slotReceiveCurFileName_001)
{
    m_tester->m_operationtype = Operation_SingleExtract;
    m_tester->slotReceiveCurFileName("1.txt");
    EXPECT_EQ(m_tester->m_pProgressdialog->m_filelable->text().contains("1.txt"), true);
}

TEST_F(UT_MainWindow, test_slotReceiveCurFileName_002)
{
    m_tester->m_operationtype = Operation_Extract;
    m_tester->slotReceiveCurFileName("1.txt");
    EXPECT_EQ(m_tester->m_pProgressPage->m_pFileNameLbl->text().contains("1.txt"), true);
}

TEST_F(UT_MainWindow, test_slotReceiveFileWriteErrorName)
{
    m_tester->m_fileWriteErrorName.clear();
    m_tester->slotReceiveFileWriteErrorName("1.txt");
    EXPECT_EQ(m_tester->m_fileWriteErrorName, "1.txt");
}

TEST_F(UT_MainWindow, test_slotQuery)
{
    Stub stub;
    CommonStub::stub_OverwriteQuery_execute(stub);
    OverwriteQuery query("", m_tester);
    m_tester->slotQuery(&query);
}

TEST_F(UT_MainWindow, test_Extract2PathFinish)
{
    Stub stub;
    CommonStub::stub_QThread_start(stub);
    stub.set(ADDR(SettingDialog, isAutoOpen), settingDialog_isAutoOpen_stub);
    m_tester->Extract2PathFinish("ddd");
    EXPECT_NE(m_tester->m_pDDesktopServicesThread, nullptr);
}

TEST_F(UT_MainWindow, test_createTempPath)
{
    EXPECT_EQ(m_tester->createTempPath().isEmpty(), false);
}

TEST_F(UT_MainWindow, test_getExtractPath)
{
    Stub stub;
    stub.set(ADDR(SettingDialog, isAutoCreatDir), settingDialog_isAutoCreatDir_stub);
    EXPECT_EQ(m_tester->getExtractPath("/home/Desktop/1.zip"), "1");
}

TEST_F(UT_MainWindow, test_handleJobNormalFinished_001)
{
    Stub stub;
    CommonStub::stub_QThread_start(stub);
    stub.set(ADDR(SettingDialog, isAutoDeleteFile), settingDialog_isAutoDeleteFile_stub);
    ArchiveManagerStub::stub_ArchiveManager_updateArchiveComment(stub, true);
    ArchiveManagerStub::stub_ArchiveManager_updateArchiveCacheData(stub, true);
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 0);

    m_tester->m_iCompressedWatchTimerID = 1000;
    m_tester->handleJobNormalFinished(ArchiveJob::JT_Create);
    EXPECT_EQ(m_tester->m_ePageID, PI_Success);
    EXPECT_EQ(m_tester->m_iCompressedWatchTimerID, 0);
}

TEST_F(UT_MainWindow, test_handleJobNormalFinished_002)
{
    Stub stub;
    CommonStub::stub_QThread_start(stub);
    stub.set(ADDR(SettingDialog, isAutoDeleteFile), settingDialog_isAutoDeleteFile_stub);
    ArchiveManagerStub::stub_ArchiveManager_updateArchiveComment(stub, true);
    ArchiveManagerStub::stub_ArchiveManager_updateArchiveCacheData(stub, true);
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 0);

    m_tester->m_eStartupType = StartupType::ST_DragDropAdd;
    m_tester->handleJobNormalFinished(ArchiveJob::JT_Add);
    EXPECT_EQ(m_tester->m_ePageID, PI_Success);
}

TEST_F(UT_MainWindow, test_handleJobNormalFinished_003)
{
    Stub stub;
    CommonStub::stub_QThread_start(stub);
    stub.set(ADDR(SettingDialog, isAutoDeleteFile), settingDialog_isAutoDeleteFile_stub);
    ArchiveManagerStub::stub_ArchiveManager_updateArchiveComment(stub, true);
    ArchiveManagerStub::stub_ArchiveManager_updateArchiveCacheData(stub, true);
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 0);
    m_tester->m_eStartupType = StartupType::ST_Normal;
    m_tester->handleJobNormalFinished(ArchiveJob::JT_Add);
    EXPECT_EQ(m_tester->m_operationtype, Operation_UpdateData);
    EXPECT_EQ(m_tester->m_ePageID, PI_Loading);
}

TEST_F(UT_MainWindow, test_handleJobNormalFinished_004)
{
    Stub stub;
    CommonStub::stub_QThread_start(stub);
    stub.set(ADDR(SettingDialog, isAutoDeleteFile), settingDialog_isAutoDeleteFile_stub);
    ArchiveManagerStub::stub_ArchiveManager_updateArchiveComment(stub, true);
    ArchiveManagerStub::stub_ArchiveManager_updateArchiveCacheData(stub, true);
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 0);

    DataManager::get_instance().archiveData().reset();
    m_tester->handleJobNormalFinished(ArchiveJob::JT_Load);
    EXPECT_EQ(m_tester->m_ePageID, PI_Home);
}

TEST_F(UT_MainWindow, test_handleJobNormalFinished_005)
{
    Stub stub;
    CommonStub::stub_QThread_start(stub);
    stub.set(ADDR(SettingDialog, isAutoDeleteFile), settingDialog_isAutoDeleteFile_stub);
    ArchiveManagerStub::stub_ArchiveManager_updateArchiveComment(stub, true);
    ArchiveManagerStub::stub_ArchiveManager_updateArchiveCacheData(stub, true);
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 0);

    m_tester->m_operationtype = Archive_OperationType::Operation_SingleExtract;
    m_tester->handleJobNormalFinished(ArchiveJob::JT_Extract);
    EXPECT_EQ(m_tester->m_ePageID, PI_UnCompress);
}

TEST_F(UT_MainWindow, test_handleJobNormalFinished_006)
{
    Stub stub;
    CommonStub::stub_QThread_start(stub);
    stub.set(ADDR(SettingDialog, isAutoDeleteFile), settingDialog_isAutoDeleteFile_stub);
    ArchiveManagerStub::stub_ArchiveManager_updateArchiveComment(stub, true);
    ArchiveManagerStub::stub_ArchiveManager_updateArchiveCacheData(stub, true);
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 0);

    m_tester->m_operationtype = Archive_OperationType::Operation_Extract;
    DataManager::get_instance().archiveData().reset();
    m_tester->handleJobNormalFinished(ArchiveJob::JT_Extract);
    EXPECT_EQ(m_tester->m_ePageID, PI_Failure);
}

TEST_F(UT_MainWindow, test_handleJobNormalFinished_007)
{
    Stub stub;
    CommonStub::stub_QThread_start(stub);
    stub.set(ADDR(SettingDialog, isAutoDeleteFile), settingDialog_isAutoDeleteFile_stub);
    ArchiveManagerStub::stub_ArchiveManager_updateArchiveComment(stub, true);
    ArchiveManagerStub::stub_ArchiveManager_updateArchiveCacheData(stub, true);
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 0);

    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->m_stUnCompressParameter.bBatch = true;
    m_tester->handleJobNormalFinished(ArchiveJob::JT_Extract);
    EXPECT_EQ(m_tester->m_ePageID, PI_Success);
}

TEST_F(UT_MainWindow, test_handleJobNormalFinished_008)
{
    Stub stub;
    CommonStub::stub_QThread_start(stub);
    stub.set(ADDR(SettingDialog, isAutoDeleteFile), settingDialog_isAutoDeleteFile_stub);
    ArchiveManagerStub::stub_ArchiveManager_updateArchiveComment(stub, true);
    ArchiveManagerStub::stub_ArchiveManager_updateArchiveCacheData(stub, true);
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 0);

    m_tester->m_stUnCompressParameter.bBatch = false;
    m_tester->handleJobNormalFinished(ArchiveJob::JT_Extract);
    EXPECT_EQ(m_tester->m_ePageID, PI_Success);
}

TEST_F(UT_MainWindow, test_handleJobNormalFinished_009)
{
    Stub stub;
    CommonStub::stub_QThread_start(stub);
    stub.set(ADDR(SettingDialog, isAutoDeleteFile), settingDialog_isAutoDeleteFile_stub);
    ArchiveManagerStub::stub_ArchiveManager_updateArchiveComment(stub, true);
    ArchiveManagerStub::stub_ArchiveManager_updateArchiveCacheData(stub, true);
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 0);

    m_tester->handleJobNormalFinished(ArchiveJob::JT_Delete);
    EXPECT_EQ(m_tester->m_ePageID, PI_Loading);
}

TEST_F(UT_MainWindow, test_handleJobNormalFinished_0010)
{
    Stub stub;
    CommonStub::stub_QThread_start(stub);
    stub.set(ADDR(SettingDialog, isAutoDeleteFile), settingDialog_isAutoDeleteFile_stub);
    ArchiveManagerStub::stub_ArchiveManager_updateArchiveComment(stub, true);
    ArchiveManagerStub::stub_ArchiveManager_updateArchiveCacheData(stub, true);
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 0);

    m_tester->m_stUnCompressParameter.bModifiable = true;
    m_tester->handleJobNormalFinished(ArchiveJob::JT_Open);
    EXPECT_EQ(m_tester->m_ePageID, PI_UnCompress);
}

TEST_F(UT_MainWindow, test_handleJobNormalFinished_0011)
{
//    Stub stub;
//    CommonStub::stub_QThread_start(stub);
//    stub.set(ADDR(SettingDialog, isAutoDeleteFile), settingDialog_isAutoDeleteFile_stub);
//    ArchiveManagerStub::stub_ArchiveManager_updateArchiveComment(stub, true);
//    ArchiveManagerStub::stub_ArchiveManager_updateArchiveCacheData(stub, true);
//    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
//    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 0);

//    m_tester->handleJobNormalFinished(ArchiveJob::JT_Convert);
//    EXPECT_EQ(m_tester->m_ePageID, PI_Success);
}

TEST_F(UT_MainWindow, test_handleJobNormalFinished_0012)
{
    Stub stub;
    CommonStub::stub_QThread_start(stub);
    stub.set(ADDR(SettingDialog, isAutoDeleteFile), settingDialog_isAutoDeleteFile_stub);
    ArchiveManagerStub::stub_ArchiveManager_updateArchiveComment(stub, true);
    ArchiveManagerStub::stub_ArchiveManager_updateArchiveCacheData(stub, true);
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 0);

    m_tester->handleJobNormalFinished(ArchiveJob::JT_Update);
    EXPECT_EQ(m_tester->m_ePageID, PI_UnCompress);
}

TEST_F(UT_MainWindow, test_handleJobNormalFinished_0013)
{
    Stub stub;
    CommonStub::stub_QThread_start(stub);
    stub.set(ADDR(SettingDialog, isAutoDeleteFile), settingDialog_isAutoDeleteFile_stub);
    ArchiveManagerStub::stub_ArchiveManager_updateArchiveComment(stub, true);
    ArchiveManagerStub::stub_ArchiveManager_updateArchiveCacheData(stub, true);
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 0);

    DataManager::get_instance().archiveData().reset();
    m_tester->handleJobNormalFinished(ArchiveJob::JT_Update);
    EXPECT_EQ(m_tester->m_ePageID, PI_Home);
}

TEST_F(UT_MainWindow, test_handleJobNormalFinished_0014)
{
    Stub stub;
    CommonStub::stub_QThread_start(stub);
    stub.set(ADDR(SettingDialog, isAutoDeleteFile), settingDialog_isAutoDeleteFile_stub);
    ArchiveManagerStub::stub_ArchiveManager_updateArchiveComment(stub, true);
    ArchiveManagerStub::stub_ArchiveManager_updateArchiveCacheData(stub, true);
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 0);

    m_tester->m_operationtype = Operation_Update_Comment;
    m_tester->handleJobNormalFinished(ArchiveJob::JT_Comment);
    EXPECT_EQ(m_tester->m_commentProgressDialog->m_progressBar->value(), 100);
}

TEST_F(UT_MainWindow, test_handleJobNormalFinished_0015)
{
    Stub stub;
    CommonStub::stub_QThread_start(stub);
    stub.set(ADDR(SettingDialog, isAutoDeleteFile), settingDialog_isAutoDeleteFile_stub);
    ArchiveManagerStub::stub_ArchiveManager_updateArchiveComment(stub, true);
    ArchiveManagerStub::stub_ArchiveManager_updateArchiveCacheData(stub, true);
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 0);
    m_tester->m_operationtype = Operation_NULL;
    m_tester->handleJobNormalFinished(ArchiveJob::JT_Comment);
    EXPECT_EQ(m_tester->m_ePageID, PI_Success);
}

TEST_F(UT_MainWindow, test_handleJobNormalFinished_016)
{
    Stub stub;
    CommonStub::stub_QThread_start(stub);
    stub.set(ADDR(SettingDialog, isAutoDeleteFile), settingDialog_isAutoDeleteFile_stub);
    ArchiveManagerStub::stub_ArchiveManager_updateArchiveComment(stub, true);
    ArchiveManagerStub::stub_ArchiveManager_updateArchiveCacheData(stub, true);
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 0);

    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;

    m_tester->handleJobNormalFinished(ArchiveJob::JT_Load);
    EXPECT_EQ(m_tester->m_ePageID, PI_UnCompress);
}

TEST_F(UT_MainWindow, test_handleJobCancelFinished_001)
{
    m_tester->m_eStartupType = StartupType::ST_Compresstozip7z;
    m_tester->handleJobCancelFinished(ArchiveJob::JT_Create);
    EXPECT_EQ(m_tester->m_operationtype, Operation_NULL);
}

TEST_F(UT_MainWindow, test_handleJobCancelFinished_002)
{
    m_tester->m_eStartupType = StartupType::ST_Compress;
    m_tester->handleJobCancelFinished(ArchiveJob::JT_Create);
    EXPECT_EQ(m_tester->m_ePageID, PI_Compress);
}

TEST_F(UT_MainWindow, test_handleJobCancelFinished_003)
{
    m_tester->m_eStartupType = StartupType::ST_DragDropAdd;
    m_tester->handleJobCancelFinished(ArchiveJob::JT_Add);
    EXPECT_EQ(m_tester->m_operationtype, Operation_NULL);
}

TEST_F(UT_MainWindow, test_handleJobCancelFinished_004)
{
    m_tester->m_eStartupType = StartupType::ST_Compress;
    m_tester->handleJobCancelFinished(ArchiveJob::JT_Add);
    EXPECT_EQ(m_tester->m_ePageID, PI_UnCompress);
}

TEST_F(UT_MainWindow, test_handleJobCancelFinished_005)
{
    m_tester->handleJobCancelFinished(ArchiveJob::JT_Load);
    EXPECT_EQ(m_tester->m_ePageID, PI_Home);
}

TEST_F(UT_MainWindow, test_handleJobCancelFinished_006)
{
    m_tester->m_operationtype = Archive_OperationType::Operation_SingleExtract;
    m_tester->handleJobCancelFinished(ArchiveJob::JT_Extract);
    m_tester->m_operationtype = Archive_OperationType::Operation_Extract;
    m_tester->m_eStartupType = ST_ExtractHere;
    m_tester->handleJobCancelFinished(ArchiveJob::JT_Extract);
    EXPECT_EQ(m_tester->m_operationtype, Operation_NULL);
}

TEST_F(UT_MainWindow, test_handleJobCancelFinished_007)
{
    m_tester->m_eStartupType = ST_Extract;
    m_tester->handleJobCancelFinished(ArchiveJob::JT_Extract);
    EXPECT_EQ(m_tester->m_ePageID, PI_UnCompress);
}

TEST_F(UT_MainWindow, test_handleJobCancelFinished_008)
{
    m_tester->handleJobCancelFinished(ArchiveJob::JT_Delete);
    EXPECT_EQ(m_tester->m_ePageID, PI_UnCompress);
}

TEST_F(UT_MainWindow, test_handleJobCancelFinished_009)
{
    m_tester->handleJobCancelFinished(ArchiveJob::JT_Open);
    EXPECT_EQ(m_tester->m_ePageID, PI_UnCompress);
}

TEST_F(UT_MainWindow, test_handleJobCancelFinished_010)
{
    m_tester->handleJobCancelFinished(ArchiveJob::JT_Convert);
    EXPECT_EQ(m_tester->m_ePageID, PI_UnCompress);
}

TEST_F(UT_MainWindow, test_handleJobErrorFinished)
{
    m_tester->m_iCompressedWatchTimerID = 1000;
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Create, ET_InsufficientDiskSpace);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Create, ET_LongNameError);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Create, ET_FileWriteError);

    m_tester->m_eStartupType = ST_Extract;
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Add, ET_WrongPassword);
    EXPECT_EQ(m_tester->m_ePageID, PI_UnCompress);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Add, ET_FileWriteError);
    EXPECT_EQ(m_tester->m_ePageID, PI_UnCompress);
    m_tester->m_eStartupType = ST_DragDropAdd;
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Add, ET_WrongPassword);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Add, ET_LongNameError);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Add, ET_FileWriteError);

    m_tester->handleJobErrorFinished(ArchiveJob::JT_Load, ET_ArchiveDamaged);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Load, ET_WrongPassword);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Load, ET_FileWriteError);

    m_tester->m_operationtype = Operation_SingleExtract;
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Extract, ET_FileWriteError);
    m_tester->m_operationtype = Operation_Extract;
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Extract, ET_ArchiveDamaged);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Extract, ET_WrongPassword);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Extract, ET_LongNameError);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Extract, ET_FileWriteError);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Extract, ET_MissingVolume);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Extract, ET_InsufficientDiskSpace);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Extract, ET_PluginError);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Extract, ET_NeedPassword);

    m_tester->handleJobErrorFinished(ArchiveJob::JT_Delete, ET_FileWriteError);
    EXPECT_EQ(m_tester->m_ePageID, PI_UnCompress);

    m_tester->handleJobErrorFinished(ArchiveJob::JT_BatchExtract, ET_ArchiveDamaged);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_BatchExtract, ET_WrongPassword);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_BatchExtract, ET_LongNameError);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_BatchExtract, ET_FileWriteError);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_BatchExtract, ET_InsufficientDiskSpace);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_BatchExtract, ET_PluginError);
    m_tester->handleJobErrorFinished(ArchiveJob::JT_BatchExtract, ET_NeedPassword);

    m_tester->m_operationtype = Operation_TempExtract_Open;
    m_tester->handleJobErrorFinished(ArchiveJob::JT_Open, ET_WrongPassword);
    EXPECT_EQ(m_tester->m_ePageID, PI_UnCompress);
}

TEST_F(UT_MainWindow, test_addFiles2Archive)
{
    Stub stub;
    CommonStub::stub_QThread_start(stub);
    m_tester->addFiles2Archive(QStringList() << "1.txt");
    EXPECT_EQ(m_tester->m_ePageID, PI_AddCompressProgress);
}

TEST_F(UT_MainWindow, test_resetMainwindow)
{
    m_tester->resetMainwindow();
    EXPECT_EQ(m_tester->m_eStartupType, StartupType::ST_Normal);
}

TEST_F(UT_MainWindow, test_deleteWhenJobFinish_001)
{
//    Stub stub;
//    QFileInfoStub::stub_QFileInfo_exists(stub, true);
//    QFileInfoStub::stub_QFileInfo_isDir(stub, true);
//    m_tester->m_eStartupType = StartupType::ST_Compresstozip7z;
//    m_tester->m_stCompressParameter.listCompressFiles << "1/";
//    m_tester->deleteWhenJobFinish(ArchiveJob::JT_Create);
}

TEST_F(UT_MainWindow, test_deleteWhenJobFinish_002)
{
//    Stub stub;
//    QFileInfoStub::stub_QFileInfo_exists(stub, true);
//    QFileInfoStub::stub_QFileInfo_isDir(stub, false);
//    m_tester->m_eStartupType = StartupType::ST_Compresstozip7z;
//    m_tester->m_stCompressParameter.listCompressFiles << "1.txt";
//    m_tester->deleteWhenJobFinish(ArchiveJob::JT_Create);
}

TEST_F(UT_MainWindow, test_deleteWhenJobFinish_003)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_exists(stub, true);
    QFileInfoStub::stub_QFileInfo_isDir(stub, false);
    m_tester->m_stUnCompressParameter.listBatchFiles << "1.txt";
    m_tester->m_stUnCompressParameter.strFullPath = "1.txt";
    m_tester->m_eStartupType = StartupType::ST_ExtractHere;
    m_tester->m_stUnCompressParameter.bBatch = true;
    m_tester->deleteWhenJobFinish(ArchiveJob::JT_Extract);
}

TEST_F(UT_MainWindow, test_deleteWhenJobFinish_004)
{
    m_tester->m_stUnCompressParameter.listBatchFiles << "1.txt";
    m_tester->m_stUnCompressParameter.strFullPath = "1.txt";
    m_tester->m_eStartupType = StartupType::ST_ExtractHere;
    m_tester->m_stUnCompressParameter.bBatch = false;
    m_tester->deleteWhenJobFinish(ArchiveJob::JT_Extract);
}

TEST_F(UT_MainWindow, test_ConstructAddOptions)
{
    Stub stub;
    CommonStub::stub_QThreadPool_waitForDone(stub);
    QFileInfoStub::stub_QFileInfo_isDir(stub, false);

    m_tester->ConstructAddOptions(QStringList() << "1.txt");
    EXPECT_EQ(m_tester->m_stUpdateOptions.listEntry.count(), 1);
}

TEST_F(UT_MainWindow, test_ConstructAddOptionsByThread)
{
    Stub stub;
    QDirStub::stub_QDir_exists(stub, true);
    QFileInfoList listInfo = QFileInfoList() << QFileInfo("1.txt");
    QDirStub::stub_QDir_entryInfoList(stub, listInfo);
    QFileInfoStub::stub_QFileInfo_isDir(stub, false);

    m_tester->ConstructAddOptionsByThread("/home/Desktop");
    EXPECT_EQ(m_tester->m_stUpdateOptions.listEntry.count(), 1);
}

TEST_F(UT_MainWindow, test_showSuccessInfo_001)
{
    m_tester->showSuccessInfo(SI_Compress);
    EXPECT_EQ(m_tester->m_pSuccessPage->m_pSuccessLbl->text(), "Compression successful");
}

TEST_F(UT_MainWindow, test_showSuccessInfo_002)
{
    m_tester->showSuccessInfo(SI_UnCompress);
    EXPECT_EQ(m_tester->m_pSuccessPage->m_pSuccessLbl->text(), "Extraction successful");
}

TEST_F(UT_MainWindow, test_showSuccessInfo_003)
{
    m_tester->showSuccessInfo(SI_Convert);
    EXPECT_EQ(m_tester->m_pSuccessPage->m_pSuccessLbl->text(), "Conversion successful");
}

TEST_F(UT_MainWindow, test_showErrorMessage_001)
{
    m_tester->showErrorMessage(FI_Compress, EI_NoPlugin);
    EXPECT_EQ(m_tester->m_pFailurePage->m_pDetailLbl->text(), "Plugin error");
}

TEST_F(UT_MainWindow, test_showErrorMessage_002)
{
    m_tester->showErrorMessage(FI_Compress, EI_CreatArchiveFailed);
    EXPECT_EQ(m_tester->m_pFailurePage->m_pDetailLbl->text(), "Failed to create file");
}

TEST_F(UT_MainWindow, test_showErrorMessage_003)
{
    m_tester->showErrorMessage(FI_Compress, EI_InsufficientDiskSpace);
    EXPECT_EQ(m_tester->m_pFailurePage->m_pDetailLbl->text(), "Insufficient disk space");
}

TEST_F(UT_MainWindow, test_showErrorMessage_004)
{
    m_tester->showErrorMessage(FI_Load, EI_NoPlugin);
    EXPECT_EQ(m_tester->m_pFailurePage->m_pDetailLbl->text(), "Plugin error");
}

TEST_F(UT_MainWindow, test_showErrorMessage_005)
{
    m_tester->showErrorMessage(FI_Load, EI_ArchiveDamaged);
    EXPECT_EQ(m_tester->m_pFailurePage->m_pDetailLbl->text(), "The archive is damaged");
}

TEST_F(UT_MainWindow, test_showErrorMessage_006)
{
    m_tester->showErrorMessage(FI_Load, EI_WrongPassword);
    EXPECT_EQ(m_tester->m_pFailurePage->m_pDetailLbl->text(), "Wrong password");
}

TEST_F(UT_MainWindow, test_showErrorMessage_007)
{
    m_tester->showErrorMessage(FI_Load, EI_ArchiveMissingVolume);
    EXPECT_EQ(m_tester->m_pFailurePage->m_pDetailLbl->text(), "Some volumes are missing");
}

TEST_F(UT_MainWindow, test_showErrorMessage_008)
{
    m_tester->showErrorMessage(FI_Uncompress, EI_NoPlugin);
    EXPECT_EQ(m_tester->m_pFailurePage->m_pDetailLbl->text(), "Plugin error");
}

TEST_F(UT_MainWindow, test_showErrorMessage_009)
{
    m_tester->showErrorMessage(FI_Uncompress, EI_ArchiveDamaged);
    EXPECT_EQ(m_tester->m_pFailurePage->m_pDetailLbl->text(), "The archive is damaged");
}

TEST_F(UT_MainWindow, test_showErrorMessage_010)
{
    m_tester->showErrorMessage(FI_Uncompress, EI_ArchiveMissingVolume);
    EXPECT_EQ(m_tester->m_pFailurePage->m_pDetailLbl->text(), "Some volumes are missing");
}

TEST_F(UT_MainWindow, test_showErrorMessage_011)
{
    m_tester->showErrorMessage(FI_Uncompress, EI_WrongPassword);
    EXPECT_EQ(m_tester->m_pFailurePage->m_pDetailLbl->text(), "Wrong password, please retry");
}

TEST_F(UT_MainWindow, test_showErrorMessage_012)
{
    m_tester->showErrorMessage(FI_Uncompress, EI_LongFileName);
    EXPECT_EQ(m_tester->m_pFailurePage->m_pDetailLbl->text(), "The file name is too long. Keep the name within 60 characters please.");
}

TEST_F(UT_MainWindow, test_showErrorMessage_013)
{
    m_tester->showErrorMessage(FI_Uncompress, EI_CreatFileFailed);
    EXPECT_EQ(m_tester->m_pFailurePage->m_pDetailLbl->text().contains("Failed to create"), true);
}

TEST_F(UT_MainWindow, test_showErrorMessage_014)
{
    m_tester->showErrorMessage(FI_Uncompress, EI_ArchiveNoData);
    EXPECT_EQ(m_tester->m_pFailurePage->m_pDetailLbl->text(), "No data in it");
}

TEST_F(UT_MainWindow, test_showErrorMessage_015)
{
    m_tester->showErrorMessage(FI_Uncompress, EI_InsufficientDiskSpace);
    EXPECT_EQ(m_tester->m_pFailurePage->m_pDetailLbl->text(), "Insufficient disk space");
}

TEST_F(UT_MainWindow, test_getConfigWinSize)
{
    EXPECT_NE(m_tester->getConfigWinSize().width(), 0);
}

TEST_F(UT_MainWindow, testsaveConfigWinSize)
{
    m_tester->saveConfigWinSize(800, 600);
    EXPECT_EQ(m_tester->m_pSettings->value(MAINWINDOW_WIDTH_NAME).toInt(), 800);
    EXPECT_EQ(m_tester->m_pSettings->value(MAINWINDOW_HEIGHT_NAME).toInt(), 600);
}

TEST_F(UT_MainWindow, test_convertArchive)
{
    Stub stub;
    ArchiveManagerStub::stub_ArchiveManager_convertArchive(stub, true);
    m_tester->convertArchive("zip");
    EXPECT_EQ(m_tester->m_operationtype, Operation_CONVERT);
    EXPECT_EQ(m_tester->m_strFinalConvertFile.contains("zip"), true);
    EXPECT_EQ(m_tester->m_ePageID, PI_ConvertProgress);
}

TEST_F(UT_MainWindow, test_updateArchiveComment)
{
    Stub stub;
    ArchiveManagerStub::stub_ArchiveManager_updateArchiveComment(stub, true);
    m_tester->updateArchiveComment();
    EXPECT_EQ(m_tester->m_operationtype, Operation_Update_Comment);
}

TEST_F(UT_MainWindow, test_addArchiveComment)
{
    Stub stub;
    ArchiveManagerStub::stub_ArchiveManager_updateArchiveComment(stub, true);
    m_tester->m_pCompressSettingPage->m_pCommentEdt->setPlainText("123456");
    m_tester->addArchiveComment();
    EXPECT_EQ(m_tester->m_operationtype, Operation_Add_Comment);
    EXPECT_EQ(m_tester->m_ePageID, PI_CommentProgress);
}

TEST_F(UT_MainWindow, test_watcherArchiveFile)
{
    m_tester->watcherArchiveFile("1.txt");
    EXPECT_NE(m_tester->m_pFileWatcher, nullptr);
}

TEST_F(UT_MainWindow, test_creatShorcutJson)
{
    EXPECT_EQ(m_tester->creatShorcutJson().size(), 1);
}

TEST_F(UT_MainWindow, test_handleArguments_Open_001)
{
    Stub stub;
    stub.set(ADDR(MainWindow, loadArchive), loadArchive_stub);

    EXPECT_EQ(m_tester->handleArguments_Open(QStringList()), false);
}

TEST_F(UT_MainWindow, test_handleArguments_Open_002)
{
    Stub stub;
    stub.set(ADDR(MainWindow, loadArchive), loadArchive_stub);

    EXPECT_EQ(m_tester->handleArguments_Open(QStringList() << "1.zip"), true);
}

TEST_F(UT_MainWindow, test_handleArguments_RightMenu_001)
{
    Stub stub;
    CommonStub::stub_QThread_start(stub);
    DFileDialogStub::stub_DFileDialog_exec(stub, 1);
    ArchiveManagerStub::stub_ArchiveManager_batchExtractFiles(stub, true);
    ArchiveManagerStub::stub_ArchiveManager_extractFiles(stub, true);
    EXPECT_EQ(m_tester->handleArguments_RightMenu(QStringList() << "1.zip"), false);
}

TEST_F(UT_MainWindow, test_handleArguments_RightMenu_002)
{
    Stub stub;
    CommonStub::stub_QThread_start(stub);
    DFileDialogStub::stub_DFileDialog_exec(stub, 1);
    ArchiveManagerStub::stub_ArchiveManager_batchExtractFiles(stub, true);
    ArchiveManagerStub::stub_ArchiveManager_extractFiles(stub, true);
    EXPECT_EQ(m_tester->handleArguments_RightMenu(QStringList() << "1.txt" << "compress"), true);
    EXPECT_EQ(m_tester->m_eStartupType, StartupType::ST_Compress);
    EXPECT_EQ(m_tester->m_ePageID, PI_CompressSetting);
}

TEST_F(UT_MainWindow, test_handleArguments_RightMenu_003)
{
    Stub stub;
    CommonStub::stub_QThread_start(stub);
    DFileDialogStub::stub_DFileDialog_exec(stub, 1);
    ArchiveManagerStub::stub_ArchiveManager_batchExtractFiles(stub, true);
    ArchiveManagerStub::stub_ArchiveManager_extractFiles(stub, true);
    stub.set(ADDR(MainWindow, loadArchive), loadArchive_stub);
    EXPECT_EQ(m_tester->handleArguments_RightMenu(QStringList() << "1.zip" << "extract"), true);
    EXPECT_EQ(m_tester->m_eStartupType, StartupType::ST_Extract);
}

TEST_F(UT_MainWindow, test_handleArguments_RightMenu_004)
{
    Stub stub;
    CommonStub::stub_QThread_start(stub);
    DFileDialogStub::stub_DFileDialog_exec(stub, 1);
    ArchiveManagerStub::stub_ArchiveManager_batchExtractFiles(stub, true);
    ArchiveManagerStub::stub_ArchiveManager_extractFiles(stub, true);
    stub.set(ADDR(MainWindow, loadArchive), loadArchive_stub);
    EXPECT_EQ(m_tester->handleArguments_RightMenu(QStringList() << "1.zip" << "2.zip" << "extract"), true);
    EXPECT_EQ(m_tester->m_eStartupType, StartupType::ST_Extract);
    EXPECT_EQ(m_tester->m_stUnCompressParameter.bBatch, true);
    EXPECT_EQ(m_tester->m_stUnCompressParameter.listBatchFiles, QStringList() << "1.zip" << "2.zip");
    EXPECT_EQ(m_tester->m_operationtype, Operation_Extract);
}

TEST_F(UT_MainWindow, test_handleArguments_RightMenu_005)
{
    Stub stub;
    CommonStub::stub_QThread_start(stub);
    DFileDialogStub::stub_DFileDialog_exec(stub, 1);
    ArchiveManagerStub::stub_ArchiveManager_batchExtractFiles(stub, true);
    ArchiveManagerStub::stub_ArchiveManager_extractFiles(stub, true);
    stub.set(ADDR(MainWindow, loadArchive), loadArchive_stub);
    EXPECT_EQ(m_tester->handleArguments_RightMenu(QStringList() << "1.txt" << "compress_to_7z"), true);
    EXPECT_EQ(m_tester->m_eStartupType, StartupType::ST_Compresstozip7z);
}

TEST_F(UT_MainWindow, test_handleArguments_RightMenu_006)
{
    Stub stub;
    CommonStub::stub_QThread_start(stub);
    DFileDialogStub::stub_DFileDialog_exec(stub, 1);
    ArchiveManagerStub::stub_ArchiveManager_batchExtractFiles(stub, true);
    ArchiveManagerStub::stub_ArchiveManager_extractFiles(stub, true);
    stub.set(ADDR(MainWindow, loadArchive), loadArchive_stub);
    EXPECT_EQ(m_tester->handleArguments_RightMenu(QStringList() << "1.zip" << "compress_to_7z"), true);
}

TEST_F(UT_MainWindow, test_handleArguments_RightMenu_007)
{
    Stub stub;
    CommonStub::stub_QThread_start(stub);
    DFileDialogStub::stub_DFileDialog_exec(stub, 1);
    ArchiveManagerStub::stub_ArchiveManager_batchExtractFiles(stub, true);
    ArchiveManagerStub::stub_ArchiveManager_extractFiles(stub, true);
    stub.set(ADDR(MainWindow, loadArchive), loadArchive_stub);
    EXPECT_EQ(m_tester->handleArguments_RightMenu(QStringList() << "1.zip" << "2.zip" << "compress_to_7z"), true);
    EXPECT_EQ(m_tester->m_operationtype, Operation_Create);
    EXPECT_EQ(m_tester->m_ePageID, PI_CompressProgress);
}

TEST_F(UT_MainWindow, test_handleArguments_RightMenu_008)
{
    Stub stub;
    CommonStub::stub_QThread_start(stub);
    DFileDialogStub::stub_DFileDialog_exec(stub, 1);
    ArchiveManagerStub::stub_ArchiveManager_batchExtractFiles(stub, true);
    ArchiveManagerStub::stub_ArchiveManager_extractFiles(stub, true);
    stub.set(ADDR(MainWindow, loadArchive), loadArchive_stub);
    EXPECT_EQ(m_tester->handleArguments_RightMenu(QStringList() << "1.zip" << "extract_here"), true);
    EXPECT_EQ(m_tester->m_eStartupType, StartupType::ST_ExtractHere);
}

TEST_F(UT_MainWindow, test_handleArguments_RightMenu_009)
{
    Stub stub;
    CommonStub::stub_QThread_start(stub);
    DFileDialogStub::stub_DFileDialog_exec(stub, 1);
    ArchiveManagerStub::stub_ArchiveManager_batchExtractFiles(stub, true);
    ArchiveManagerStub::stub_ArchiveManager_extractFiles(stub, true);
    stub.set(ADDR(MainWindow, loadArchive), loadArchive_stub);
    EXPECT_EQ(m_tester->handleArguments_RightMenu(QStringList() << "1.zip" << "extract_to_specifypath"), true);
    EXPECT_EQ(m_tester->m_eStartupType, StartupType::ST_Extractto);
}

TEST_F(UT_MainWindow, test_handleArguments_Append_001)
{
    Stub stub;
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    CommonStub::stub_QThread_start(stub);
    PluginManagerStub::stub_PluginManager_supportedWriteMimeTypes(stub);

    EXPECT_EQ(m_tester->handleArguments_Append(QStringList() << "1.zip" << "dragdropadd"), false);
}

TEST_F(UT_MainWindow, test_handleArguments_Append_002)
{
    Stub stub;
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    CommonStub::stub_QThread_start(stub);
    PluginManagerStub::stub_PluginManager_supportedWriteMimeTypes(stub);
    EXPECT_EQ(m_tester->handleArguments_Append(QStringList() << "1.zip" << "1.zip" << "dragdropadd"), false);
    EXPECT_EQ(m_tester->m_stUnCompressParameter.bCommentModifiable, false);
    EXPECT_EQ(m_tester->m_stUnCompressParameter.bMultiplePassword, false);
}

TEST_F(UT_MainWindow, test_handleArguments_Append_003)
{
    Stub stub;
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    CommonStub::stub_QThread_start(stub);
    PluginManagerStub::stub_PluginManager_supportedWriteMimeTypes(stub);
    QFileInfoStub::stub_QFileInfo_isWritable(stub, false);
    EXPECT_EQ(m_tester->handleArguments_Append(QStringList() << "1.zip" << "1.zip" << "dragdropadd"), false);
    EXPECT_EQ(m_tester->m_stUnCompressParameter.bCommentModifiable, false);
    EXPECT_EQ(m_tester->m_stUnCompressParameter.bMultiplePassword, false);
}

TEST_F(UT_MainWindow, test_handleArguments_Append_004)
{
    Stub stub;
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    CommonStub::stub_QThread_start(stub);
    PluginManagerStub::stub_PluginManager_supportedWriteMimeTypes(stub);
    QFileInfoStub::stub_QFileInfo_isWritable(stub, true);
    EXPECT_EQ(m_tester->handleArguments_Append(QStringList() << "1.zip" << "1.txt" << "dragdropadd"), true);
    EXPECT_EQ(m_tester->m_stUnCompressParameter.bCommentModifiable, true);
    EXPECT_EQ(m_tester->m_stUnCompressParameter.bMultiplePassword, false);
}

TEST_F(UT_MainWindow, test_rightExtract2Path_001)
{
    Stub stub;
    ArchiveManagerStub::stub_ArchiveManager_extractFiles(stub, true);
    ArchiveManagerStub::stub_ArchiveManager_batchExtractFiles(stub, true);

    m_tester->rightExtract2Path(ST_Extract, QStringList());
}

TEST_F(UT_MainWindow, test_rightExtract2Path_002)
{
    Stub stub;
    ArchiveManagerStub::stub_ArchiveManager_extractFiles(stub, true);
    ArchiveManagerStub::stub_ArchiveManager_batchExtractFiles(stub, true);

    m_tester->m_eStartupType = StartupType::ST_ExtractHere;
    m_tester->rightExtract2Path(ST_Extract, QStringList() << "1.zip");
    EXPECT_EQ(m_tester->m_operationtype, Operation_NULL);
    EXPECT_EQ(m_tester->m_ePageID, PI_Failure);
}

TEST_F(UT_MainWindow, test_rightExtract2Path_003)
{
    Stub stub;
    ArchiveManagerStub::stub_ArchiveManager_extractFiles(stub, true);
    ArchiveManagerStub::stub_ArchiveManager_batchExtractFiles(stub, true);
    m_tester->m_eStartupType = StartupType::ST_Extract;
    m_tester->rightExtract2Path(ST_Extract, QStringList() << "1.zip");
    EXPECT_EQ(m_tester->m_operationtype, Operation_NULL);
    EXPECT_EQ(m_tester->m_ePageID, PI_Failure);
}

TEST_F(UT_MainWindow, test_rightExtract2Path_004)
{
    Stub stub;
    ArchiveManagerStub::stub_ArchiveManager_extractFiles(stub, true);
    ArchiveManagerStub::stub_ArchiveManager_batchExtractFiles(stub, true);
    m_tester->m_eStartupType = StartupType::ST_ExtractHere;
    m_tester->rightExtract2Path(ST_Extract, QStringList() << "1.zip" << "2.zip");
    EXPECT_EQ(m_tester->m_operationtype, Operation_Extract);
    EXPECT_EQ(m_tester->m_ePageID, PI_UnCompressProgress);
}

TEST_F(UT_MainWindow, test_showWarningDialog)
{
    Stub stub;
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    EXPECT_EQ(m_tester->showWarningDialog("", ""), 0);
}

TEST_F(UT_MainWindow, test_moveDialogToCenter)
{
    DDialog dialog(m_tester);
    m_tester->moveDialogToCenter(&dialog);
}

TEST_F(UT_MainWindow, test_delayQuitApp)
{
    m_tester->delayQuitApp();
}

TEST_F(UT_MainWindow, test_slotExtract2Path)
{
    Stub stub;
    ArchiveManagerStub::stub_ArchiveManager_extractFiles2Path(stub, false);
    QList<FileEntry> listEntry;
    FileEntry entry;
    entry.strFileName = "1.txt";
    listEntry << entry;
    m_tester->slotExtract2Path(listEntry, ExtractionOptions());
    EXPECT_EQ(m_tester->m_operationtype, Operation_NULL);
    EXPECT_EQ(m_tester->m_stUnCompressParameter.listExractFiles.count(), 1);
}

TEST_F(UT_MainWindow, test_slotDelFiles_001)
{
    Stub stub;
    ArchiveManagerStub::stub_ArchiveManager_deleteFiles(stub, true);
    QList<FileEntry> listEntry;
    FileEntry entry;
    entry.strFileName = "1.txt";
    listEntry << entry;
    m_tester->slotDelFiles(listEntry, 10);
    EXPECT_EQ(m_tester->m_operationtype, Operation_DELETE);
    EXPECT_EQ(m_tester->m_ePageID, PI_DeleteProgress);
}

TEST_F(UT_MainWindow, test_slotDelFiles_002)
{
    Stub stub;
    ArchiveManagerStub::stub_ArchiveManager_deleteFiles(stub, false);

    QList<FileEntry> listEntry;
    FileEntry entry;
    entry.strFileName = "1.txt";
    listEntry << entry;
    m_tester->slotDelFiles(listEntry, 10);
    EXPECT_EQ(m_tester->m_operationtype, Operation_NULL);
}

TEST_F(UT_MainWindow, test_slotRenameFiles_001)
{
    Stub stub;
    ArchiveManagerStub::stub_ArchiveManager_renameFiles(stub, true);
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strAlias = "3.txt";
    m_tester->slotRenameFile(entry, 10);
    EXPECT_EQ(m_tester->m_operationtype, Operation_NULL);
    EXPECT_EQ(m_tester->m_ePageID, PI_RenameProgress);
}

TEST_F(UT_MainWindow, test_slotRenameFiles_002)
{
    Stub stub;
    ArchiveManagerStub::stub_ArchiveManager_renameFiles(stub, false);

    FileEntry entry;
    entry.strFileName = "1.txt";
    m_tester->slotRenameFile(entry, 10);
    EXPECT_EQ(m_tester->m_operationtype, Operation_NULL);
}


TEST_F(UT_MainWindow, test_slotReceiveCurArchiveName)
{
    m_tester->slotReceiveCurArchiveName("1.zip");
    EXPECT_EQ(m_tester->m_pProgressPage->m_strArchiveName, "1.zip");
}

TEST_F(UT_MainWindow, test_slotOpenFile_001)
{
    Stub stub;
    ArchiveManagerStub::stub_ArchiveManager_openFile(stub, true);
    FileEntry entry;
    entry.strFileName = "1.txt";
    m_tester->slotOpenFile(entry, "");
    EXPECT_EQ(m_tester->m_operationtype, Operation_TempExtract_Open);
    EXPECT_EQ(m_tester->m_ePageID, PI_Loading);
}

TEST_F(UT_MainWindow, test_slotOpenFile_002)
{
    Stub stub;
    ArchiveManagerStub::stub_ArchiveManager_openFile(stub, false);
    FileEntry entry;
    entry.strFileName = "1.txt";
    m_tester->slotOpenFile(entry, "");
    EXPECT_EQ(m_tester->m_operationtype, Operation_NULL);
}

TEST_F(UT_MainWindow, test_slotOpenFileChanged)
{
    Stub stub;
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 1);
    m_tester->m_pOpenFileWatcher->m_mapFileHasModified["1.txt"] = false;
    m_tester->m_stUnCompressParameter.bModifiable = false;
    m_tester->slotOpenFileChanged("1.txt");
    EXPECT_NE(m_tester->m_pOpenFileWatcher, nullptr);
}

TEST_F(UT_MainWindow, test_slotPause)
{
    Stub stub;
    ArchiveManagerStub::stub_ArchiveManager_pauseOperation(stub, true);
    m_tester->slotPause();
}

TEST_F(UT_MainWindow, test_slotContinue)
{
    Stub stub;
    ArchiveManagerStub::stub_ArchiveManager_continueOperation(stub, true);
    m_tester->slotContinue();
}

TEST_F(UT_MainWindow, test_slotCancel)
{
    Stub stub;
    ArchiveManagerStub::stub_ArchiveManager_cancelOperation(stub, true);
    m_tester->slotCancel();
}

TEST_F(UT_MainWindow, test_slotAddFiles)
{
    m_tester->slotAddFiles(QStringList(), "");
}

TEST_F(UT_MainWindow, test_slotSuccessView_001)
{
    Stub stub;
    stub.set(ADDR(MainWindow, loadArchive), loadArchive_stub);
    CommonStub::stub_QThread_start(stub);

    m_tester->m_pSuccessPage->m_successInfoType = SI_Convert;
    m_tester->slotSuccessView();
}

TEST_F(UT_MainWindow, test_slotSuccessView_002)
{
    Stub stub;
    stub.set(ADDR(MainWindow, loadArchive), loadArchive_stub);
    CommonStub::stub_QThread_start(stub);

    if (m_tester->m_pDDesktopServicesThread == nullptr) {
        m_tester->m_pDDesktopServicesThread = new DDesktopServicesThread(m_tester);
    }

    m_tester->m_pSuccessPage->m_successInfoType = SI_UnCompress;
    m_tester->m_pDDesktopServicesThread->m_listFiles << "1.txt";
    m_tester->slotSuccessView();
}

TEST_F(UT_MainWindow, test_slotSuccessReturn_001)
{
    m_tester->m_pSuccessPage->m_successInfoType = SI_Compress;
    m_tester->slotSuccessReturn();
    EXPECT_EQ(m_tester->m_ePageID, PI_Home);
}

TEST_F(UT_MainWindow, test_slotSuccessReturn_002)
{
    m_tester->m_pSuccessPage->m_successInfoType = SI_UnCompress;
    m_tester->slotSuccessReturn();
    EXPECT_EQ(m_tester->m_ePageID, PI_Home);
}

TEST_F(UT_MainWindow, test_slotSuccessReturn_003)
{
    m_tester->m_pSuccessPage->m_successInfoType = SI_Convert;
    m_tester->slotSuccessReturn();
    EXPECT_EQ(m_tester->m_ePageID, PI_Home);
}

TEST_F(UT_MainWindow, test_slotFailureRetry_001)
{
    m_tester->m_pFailurePage->m_failureInfo = FI_Compress;
    m_tester->slotFailureRetry();
    EXPECT_EQ(m_tester->m_ePageID, PI_CompressSetting);
}

TEST_F(UT_MainWindow, test_slotFailureRetry_002)
{
    m_tester->m_pFailurePage->m_failureInfo = FI_Uncompress;
    m_tester->m_eStartupType = ST_Extract;
    m_tester->slotFailureRetry();
    EXPECT_EQ(m_tester->m_ePageID, PI_UnCompress);
}

TEST_F(UT_MainWindow, test_slotFailureRetry_003)
{
    m_tester->m_pFailurePage->m_failureInfo = FI_Convert;
    m_tester->slotFailureRetry();
    EXPECT_EQ(m_tester->m_ePageID, PI_UnCompress);
}

TEST_F(UT_MainWindow, test_slotFailureReturn_001)
{
    m_tester->m_pFailurePage->m_failureInfo = FI_Compress;
    m_tester->slotFailureReturn();
    EXPECT_EQ(m_tester->m_ePageID, PI_Home);
}

TEST_F(UT_MainWindow, test_slotFailureReturn_002)
{
    m_tester->m_pFailurePage->m_failureInfo = FI_Uncompress;
    m_tester->slotFailureReturn();
    EXPECT_EQ(m_tester->m_ePageID, PI_Home);
}

TEST_F(UT_MainWindow, test_slotFailureReturn_003)
{
    m_tester->m_pFailurePage->m_failureInfo = FI_Convert;
    m_tester->slotFailureReturn();
    EXPECT_EQ(m_tester->m_ePageID, PI_Home);
}

TEST_F(UT_MainWindow, test_slotTitleCommentButtonPressed_001)
{
    Stub stub;
    ArchiveManagerStub::stub_ArchiveManager_updateArchiveComment(stub, false);
    DataManager::get_instance().archiveData().strComment = "123";
    m_tester->m_isFirstViewComment = true;
    m_tester->m_ePageID = PI_UnCompress;
    CommonStub::stub_DDialog_exec(stub, 0);
    m_tester->slotTitleCommentButtonPressed();
    EXPECT_EQ(m_tester->m_comment, "123");
}

TEST_F(UT_MainWindow, test_slotThemeChanged_001)
{
    Stub stub;
    DGuiApplicationHelperStub::stub_DGuiApplicationHelper_themeType(stub, DGuiApplicationHelper::LightType);
    m_tester->slotThemeChanged();
    EXPECT_EQ(m_tester->m_pTitleWidget->m_pTitleCommentButton->iconSize(), QSize(15, 15));
}

TEST_F(UT_MainWindow, test_slotThemeChanged_002)
{
    Stub stub;
    DGuiApplicationHelperStub::stub_DGuiApplicationHelper_themeType(stub, DGuiApplicationHelper::DarkType);
    m_tester->slotThemeChanged();
    EXPECT_EQ(m_tester->m_pTitleWidget->m_pTitleCommentButton->iconSize(), QSize(15, 15));
}

TEST_F(UT_MainWindow, test_slotThemeChanged_003)
{
    Stub stub;
    DGuiApplicationHelperStub::stub_DGuiApplicationHelper_themeType(stub, DGuiApplicationHelper::UnknownType);
    m_tester->slotThemeChanged();
    EXPECT_EQ(m_tester->m_pTitleWidget->m_pTitleCommentButton->iconSize(), QSize(15, 15));
}

TEST_F(UT_MainWindow, test_slotShowShortcutTip)
{
    Stub stub;
    CommonStub::stub_QProcess_startDetached(stub);
    m_tester->slotShowShortcutTip();
}

TEST_F(UT_MainWindow, test_slotFinishCalculateSize_001)
{
    Stub stub;
    ArchiveManagerStub::stub_ArchiveManager_createArchive(stub, true);
    ArchiveManagerStub::stub_ArchiveManager_addFiles(stub, true);

    m_tester->m_eStartupType = StartupType::ST_Compresstozip7z;
    m_tester->slotFinishCalculateSize(10, "1.txt", QList<FileEntry>(), CompressOptions(), QList<FileEntry>());
    EXPECT_EQ(m_tester->m_stCompressParameter.qSize, 10);
    EXPECT_EQ(m_tester->m_mywork, nullptr);
}

TEST_F(UT_MainWindow, test_slotFinishCalculateSize_002)
{
    Stub stub;
    ArchiveManagerStub::stub_ArchiveManager_createArchive(stub, true);
    ArchiveManagerStub::stub_ArchiveManager_addFiles(stub, true);

    m_tester->m_eStartupType = StartupType::ST_Extract;
    m_tester->slotFinishCalculateSize(10, "1.txt", QList<FileEntry>(), CompressOptions(), QList<FileEntry>());
    EXPECT_EQ(m_tester->m_stUpdateOptions.qSize, 10);
    EXPECT_EQ(m_tester->m_stUpdateOptions.listEntry.count(), 0);
    EXPECT_EQ(m_tester->m_stUpdateOptions.eType, UpdateOptions::Add);
    EXPECT_EQ(m_tester->m_mywork, nullptr);
}

TEST_F(UT_MainWindow, test_slotCheckFinished_001)
{
    Stub stub;
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    m_tester->m_eStartupType = ST_Compresstozip7z;
    m_tester->slotCheckFinished("ddd", "hhh");
    EXPECT_EQ(m_tester->m_operationtype, Operation_NULL);
}

TEST_F(UT_MainWindow, test_slotCheckFinished_002)
{
    Stub stub;
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);

    m_tester->m_eStartupType = ST_Compress;
    m_tester->slotCheckFinished("ddd", "hhh");
    EXPECT_EQ(m_tester->m_operationtype, Operation_NULL);
    EXPECT_EQ(m_tester->m_ePageID, PI_UnCompress);
}
