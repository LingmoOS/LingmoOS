// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "compressview.h"
#include "treeheaderview.h"
#include "ut_commonstub.h"
#include "datamodel.h"

#include "gtest/src/stub.h"

#include <gtest/gtest.h>
#include <QTest>
#include <QAction>
#include <QFileSystemWatcher>

/*******************************函数打桩************************************/
// 对CompressView的handleDoubleClick进行打桩
void compressView_handleDoubleClick_stub(const QModelIndex &)
{
    return;
}
/*******************************函数打桩************************************/

class UT_StyleTreeViewDelegate : public ::testing::Test
{
public:
    UT_StyleTreeViewDelegate(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new StyleTreeViewDelegate;
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    StyleTreeViewDelegate *m_tester;
};

TEST_F(UT_StyleTreeViewDelegate, initTest)
{
    QStyleOptionViewItem option;
    QModelIndex index;
    EXPECT_EQ(m_tester->sizeHint(option, index).height(), 36);
}


class UT_CompressView : public ::testing::Test
{
public:
    UT_CompressView(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CompressView;
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CompressView *m_tester;
};

TEST_F(UT_CompressView, initTest)
{

}

TEST_F(UT_CompressView, test_getHeaderView)
{
    EXPECT_EQ(m_tester->getHeaderView(), m_tester->m_pHeaderView);
}

TEST_F(UT_CompressView, test_addCompressFiles_001)
{
    Stub stub;
    CustomDialogStub::stub_OverwriteQueryDialog_showDialog(stub);
    CustomDialogStub::stub_OverwriteQueryDialog_getApplyAll(stub, true);
    CustomDialogStub::stub_OverwriteQueryDialog_getDialogResult(stub, OR_Cancel);

    QStringList listFiles = QStringList() << "1.txt" << "2.txt";
    m_tester->m_listCompressFiles.clear();
    m_tester->m_listCompressFiles << "1.txt";
    m_tester->addCompressFiles(listFiles);

    EXPECT_EQ(m_tester->m_listCompressFiles == listFiles, true);
}

TEST_F(UT_CompressView, ttest_addCompressFiles_002)
{
    Stub stub;
    CustomDialogStub::stub_OverwriteQueryDialog_showDialog(stub);
    CustomDialogStub::stub_OverwriteQueryDialog_getApplyAll(stub, true);
    CustomDialogStub::stub_OverwriteQueryDialog_getDialogResult(stub, OR_Overwrite);

    QStringList listFiles = QStringList() << "1.txt" << "2.txt";
    m_tester->m_listCompressFiles.clear();
    m_tester->m_listCompressFiles << "1.txt";
    m_tester->addCompressFiles(listFiles);

    EXPECT_EQ(m_tester->m_listCompressFiles == listFiles, true);
}

TEST_F(UT_CompressView, test_getCompressFiles)
{
    QStringList listFiles = QStringList() << "1.txt" << "2.txt";
    m_tester->m_listCompressFiles.clear();
    m_tester->m_listCompressFiles << listFiles;

    EXPECT_EQ(m_tester->getCompressFiles(), listFiles);
}

TEST_F(UT_CompressView, test_refreshCompressedFiles)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_exists(stub, false);

    QStringList listFiles = QStringList() << "1.txt" << "2.txt";
    m_tester->m_listCompressFiles.clear();
    m_tester->m_listCompressFiles << listFiles;
    m_tester->m_listSelFiles << "2.txt";
    m_tester->refreshCompressedFiles(true, "1.txt");

    EXPECT_EQ(m_tester->m_listSelFiles.isEmpty(), true);
}

TEST_F(UT_CompressView, test_clear)
{
    m_tester->clear();

    EXPECT_EQ(m_tester->m_pFileWatcher->files().isEmpty(), true);
    EXPECT_EQ(m_tester->m_listCompressFiles.isEmpty(), true);
    EXPECT_EQ(m_tester->m_listEntry.isEmpty(), true);
    EXPECT_EQ(m_tester->m_iLevel, 0);
}

TEST_F(UT_CompressView, test_mouseDoubleClickEvent)
{
    typedef void (*fptr)(CompressView *, const QModelIndex &);
    fptr A_foo = (fptr)(&CompressView::handleDoubleClick);   //获取虚函数地址
    Stub stub;
    stub.set(A_foo, compressView_handleDoubleClick_stub);

    QPoint point(0, 40);
    QTest::mouseDClick(m_tester->viewport(), Qt::LeftButton, Qt::KeyboardModifiers(), point);
}

TEST_F(UT_CompressView, test_fileInfo2Entry_001)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_isDir(stub, true);
    QFileInfoStub::stub_QFileInfo_fileName(stub, "1.txt");

    QFileInfo fileInfo;
    FileEntry entry = m_tester->fileInfo2Entry(fileInfo);
    EXPECT_EQ(entry.strFileName, "1.txt");
}

