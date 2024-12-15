// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "uncompressview.h"
#include "datamanager.h"
#include "popupdialog.h"
#include "datamodel.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"

#include <DFileDragServer>
#include <DFileDrag>

#include <QTest>
#include <QMimeData>
#include <QAction>

#include <gtest/gtest.h>

DWIDGET_USE_NAMESPACE

/*******************************函数打桩************************************/
// 对UnCompressView的slotOpen进行打桩
void unCompressView_slotOpen_stub()
{
    return;
}

// 对UnCompressView的slotOpen进行打桩
void unCompressView_handleDoubleClick_stub(const QModelIndex &)
{
    return;
}

// 对QItemSelectionModel的selectedRows进行打桩
QModelIndexList qItemSelectionModel_selectedRows_stub(int &)
{
    return QModelIndexList() << QModelIndex();
}

// 对slotDeleteFile进行打桩
void slotDeleteFile_stub(const QModelIndex &)
{
    return;
}

// 对slotShowRightMenu进行打桩
void slotShowRightMenu_stub(const QPoint &)
{
    return;
}

// handleDoubleClick进行打桩
void handleDoubleClick_stub(const QModelIndex &)
{
    return;
}
/*******************************函数打桩************************************/


class UT_UnCompressView : public ::testing::Test
{
public:
    UT_UnCompressView(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new UnCompressView;
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    UnCompressView *m_tester;
};

TEST_F(UT_UnCompressView, initTest)
{

}

TEST_F(UT_UnCompressView, test_refreshArchiveData)
{
    m_tester->m_iLevel = 1;

    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1/";
    entry.strFullPath = "1/";
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1/1.txt";
    entry.qSize = 10;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();
    EXPECT_EQ(m_tester->m_iLevel, 0);
}

TEST_F(UT_UnCompressView, test_setArchivePath)
{
    m_tester->setArchivePath("1/2/3");
    EXPECT_EQ(m_tester->m_strArchive, "1/2/3");
    EXPECT_EQ(m_tester->m_strArchivePath, "1/2");
}

TEST_F(UT_UnCompressView, test_setDefaultUncompressPath)
{
    m_tester->setDefaultUncompressPath("/home/Desktop");
    EXPECT_EQ(m_tester->m_strUnCompressPath, "/home/Desktop");
}

TEST_F(UT_UnCompressView, test_mousePressEvent)
{
    QMouseEvent *event = new QMouseEvent(QEvent::MouseButtonRelease, QPointF(50, 50), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_tester->mousePressEvent(event);
    delete event;
    EXPECT_EQ(m_tester->m_dragPos, QPointF(50, 50));
}

TEST_F(UT_UnCompressView, test_mouseMoveEvent)
{
    m_tester->m_isPressed = true;
    QMouseEvent *event = new QMouseEvent(QEvent::MouseMove, QPointF(50, 50), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_tester->m_lastTouchBeginPos = QPointF(200, 100);
    m_tester->mouseMoveEvent(event);
    delete event;

    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1/";
    entry.strFullPath = "1/";
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();

    m_tester->selectAll();

    m_tester->m_isPressed = false;
    event = new QMouseEvent(QEvent::MouseMove, QPointF(50, 50), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_tester->m_lastTouchBeginPos = QPointF(200, 100);
    m_tester->mouseMoveEvent(event);
    delete event;
}

TEST_F(UT_UnCompressView, test_clearDragData)
{
    m_tester->m_pFileDragServer = new DFileDragServer(m_tester);
    m_tester->m_pDrag = new DFileDrag(m_tester, m_tester->m_pFileDragServer);
    m_tester->clearDragData();
    EXPECT_EQ(m_tester->m_bReceive, false);
    EXPECT_EQ(m_tester->m_strSelUnCompressPath.isEmpty(), true);
    EXPECT_EQ(m_tester->m_bDrop, false);
}

TEST_F(UT_UnCompressView, test_mouseDoubleClickEvent)
{
    QTest::mouseDClick(m_tester, Qt::LeftButton);
}

TEST_F(UT_UnCompressView, test_calDirItemCount)
{
    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1/";
    entry.strFullPath = "1/";
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1/1.txt";
    entry.qSize = 10;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;

    EXPECT_EQ(m_tester->calDirItemCount("1/"), 1);
}

TEST_F(UT_UnCompressView, test_handleDoubleClick_001)
{
    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1/";
    entry.strFullPath = "1/";
    entry.isDirectory = true;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();

    Stub stub;
    stub.set(ADDR(UnCompressView, slotOpen), unCompressView_slotOpen_stub);

    QModelIndex index = m_tester->model()->index(0, 0);

    m_tester->m_iLevel = 0;
    m_tester->handleDoubleClick(index);
    EXPECT_EQ(m_tester->m_iLevel, 1);
}

TEST_F(UT_UnCompressView, test_handleDoubleClick_002)
{
    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1/1.txt";
    entry.isDirectory = false;
    entry.qSize = 10;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();

    Stub stub;
    stub.set(ADDR(UnCompressView, slotOpen), unCompressView_slotOpen_stub);

    QModelIndex index = m_tester->model()->index(0, 0);

    m_tester->m_iLevel = 0;
    m_tester->handleDoubleClick(index);
    EXPECT_EQ(m_tester->m_iLevel, 0);
}

TEST_F(UT_UnCompressView, test_refreshDataByCurrentPath_001)
{
    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1/";
    entry.strFullPath = "1/";
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1/1.txt";
    entry.qSize = 10;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();

    m_tester->m_iLevel = 0;
    m_tester->m_strCurrentPath = "/";
    m_tester->m_mapShowEntry.clear();
    m_tester->m_mapShowEntry[m_tester->m_strCurrentPath] = DataManager::get_instance().archiveData().listRootEntry;
    m_tester->refreshDataByCurrentPath();
}

TEST_F(UT_UnCompressView, test_refreshDataByCurrentPath_002)
{
    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1/";
    entry.strFullPath = "1/";
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1/1.txt";
    entry.qSize = 10;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();

    m_tester->m_iLevel = 1;
    m_tester->m_strCurrentPath = "/";
    m_tester->m_mapShowEntry.clear();
    m_tester->m_mapShowEntry[m_tester->m_strCurrentPath] = DataManager::get_instance().archiveData().listRootEntry;
    m_tester->refreshDataByCurrentPath();
}

TEST_F(UT_UnCompressView, test_refreshDataByCurrentPathChanged_001)
{
    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1/";
    entry.strFullPath = "1/";
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1/1.txt";
    entry.qSize = 10;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();

    m_tester->m_iLevel = 0;
    m_tester->m_strCurrentPath = "/";
    m_tester->m_mapShowEntry.clear();
    m_tester->m_mapShowEntry[m_tester->m_strCurrentPath] = DataManager::get_instance().archiveData().listRootEntry;
    m_tester->refreshDataByCurrentPathChanged();
    EXPECT_EQ(m_tester->m_eChangeType, UnCompressView::ChangeType::CT_None);
}

TEST_F(UT_UnCompressView, test_refreshDataByCurrentPathChanged_002)
{
    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1/";
    entry.strFullPath = "1/";
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1/1.txt";
    entry.qSize = 10;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();

    m_tester->m_iLevel = 1;
    m_tester->m_strCurrentPath = "1/";
    m_tester->m_eChangeType = UnCompressView::CT_Add;
    m_tester->refreshDataByCurrentPathChanged();
    EXPECT_EQ(m_tester->m_eChangeType, UnCompressView::ChangeType::CT_None);
}

TEST_F(UT_UnCompressView, test_addNewFiles_001)
{
    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1/";
    entry.strFullPath = "1/";
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.qSize = 10;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();

    Stub stub;
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    CustomDialogStub::stub_OverwriteQueryDialog_showDialog(stub);
    CustomDialogStub::stub_AppendDialog_showDialog(stub, 1);
    QFileInfoStub::stub_QFileInfo_path(stub, "");


    m_tester->m_strArchive = "/1.zip";

    m_tester->addNewFiles(QStringList() << "/1.zip");
    EXPECT_EQ(m_tester->m_eChangeType, UnCompressView::CT_None);
}

TEST_F(UT_UnCompressView, test_addNewFiles_002)
{
    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1/";
    entry.strFullPath = "1/";
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.qSize = 10;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();

    Stub stub;
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    CustomDialogStub::stub_OverwriteQueryDialog_showDialog(stub);
    CustomDialogStub::stub_AppendDialog_showDialog(stub, 1);
    QFileInfoStub::stub_QFileInfo_path(stub, "");
    QFileInfoStub::stub_QFileInfo_filePath(stub, "/1.txt");


    m_tester->m_strArchive = "/1.zip";

    CustomDialogStub::stub_OverwriteQueryDialog_getDialogResult(stub, Overwrite_Result::OR_Cancel);
    m_tester->addNewFiles(QStringList() << "/1.txt");
    EXPECT_EQ(m_tester->m_eChangeType, UnCompressView::CT_None);
}

TEST_F(UT_UnCompressView, test_addNewFiles_003)
{
    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1/";
    entry.strFullPath = "1/";
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.qSize = 10;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();

    Stub stub;
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    CustomDialogStub::stub_OverwriteQueryDialog_showDialog(stub);
    CustomDialogStub::stub_AppendDialog_showDialog(stub, 1);
    QFileInfoStub::stub_QFileInfo_path(stub, "");
    QFileInfoStub::stub_QFileInfo_filePath(stub, "/1.txt");
    CustomDialogStub::stub_OverwriteQueryDialog_getDialogResult(stub, Overwrite_Result::OR_Skip);

    m_tester->m_strArchive = "/1.zip";

    m_tester->addNewFiles(QStringList() << "/1.txt");
    EXPECT_EQ(m_tester->m_listAddFiles.count(), 0);
}

TEST_F(UT_UnCompressView, test_addNewFiles_004)
{
    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1/";
    entry.strFullPath = "1/";
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.qSize = 10;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();

    Stub stub;
    CustomDialogStub::stub_TipDialog_showDialog(stub, 0);
    CustomDialogStub::stub_OverwriteQueryDialog_showDialog(stub);
    CustomDialogStub::stub_AppendDialog_showDialog(stub, 1);
    QFileInfoStub::stub_QFileInfo_path(stub, "");
    QFileInfoStub::stub_QFileInfo_filePath(stub, "/1.txt");
    CustomDialogStub::stub_OverwriteQueryDialog_getDialogResult(stub, Overwrite_Result::OR_Overwrite);

    m_tester->m_strArchive = "/1.zip";

    m_tester->addNewFiles(QStringList() << "/1.txt");
    EXPECT_EQ(m_tester->m_listAddFiles.count(), 1);
    EXPECT_EQ(m_tester->m_eChangeType, UnCompressView::ChangeType::CT_Add);
}

TEST_F(UT_UnCompressView, test_getCurPath_001)
{
    m_tester->m_iLevel = 0;
    EXPECT_EQ(m_tester->getCurPath(), "");
}

TEST_F(UT_UnCompressView, test_getCurPath_002)
{
    m_tester->m_strCurrentPath = "/";
    m_tester->m_iLevel = 1;
    EXPECT_EQ(m_tester->getCurPath(), "/");
}

TEST_F(UT_UnCompressView, test_setModifiable)
{
    m_tester->setModifiable(false, false);
    EXPECT_EQ(m_tester->m_bMultiplePassword, false);
}

TEST_F(UT_UnCompressView, test_isModifiable)
{
    m_tester->m_bModifiable = false;
    EXPECT_EQ(m_tester->isModifiable(), false);
}

TEST_F(UT_UnCompressView, test_clear)
{
    m_tester->clear();
    EXPECT_EQ(m_tester->m_bReceive, false);
}

TEST_F(UT_UnCompressView, test_getCurPathFiles)
{
    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1/";
    entry.strFullPath = "1/";
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1/1.txt";
    entry.qSize = 10;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();

    m_tester->m_strCurrentPath = "1/";
    EXPECT_EQ(m_tester->getCurPathFiles().count(), 1);
}

TEST_F(UT_UnCompressView, test_getSelEntry)
{
    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1/";
    entry.strFullPath = "1/";
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();

    m_tester->selectAll();
    EXPECT_EQ(m_tester->getSelEntry().count(), 1);
}

TEST_F(UT_UnCompressView, test_extract2Path)
{
    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1/";
    entry.strFullPath = "1/";
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.qSize = 10;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();

    m_tester->selectAll();
    m_tester->extract2Path("/home/Desktop");
}

TEST_F(UT_UnCompressView, test_calEntrySizeByParentPath)
{
    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1/";
    entry.strFullPath = "1/";
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    entry.strFileName = "1/1.txt";
    entry.strFullPath = "1/1.txt";
    entry.qSize = 10;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;

    qint64 qSize;
    m_tester->calEntrySizeByParentPath("1/", qSize);
    EXPECT_EQ(qSize, 10);
}

TEST_F(UT_UnCompressView, test_slotDragFiles)
{
    m_tester->slotDragFiles(QStringList());
    EXPECT_EQ(m_tester->m_listAddFiles.isEmpty(), true);
}

TEST_F(UT_UnCompressView, test_slotShowRightMenu_001)
{
    Stub stub;
    CommonStub::stub_QTreeView_indexAt(stub);
    CommonStub::stub_QMenu_exec(stub);

    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1/";
    entry.strFullPath = "1/";
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    entry.strFileName = "1/1.txt";
    entry.strFullPath = "1/1.txt";
    entry.qSize = 10;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();

    m_tester->m_stRightEntry.isDirectory = true;
    m_tester->slotShowRightMenu(QPoint(10, 50));
}

TEST_F(UT_UnCompressView, test_slotShowRightMenu_002)
{
    Stub stub;
    CommonStub::stub_QTreeView_indexAt(stub);
    CommonStub::stub_QMenu_exec(stub);

    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1/";
    entry.strFullPath = "1/";
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    entry.strFileName = "1/1.txt";
    entry.strFullPath = "1/1.txt";
    entry.qSize = 10;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();

    m_tester->m_stRightEntry.isDirectory = false;
    m_tester->slotShowRightMenu(QPoint(10, 50));
}

TEST_F(UT_UnCompressView, test_slotExtract_001)
{
    Stub stub;
    DFileDialogStub::stub_DFileDialog_exec(stub, 0);
    m_tester->slotExtract();
}

TEST_F(UT_UnCompressView, test_slotExtract_002)
{
    Stub stub;
    QList<QUrl> listUrl;
    listUrl << QUrl("/home/Desktop");
    DFileDialogStub::stub_DFileDialog_selectedUrls(stub, listUrl);
    DFileDialogStub::stub_DFileDialog_exec(stub, 1);
    m_tester->slotExtract();
    EXPECT_EQ(m_tester->m_strSelUnCompressPath, "");
}

TEST_F(UT_UnCompressView, test_slotExtract2Here)
{
    m_tester->extract2Path("/home/Desktop");
}

TEST_F(UT_UnCompressView, test_slotDeleteFile)
{
    Stub stub;
    CustomDialogStub::stub_SimpleQueryDialog_showDialog(stub, 1);

    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1/";
    entry.strFullPath = "1/";
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();

    m_tester->selectAll();
    m_tester->m_bModifiable = true;
    m_tester->slotDeleteFile();
    EXPECT_EQ(m_tester->m_eChangeType, UnCompressView::CT_Delete);
}

TEST_F(UT_UnCompressView, test_slotRenameFile)
{
    Stub stub;
    CustomDialogStub::stub_RenameDialog_showDialog(stub, 1);

    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1";
    entry.strAlias = "2";
    entry.strFullPath = "1/";
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();

    m_tester->selectAll();
    m_tester->m_bModifiable = true;
    m_tester->slotRenameFile();
    EXPECT_EQ(m_tester->m_eChangeType, UnCompressView::CT_Rename);
}

TEST_F(UT_UnCompressView, test_slotOpen_001)
{
    Stub stub;
    typedef void (*fptr)(UnCompressView *, const QModelIndex &);
    fptr A_foo = (fptr)(&UnCompressView::handleDoubleClick);   // 获取虚函数地址
    stub.set(A_foo, unCompressView_handleDoubleClick_stub);

    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1/";
    entry.strFullPath = "1/";
    entry.isDirectory = true;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.isDirectory = false;
    entry.qSize = 10;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();

    m_tester->setCurrentIndex(m_tester->m_pModel->index(0, 0));
    m_tester->slotOpen();
}

TEST_F(UT_UnCompressView, test_slotOpen_002)
{
    Stub stub;
    typedef void (*fptr)(UnCompressView *, const QModelIndex &);
    fptr A_foo = (fptr)(&UnCompressView::handleDoubleClick);   // 获取虚函数地址
    stub.set(A_foo, unCompressView_handleDoubleClick_stub);

    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1/";
    entry.strFullPath = "1/";
    entry.isDirectory = true;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.isDirectory = false;
    entry.qSize = 10;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();

    m_tester->setCurrentIndex(m_tester->m_pModel->index(1, 0));
    m_tester->slotOpen();
}

TEST_F(UT_UnCompressView, test_focusInEvent_001)
{
    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1/1.txt";
    entry.qSize = 10;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();

    QFocusEvent *event = new QFocusEvent(QEvent::FocusIn, Qt::TabFocusReason);
    m_tester->focusInEvent(event);
    delete event;
    EXPECT_EQ(m_tester->m_reson, Qt::TabFocusReason);
}

TEST_F(UT_UnCompressView, test_focusInEvent_002)
{
    QFocusEvent *event = new QFocusEvent(QEvent::FocusIn, Qt::TabFocusReason);
    m_tester->selectionModel()->setCurrentIndex(m_tester->model()->index(0, 0), QItemSelectionModel::Select);
    m_tester->focusInEvent(event);
    delete event;
    EXPECT_EQ(m_tester->m_reson, Qt::TabFocusReason);
}

TEST_F(UT_UnCompressView, test_dragEnterEvent_001)
{
    QMimeData *data = new QMimeData;
    QDragEnterEvent *e = new QDragEnterEvent(QPoint(0, 0), Qt::CopyAction, data, Qt::LeftButton, Qt::NoModifier);
    m_tester->dragEnterEvent(e);

    delete data;
    delete e;
}

TEST_F(UT_UnCompressView, test_dragEnterEvent_002)
{
    QMimeData *data = new QMimeData;
    QDragEnterEvent *e = new QDragEnterEvent(QPoint(0, 0), Qt::CopyAction, data, Qt::LeftButton, Qt::NoModifier);

    QList<QUrl> listUrls;
    listUrls << QUrl("123456");
    data->setUrls(listUrls);
    m_tester->dragEnterEvent(e);

    delete data;
    delete e;
}

TEST_F(UT_UnCompressView, test_dragMoveEvent_001)
{
    QMimeData *data = new QMimeData;
    QDragMoveEvent *e = new QDragMoveEvent(QPoint(0, 0), Qt::CopyAction, data, Qt::LeftButton, Qt::NoModifier);
    m_tester->dragMoveEvent(e);

    delete data;
    delete e;
}

TEST_F(UT_UnCompressView, test_dropEvent_001)
{
    QMimeData *data = new QMimeData;
    QDropEvent *e = new QDropEvent(QPoint(0, 0), Qt::CopyAction, data, Qt::LeftButton, Qt::NoModifier);
    m_tester->dropEvent(e);

    delete data;
    delete e;
}

TEST_F(UT_UnCompressView, test_dropEvent_002)
{
    QMimeData *data = new QMimeData;
    QDropEvent *e = new QDropEvent(QPoint(0, 0), Qt::CopyAction, data, Qt::LeftButton, Qt::NoModifier);

    QList<QUrl> listUrls;
    listUrls << QUrl("123456");
    data->setUrls(listUrls);
    m_tester->dropEvent(e);

    delete data;
    delete e;
}

TEST_F(UT_UnCompressView, test_resizeEvent)
{
    QResizeEvent *event = new QResizeEvent(QSize(100, 100), QSize(80, 80));
    m_tester->resizeEvent(event);
    delete event;
    EXPECT_EQ(m_tester->columnWidth(0), m_tester->width() * 25 / 58);
    EXPECT_EQ(m_tester->columnWidth(1), m_tester->width() * 17 / 58);
    EXPECT_EQ(m_tester->columnWidth(2), m_tester->width() * 8 / 58);
    EXPECT_EQ(m_tester->columnWidth(3), m_tester->width() * 8 / 58);
}

TEST_F(UT_UnCompressView, test_event)
{
    QTouchDevice *pDevice = new QTouchDevice;
    pDevice->setType(QTouchDevice::TouchScreen);
    QTouchEvent *e = new QTouchEvent(QEvent::TouchBegin, pDevice, Qt::NoModifier, Qt::TouchPointPressed);
    e->setTouchPoints(QList<QTouchEvent::TouchPoint>() << QTouchEvent::TouchPoint());
    m_tester->m_isPressed = false;
    m_tester->event(e);

    delete pDevice;
    delete e;

    EXPECT_EQ(m_tester->m_isPressed, true);
}

TEST_F(UT_UnCompressView, test_keyPressEvent)
{
    Stub stub;
    stub.set(ADDR(QItemSelectionModel, selectedRows), qItemSelectionModel_selectedRows_stub);
    typedef void (*fptr)(UnCompressView *);
    fptr A_foo = (fptr)(&UnCompressView::slotDeleteFile);   // 获取虚函数地址
    stub.set(A_foo, slotDeleteFile_stub);

    typedef void (*fptr1)(UnCompressView *, const QPoint &);
    fptr1 A_foo1 = (fptr1)(&UnCompressView::slotShowRightMenu);   // 获取虚函数地址
    stub.set(A_foo1, slotShowRightMenu_stub);

    typedef void (*fptr2)(UnCompressView *, const QModelIndex &);
    fptr2 A_foo2 = (fptr2)(&UnCompressView::handleDoubleClick);   // 获取虚函数地址
    stub.set(A_foo2, handleDoubleClick_stub);
    QTest::keyPress(m_tester, Qt::Key_Delete);
    QTest::keyPress(m_tester, Qt::Key_M, Qt::AltModifier);
    QTest::keyPress(m_tester, Qt::Key_Enter);
    QTest::keyPress(m_tester, Qt::Key_Tab);
}

TEST_F(UT_UnCompressView, test_setPreLblVisible)
{
    Stub stub;
    CustomDialogStub::stub_OpenWithDialog_showOpenWithDialog(stub, "");

    QAction *pAction = new QAction("Select default program", m_tester);
    CommonStub::stub_QObject_sender(stub, pAction);
    m_tester->setPreLblVisible(true, "");

    pAction = new QAction("sss", m_tester);
    Stub stub1;
    CommonStub::stub_QObject_sender(stub1, pAction);
}

TEST_F(UT_UnCompressView, test_slotPreClicked_001)
{
    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1/1.txt";
    entry.qSize = 10;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();
    m_tester->m_vPre << "/";
    m_tester->m_iLevel = 0;
    m_tester->slotPreClicked();
}

TEST_F(UT_UnCompressView, test_slotPreClicked_002)
{
    DataManager::get_instance().resetArchiveData();
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1/1.txt";
    entry.qSize = 10;
    DataManager::get_instance().archiveData().listRootEntry << entry;
    DataManager::get_instance().archiveData().mapFileEntry[entry.strFullPath] = entry;
    m_tester->refreshArchiveData();
    m_tester->m_iLevel = 1;
    m_tester->m_vPre << "1/";
    m_tester->slotPreClicked();
}