TEST_F(UT_CompressView, test_fileInfo2Entry_002)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_isDir(stub, false);
    QFileInfoStub::stub_QFileInfo_fileName(stub, "2.txt");

    QFileInfo fileInfo;
    FileEntry entry = m_tester->fileInfo2Entry(fileInfo);
    EXPECT_EQ(entry.strFileName, "2.txt");
}

TEST_F(UT_CompressView, test_handleDoubleClick_001)
{
    m_tester->m_iLevel = 0;
    QStringList listFiles = QStringList() << "/home/Desktop/compress/";


    Stub stub;
    QFileInfoStub::stub_QFileInfo_isDir(stub, true);
    m_tester->addCompressFiles(listFiles);
    QModelIndex index = m_tester->model()->index(0, 0);
    CommonStub::stub_ProcessOpenThread_start(stub);
    CustomDialogStub::stub_OpenWithDialog_openWithProgram(stub);

    m_tester->handleDoubleClick(index);
    EXPECT_EQ(m_tester->m_iLevel, 1);
}

TEST_F(UT_CompressView, test_handleDoubleClick_002)
{
    m_tester->m_iLevel = 0;
    QStringList listFiles = QStringList() << "/home/Desktop/compress/";


    Stub stub;
    QFileInfoStub::stub_QFileInfo_isDir(stub, false);
    m_tester->addCompressFiles(listFiles);
    QModelIndex index = m_tester->model()->index(0, 0);
    CommonStub::stub_ProcessOpenThread_start(stub);
    CustomDialogStub::stub_OpenWithDialog_openWithProgram(stub);

    m_tester->handleDoubleClick(index);
    EXPECT_EQ(m_tester->m_iLevel, 0);
}

TEST_F(UT_CompressView, test_getCurrentDirFiles)
{
    QFileInfo fi1;
    fi1.setFile("1.txt");
    QFileInfoList filist{fi1};

    Stub stub;
    QDirStub::stub_QDir_entryInfoList(stub, filist);

    QList<FileEntry> listEntry = m_tester->getCurrentDirFiles();

    if (listEntry.count() > 0)
        EXPECT_EQ(listEntry[0].strFileName, "1.txt");
}

TEST_F(UT_CompressView, test_handleLevelChanged)
{
    m_tester->m_iLevel = 0;
    m_tester->handleLevelChanged();
    EXPECT_EQ(m_tester->acceptDrops(), true);
}

TEST_F(UT_CompressView, test_getPrePathByLevel_001)
{
    m_tester->m_iLevel = 0;
    m_tester->getPrePathByLevel("");
}

TEST_F(UT_CompressView, test_getPrePathByLevel_002)
{
    m_tester->m_iLevel = 1;
    QString strResult = m_tester->getPrePathByLevel("/home/Desktop");
    EXPECT_EQ(strResult, "Desktop");
}

TEST_F(UT_CompressView, test_getPrePathByLevel_003)
{
    m_tester->m_iLevel = 0;
    m_tester->refreshDataByCurrentPath();
    EXPECT_EQ(m_tester->m_pHeaderView->getpreLbl()->isVisible(), false);
}

TEST_F(UT_CompressView, test_getPrePathByLevel_004)
{
    Stub stub;
    CommonStub::stub_QWidget_isVisible(stub, true);
    m_tester->m_iLevel = 1;
    m_tester->m_strCurrentPath = "/home/Desktop";
    m_tester->refreshDataByCurrentPath();
    EXPECT_EQ(m_tester->m_pHeaderView->getpreLbl()->isVisible(), true);
}

TEST_F(UT_CompressView, test_slotShowRightMenu_001)
{
    Stub stub;
    CommonStub::stub_QTreeView_indexAt(stub);
    CommonStub::stub_QMenu_exec(stub);

    m_tester->m_stRightEntry.isDirectory = true;
    QStringList listFiles = QStringList() << "/home/Desktop/compress";
    m_tester->addCompressFiles(listFiles);
    QPoint p;
    m_tester->slotShowRightMenu(p);
    m_tester->handleDoubleClick(m_tester->model()->index(0, 0));
    EXPECT_EQ(m_tester->m_stRightEntry.strFileName, "compress");
}

TEST_F(UT_CompressView, test_slotShowRightMenu_002)
{
    Stub stub;
    CommonStub::stub_QTreeView_indexAt(stub);
    CommonStub::stub_QMenu_exec(stub);

    m_tester->m_stRightEntry.isDirectory = false;
    QStringList listFiles = QStringList() << "/home/Desktop/compress.txt";
    m_tester->addCompressFiles(listFiles);
    QPoint p;
    m_tester->slotShowRightMenu(p);
    m_tester->handleDoubleClick(m_tester->model()->index(0, 0));
    EXPECT_EQ(m_tester->m_stRightEntry.strFileName, "compress.txt");
}

TEST_F(UT_CompressView, test_slotDeleteFile_001)
{
    QStringList listFiles = QStringList() << "/home/Desktop/compress";
    m_tester->addCompressFiles(listFiles);
    m_tester->slotDeleteFile();
}

TEST_F(UT_CompressView, test_slotDeleteFile_002)
{
    Stub stub;
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 1);

    QStringList listFiles = QStringList() << "/home/Desktop/compress.txt";
    m_tester->addCompressFiles(listFiles);
    m_tester->m_iLevel = 1;
    m_tester->slotDeleteFile();
}

TEST_F(UT_CompressView, test_slotRenameFile_001)
{
    Stub stub;
    CustomDialogStub::stub_RenameDialog_showDialog(stub, 1);
    QStringList listFiles = QStringList() << "/home/Desktop/compress";
    m_tester->addCompressFiles(listFiles);
    m_tester->slotRenameFile();
}

TEST_F(UT_CompressView, test_slotRenameFile_002)
{
    Stub stub;
    CustomDialogStub::stub_RenameDialog_showDialog(stub, 1);

    QStringList listFiles = QStringList() << "/home/Desktop/compress.txt";
    m_tester->addCompressFiles(listFiles);
    m_tester->m_iLevel = 1;
    m_tester->slotRenameFile();
}

TEST_F(UT_CompressView, test_slotDirChanged)
{
    m_tester->slotDirChanged();
    EXPECT_EQ(m_tester->m_pModel->m_listEntry.isEmpty(), false);
}

TEST_F(UT_CompressView, test_slotDragFiles_001)
{
    Stub stub;
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 1);
    CommonStub::stub_UiTools_isArchiveFile(stub, true);
    m_tester->m_listCompressFiles << "1.txt";
    m_tester->slotDragFiles(QStringList() << "1.zip");
    EXPECT_EQ(m_tester->m_listSelFiles.isEmpty(), true);
}

TEST_F(UT_CompressView, test_slotDragFiles_002)
{
    Stub stub;
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 2);
    CommonStub::stub_UiTools_isArchiveFile(stub, true);
    CommonStub::stub_ProcessOpenThread_start(stub);
    m_tester->m_listCompressFiles << "1.txt";
    m_tester->slotDragFiles(QStringList() << "1.zip");
}

TEST_F(UT_CompressView, test_slotDragFiles_003)
{
    Stub stub;
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 1);
    CommonStub::stub_UiTools_isArchiveFile(stub, false);
    m_tester->slotDragFiles(QStringList() << "1.txt");
    EXPECT_EQ(m_tester->m_listSelFiles.isEmpty(), true);
}

TEST_F(UT_CompressView, test_slotOpenStyleClicked_001)
{
    QAction *pAction = nullptr;
    Stub stub;
    CommonStub::stub_QObject_sender(stub, pAction);
    CustomDialogStub::stub_OpenWithDialog_openWithProgram(stub);
    m_tester->slotOpenStyleClicked();
}

TEST_F(UT_CompressView, test_slotOpenStyleClicked_002)
{
    QAction *pAction = new QAction(m_tester);
    pAction->setText(QObject::tr("Select default program"));
    Stub stub;
    CommonStub::stub_QObject_sender(stub, pAction);
    CustomDialogStub::stub_OpenWithDialog_showOpenWithDialog(stub, "");
    CustomDialogStub::stub_OpenWithDialog_openWithProgram(stub);
    m_tester->slotOpenStyleClicked();
}

TEST_F(UT_CompressView, test_slotOpenStyleClicked_003)
{
    QAction *pAction = new QAction(m_tester);
    pAction->setText(QObject::tr("other"));
    Stub stub;
    CommonStub::stub_QObject_sender(stub, pAction);
    CustomDialogStub::stub_OpenWithDialog_showOpenWithDialog(stub, "");
    CustomDialogStub::stub_OpenWithDialog_openWithProgram(stub);
    m_tester->slotOpenStyleClicked();
}

TEST_F(UT_CompressView, test_slotPreClicked_001)
{
    m_tester->m_iLevel = 1;
    m_tester->m_vPre << "compress";
    m_tester->slotPreClicked();
    EXPECT_EQ(m_tester->m_iLevel, 0);
}

TEST_F(UT_CompressView, test_slotPreClicked_002)
{
    m_tester->m_iLevel = 2;
    m_tester->m_vPre << "compress";
    m_tester->slotPreClicked();
    EXPECT_EQ(m_tester->m_iLevel, 1);
}

TEST_F(UT_CompressView, test_mouseReleaseEvent)
{
    QMouseEvent *event = new QMouseEvent(QEvent::MouseButtonRelease, QPointF(50, 50), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_tester->mouseReleaseEvent(event);
    delete event;
    EXPECT_EQ(m_tester->m_isPressed, false);
}

TEST_F(UT_CompressView, test_mouseMoveEvent)
{
    m_tester->m_isPressed = true;
    m_tester->m_lastTouchBeginPos = QPoint(0, 0);
    QMouseEvent *event = new QMouseEvent(QEvent::MouseMove, QPointF(50, 50), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_tester->mouseMoveEvent(event);
    delete event;
    EXPECT_EQ(m_tester->m_lastTouchBeginPos, QPointF(50, 50));
}
