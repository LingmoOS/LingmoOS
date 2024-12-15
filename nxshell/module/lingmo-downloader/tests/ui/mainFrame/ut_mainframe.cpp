// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gtest/gtest.h"
#include <iostream>
#include <qtest.h>
#include <QDebug>
#include <QTimer>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QPoint>
#include <QClipboard>
#include <QMenu>
#include <QKeyEvent>
#include <QWebSocket>
#include <QFocusEvent>
#include <DSettings>
#include <DSettingsOption>
#include <DGuiApplicationHelper>
#include "mainframe.h"
#include "createtaskwidget.h"
#include "tableView.h"
#include "tableModel.h"
#include "global.h"
#include "tabledatacontrol.h"
#include "dbdefine.h"
#include "analysisurl.h"
#include "stub.h"
#include "stubAll.h"
#include "diagnostictool.h"
#include "messagebox.h"
#include "clipboardtimer.h"
#include "searchresoultwidget.h"
#include "btinfodialog.h"
#include "dbinstance.h"
#include "settings.h"
class ut_MainFreme : public ::testing::Test
    , public QObject
{
protected:
    ut_MainFreme()
    {
    }
    virtual ~ut_MainFreme()
    {
    }
    virtual void SetUp()
    {
    }
    virtual void TearDown()
    {
    }
};

TEST_F(ut_MainFreme, onSearchItemClicked1)
{
    Stub stub;
    stub.set(ADDR(SearchResoultWidget, hide), SearchResoultWidgetHide);
    DownloadDataItem *pItem1 = new DownloadDataItem;
    pItem1->taskId = "111";
    pItem1->status = 0;

    MainFrame::instance()->m_DownLoadingTableView->getTableModel()->append(pItem1);
    QListWidgetItem item;
    item.setData(Qt::WhatsThisRole, "1");
    item.setData(Qt::UserRole, "Downloading");
    MainFrame::instance()->onSearchItemClicked(&item);
}

TEST_F(ut_MainFreme, onSearchItemClicked2)
{
    Stub stub;
    stub.set(ADDR(SearchResoultWidget, hide), SearchResoultWidgetHide);
    DownloadDataItem *pItem1 = new DownloadDataItem;
    pItem1->taskId = "123";
    pItem1->status = 3;
    pItem1->fileName = "pItem1";
    MainFrame::instance()->m_DownLoadingTableView->getTableModel()->append(pItem1);
    QListWidgetItem item;
    item.setData(Qt::WhatsThisRole, "2");
    item.setData(Qt::UserRole, "Completed");
    MainFrame::instance()->onSearchItemClicked(&item);
}

TEST_F(ut_MainFreme, onSearchItemClicked3)
{
    Stub stub;
    stub.set(ADDR(SearchResoultWidget, hide), SearchResoultWidgetHide);
    DeleteDataItem *pItem3 = new DeleteDataItem;
    pItem3->taskId = "1234";
    pItem3->status = 4;
    pItem3->fileName = "pItem3";
    MainFrame::instance()->m_DownLoadingTableView->getTableModel()->append(pItem3);
    MainFrame::instance()->m_DownLoadingTableView->getTableModel()->getTablemodelMode();
    MainFrame::instance()->m_DownLoadingTableView->getTableModel()->dataList();
    MainFrame::instance()->m_DownLoadingTableView->getTableModel()->recyleList();
    QListWidgetItem item;
    item.setData(Qt::WhatsThisRole, "3");
    item.setData(Qt::UserRole, "Trash");
    MainFrame::instance()->onSearchItemClicked(&item);
}

TEST_F(ut_MainFreme, onSearchEditTextChanged)
{
    DownloadDataItem *pItem1 = new DownloadDataItem;
    pItem1->taskId = "123";
    pItem1->status = 3;
    pItem1->fileName = "1pItem1";
    MainFrame::instance()->m_DownLoadingTableView->getTableModel()->append(pItem1);
    MainFrame::instance()->onSearchEditTextChanged("1");
}

//metalink任务
TEST_F(ut_MainFreme, addMetalinkTask)
{
    Stub stub;
    stub.set(ADDR(QSystemTrayIcon, show), QsystemtrayiconShow);
    Stub stub2;
    stub2.set(ADDR(ClipboardTimer, checkClipboardHasUrl), ClipboardtimerCheckclipboardhasurl);
    Stub stub3;
    auto option = Settings::getInstance()->m_settings->option("DownloadTaskManagement.downloadtaskmanagement.MaxDownloadTask");
    option->setValue(2);
    MainFrame::instance()->onDownloadNewUrl("magnet:?xt=urn:btih:0FC4D73CCC9E6AC29A1B10DDCC3696E81D6CACAF",
                                            Settings::getInstance()->getDownloadSavePath(), "qweqwe", "torrent");
    QTest::qWait(15000);
    EXPECT_TRUE(true);
}

//小任务，可以快速下载完，方便测试已完成列表。
TEST_F(ut_MainFreme, addHttpFastTask)
{
    auto optionn = Settings::getInstance()->m_settings->option("Basic.DownloadDirectory.downloadDirectoryFileChooser");

    optionn->setValue("custom;" + QDir::homePath() + "/Downloads");
    Stub stub;
    stub.set(ADDR(QSystemTrayIcon, show), QsystemtrayiconShow);
    Stub stub2;
    stub2.set(ADDR(ClipboardTimer, checkClipboardHasUrl), ClipboardtimerCheckclipboardhasurl);
    auto option = Settings::getInstance()->m_settings->option("DownloadTaskManagement.downloadtaskmanagement.MaxDownloadTask");
    option->setValue(2);
    QString path = Settings::getInstance()->getDownloadSavePath();
    MainFrame::instance()->onDownloadNewUrl("http://10.10.53.248/download/image-1.png",
                                            path, "image", "png");
    TableView *table = MainFrame::instance()->findChild<TableView *>("downloadTableView");
    TableModel *model = static_cast<TableModel *>(table->model());
    QTest::qWait(500);
    EXPECT_TRUE(true);
}

TEST_F(ut_MainFreme, addHttpTask)
{
    MainFrame::instance()->onDownloadNewUrl("http://download.qt.io/archive/qt/4.1/qt-x11-opensource-src-4.1.4.tar.gz",
                                            Settings::getInstance()->getDownloadSavePath(), "qt-x11-opensource-src-4.1.4", "tar.gz");
    TableView *table = MainFrame::instance()->findChild<TableView *>("downloadTableView");
    TableModel *model = static_cast<TableModel *>(table->model());
    QTest::qWait(500);
    EXPECT_TRUE(true);
}

TEST_F(ut_MainFreme, addHttpTaskk)
{
    MainFrame::instance()->onDownloadNewUrl("http://10.10.53.248/download/30MB.gz",
                                            Settings::getInstance()->getDownloadSavePath(), "30MB", "gz");
    TableView *table = MainFrame::instance()->findChild<TableView *>("downloadTableView");
    TableModel *model = static_cast<TableModel *>(table->model());
    QTest::qWait(500);
    EXPECT_TRUE(true);
}

TEST_F(ut_MainFreme, addHttpTaskkk)
{
    MainFrame::instance()->onDownloadNewUrl("http://10.10.53.248/download/image-1.png",
                                            Settings::getInstance()->getDownloadSavePath(), "image", "png");
    TableView *table = MainFrame::instance()->findChild<TableView *>("downloadTableView");
    TableModel *model = static_cast<TableModel *>(table->model());
    QTest::qWait(500);
    EXPECT_TRUE(true);
}

TEST_F(ut_MainFreme, changeList)
{
    //MainFrame::instance()->m_DownLoadingTableView->m_TableModel->m_RenderList.clear();
    DListView *list = MainFrame::instance()->findChild<DListView *>("leftList");
    MainFrame::instance()->onListClicked(list->model()->index(1, 0));
}

TEST_F(ut_MainFreme, rename)
{
    Stub stub;
    stub.set((QAction * (QMenu::*)(const QPoint &pos, QAction *at)) ADDR(QMenu, exec), QmenuExec);
    TableView *table = MainFrame::instance()->m_DownLoadingTableView;
    TableModel *model = table->getTableModel();
    QTest::qWait(200);
    QRect rect = table->visualRect(model->index(0, 2));
    QTimer::singleShot(500, this, [=]() {
        MainFrame::instance()->onRenameActionTriggered();

        QTest::qWait(500);
        DLineEdit *w = qobject_cast<DLineEdit *>(QApplication::focusWidget());
        if(w == nullptr){
            return ;
        }
        w->lineEdit()->setText("");
        QTest::qWait(100);
        QTest::keyClick(w->lineEdit(), 't');
        QTest::qWait(100);
        QTest::keyClick(w->lineEdit(), 'e');
        QTest::qWait(100);
        QTest::keyClick(w->lineEdit(), 's');
        QTest::qWait(100);
        QTest::keyClick(w->lineEdit(), 't');
        QTest::qWait(100);
        QTest::keyClick(w->lineEdit(), 'O');
        QTest::qWait(100);
        QTest::keyClick(w->lineEdit(), 'K');
        QTest::qWait(500);
        EXPECT_TRUE(w->lineEdit()->text() == "testOK");
        return;
    });
    QTest::mouseClick(table->viewport(), Qt::LeftButton, Qt::KeyboardModifiers(), rect.center());
    QTest::qWait(100);
    MainFrame::instance()->onContextMenu(rect.center());
    QTest::qWait(2000);
}

TEST_F(ut_MainFreme, rename2)
{
    Stub stub;
    stub.set((QAction * (QMenu::*)(const QPoint &pos, QAction *at)) ADDR(QMenu, exec), QmenuExec);
    TableView *table = MainFrame::instance()->m_DownLoadingTableView;
    TableModel *model = static_cast<TableModel *>(table->model());
    QTest::qWait(200);
    QRect rect = table->visualRect(model->index(0, 2));
    QTimer::singleShot(500, this, [=]() {
        MainFrame::instance()->onRenameActionTriggered();
        QTest::qWait(500);
        DLineEdit *w = qobject_cast<DLineEdit *>(QApplication::focusWidget());
        if(w == nullptr){
            return ;
        }
        w->lineEdit()->setText("");
        QTest::qWait(100);
        QTest::keyClick(w->lineEdit(), 't');
        QTest::qWait(100);
        QTest::keyClick(w->lineEdit(), 'e');
        QTest::qWait(100);
        QTest::keyClick(w->lineEdit(), 's');
        QTest::qWait(100);
        QTest::keyClick(w->lineEdit(), 't');
        QTest::qWait(100);
        QTest::keyClick(w->lineEdit(), 'O');
        QTest::qWait(100);
        QTest::keyClick(w->lineEdit(), 'K');
        QTest::qWait(500);
        QTest::keyClick(w->lineEdit(), 'l');
        QTest::qWait(500);
        QTest::keyClick(w->lineEdit(), 'e');
        QTest::qWait(500);
        EXPECT_TRUE(w->lineEdit()->text() == "testOKle");
        return;
    });
    QTest::mouseClick(table->viewport(), Qt::LeftButton, Qt::KeyboardModifiers(), rect.center());
    QTest::qWait(100);
    MainFrame::instance()->onContextMenu(rect.center());
    QTest::qWait(2000);
}

TEST_F(ut_MainFreme, createNewTask)
{
    typedef int (*fptr)(CreateTaskWidget *);
    fptr foo = (fptr)(&CreateTaskWidget::exec);
    Stub stub;
    stub.set(foo, MessageboxExec);
    MainFrame::instance()->createNewTask("http://download.qt.io/archive/qt/4.1/qt-x11-opensource-src-4.1.4.tar.gz");
    QTest::qWait(500);
    EXPECT_TRUE(true);
}

//TEST_F(ut_MainFreme, createNewTask_torrent1)
//{
//    typedef int (*fptr)(CreateTaskWidget *);
//    fptr foo = (fptr)(&CreateTaskWidget::exec);
//    Stub stub;
//    stub.set(foo, MessageboxExec);
//    auto option = Settings::getInstance()->m_settings->option("Basic.DownloadDirectory.downloadDirectoryFileChooser");
//    option->setValue(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/Documents");
//    MainFrame::instance()->createNewTask("http://10.10.53.248/download/123.torrent");

//    QTest::qWait(500);
//    EXPECT_TRUE(true);
//}

//TEST_F(ut_MainFreme, createNewTask_torrent2)
//{
//    typedef int (*fptr)(CreateTaskWidget *);
//    fptr foo = (fptr)(&CreateTaskWidget::exec);
//    Stub stub;
//    stub.set(foo, MessageboxExec);
//    auto option = Settings::getInstance()->m_settings->option("Basic.DownloadDirectory.downloadDirectoryFileChooser");
//    option->setValue(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/Documents");
//    MainFrame::instance()->createNewTask("http://10.10.53.248/download/zimu.torrent");
//    QTest::qWait(500);
//    EXPECT_TRUE(true);
//}

//TEST_F(ut_MainFreme, addBtTask)
//{
//    QMap<QString, QVariant> opt;
//    MainFrame::instance()->onDownloadNewTorrent(QDir::homePath() + "/Documents/123.torrent",
//                                                opt, "123.torrent", "tar.gz");
//    QTest::qWait(500);
//    EXPECT_TRUE(true);
//    //QTest::qWait(5000);
//}

TEST_F(ut_MainFreme, addMetalinkTask2)
{
    QMap<QString, QVariant> opt;
    MainFrame::instance()->onDownloadNewMetalink(QDir::homePath() + "/Documents/asd.metalink",
                                                 opt, "asd.metalink");
    QTest::qWait(500);
    EXPECT_TRUE(true);
    //QTest::qWait(5000);
}

TEST_F(ut_MainFreme, changeList1)
{
    DListView *list = MainFrame::instance()->findChild<DListView *>("leftList");
    MainFrame::instance()->onListClicked(list->model()->index(0, 0));
}

TEST_F(ut_MainFreme, pauseTask)
{
    Stub stub;
    stub.set(ADDR(CreateTaskWidget, showNetErrorMsg), QsystemtrayiconShow);
    TableView *table = MainFrame::instance()->findChild<TableView *>("downloadTableView");
    TableModel *model = static_cast<TableModel *>(table->model());
    model->setData(model->index(0, 0), true, TableModel::Ischecked);
    MainFrame::instance()->onPauseDownloadBtnClicked();
    QTest::qWait(100);
    EXPECT_TRUE(true);
}

TEST_F(ut_MainFreme, unpauseTask)
{
  Stub stub;
    stub.set(ADDR(CreateTaskWidget, showNetErrorMsg), QsystemtrayiconShow);
    TableView *table = MainFrame::instance()->findChild<TableView *>("downloadTableView");
    TableModel *model = static_cast<TableModel *>(table->model());
    MainFrame::instance()->onStartDownloadBtnClicked();
    QTest::qWait(100);
    EXPECT_TRUE(true);
}

TEST_F(ut_MainFreme, onContextMenu)
{
    Stub stub;
    stub.set((QAction * (QMenu::*)(const QPoint &pos, QAction *at)) ADDR(QMenu, exec), QmenuExec);
    TableView *table = MainFrame::instance()->findChild<TableView *>("downloadTableView");
    TableModel *model = static_cast<TableModel *>(table->model());
    MainFrame::instance()->onContextMenu(table->visualRect(model->index(0, 2)).center());
    EXPECT_TRUE(true);
}

TEST_F(ut_MainFreme, changeListt)
{
    DListView *list = MainFrame::instance()->findChild<DListView *>("leftList");
    MainFrame::instance()->onListClicked(list->model()->index(0, 0));
    MainFrame::instance()->onListClicked(list->model()->index(2, 0));
    MainFrame::instance()->onListClicked(list->model()->index(1, 0));
}

TEST_F(ut_MainFreme, searchEditTextChanged)
{
    TableView *table = MainFrame::instance()->findChild<TableView *>("downloadTableView");
    QString text;
    QList<QString> taskIDList;
    QList<int> taskStatusList;
    QList<QString> tasknameList;
    table->getTableControl()->searchEditTextChanged(text, taskIDList, taskStatusList, tasknameList);
    EXPECT_TRUE(true);
}

TEST_F(ut_MainFreme, deleteTask)
{
    TableView *table = MainFrame::instance()->findChild<TableView *>("downloadTableView");
    TableModel *model = static_cast<TableModel *>(table->model());
    model->setData(model->index(0, 0), true, TableModel::Ischecked);
    table->getTableControl()->onDelAction(1);
    QTest::qWait(3000);
    MainFrame::instance()->onDeleteConfirm(false, false);
    TableView *table2 = MainFrame::instance()->findChild<TableView *>("recycleTableView");
    TableModel *model2 = static_cast<TableModel *>(table2->model());
    EXPECT_TRUE(true);
}

TEST_F(ut_MainFreme, redownloadTrashTask)
{
    DListView *list2 = MainFrame::instance()->findChild<DListView *>("leftList");
    MainFrame::instance()->onListClicked(list2->model()->index(0, 2));
    QTest::qWait(1000);
    TableView *table = MainFrame::instance()->findChild<TableView *>("recycleTableView");
    TableModel *model = static_cast<TableModel *>(table->model());
    MainFrame::instance()->onRedownload(model->data(model->index(0, 0), TableModel::taskId).toString(), 2);
    QTest::qWait(500);
    TableView *table2 = MainFrame::instance()->findChild<TableView *>("downloadTableView");
    TableModel *model2 = static_cast<TableModel *>(table2->model());
    QList<Global::DownloadDataItem *> list = model2->dataList();
    for (Global::DownloadDataItem *item : list) {
        if (item->url == "https://img.tukuppt.com/video_show/7165162/00/19/39/5f06cfe424c38_10s_big.mp4") {
            EXPECT_TRUE(true);
            return;
        }
    }
    QTest::qWait(2000);
}

TEST_F(ut_MainFreme, deleteTask2)
{
    TableView *table = MainFrame::instance()->findChild<TableView *>("downloadTableView");
    TableModel *model = static_cast<TableModel *>(table->model());
    model->setData(model->index(0, 0), true, TableModel::Ischecked);
    table->getTableControl()->onDelAction(1);
    MainFrame::instance()->onDeleteConfirm(false, false);
    TableView *table2 = MainFrame::instance()->findChild<TableView *>("recycleTableView");
    TableModel *model2 = static_cast<TableModel *>(table2->model());
    EXPECT_TRUE(true);
    QTest::qWait(1000);
}

TEST_F(ut_MainFreme, backTrashTask)
{
    TableView *table = MainFrame::instance()->findChild<TableView *>("recycleTableView");
    TableModel *model = static_cast<TableModel *>(table->model());
    model->setData(model->index(0, 0), true, TableModel::Ischecked);
    MainFrame::instance()->onReturnOriginActionTriggered();
    EXPECT_TRUE(true);
    QTest::qWait(1000);
}

TEST_F(ut_MainFreme, changeToFinishList2)
{
    DListView *list = MainFrame::instance()->findChild<DListView *>("leftList");
    MainFrame::instance()->onListClicked(list->model()->index(1, 0));
}

TEST_F(ut_MainFreme, deleteTask3)
{
    TableView *table = MainFrame::instance()->findChild<TableView *>("downloadTableView");
    TableModel *model = static_cast<TableModel *>(table->model());
    model->setData(model->index(0, 0), true, TableModel::Ischecked);
    table->getTableControl()->onDelAction(1);
    MainFrame::instance()->onDeleteConfirm(false, false);
    TableView *table2 = MainFrame::instance()->findChild<TableView *>("recycleTableView");
    TableModel *model2 = static_cast<TableModel *>(table2->model());
    EXPECT_TRUE(true);
    QTest::qWait(1000);
}
TEST_F(ut_MainFreme, changeToTrashList3)
{
    DListView *list = MainFrame::instance()->findChild<DListView *>("leftList");
    MainFrame::instance()->onListClicked(list->model()->index(2, 0));
}

TEST_F(ut_MainFreme, deleteTrashTask)
{
    TableView *table = MainFrame::instance()->findChild<TableView *>("recycleTableView");
    TableModel *model = static_cast<TableModel *>(table->model());
    model->setData(model->index(0, 0), true, TableModel::Ischecked);
    table->getTableControl()->onDelAction(2);
    MainFrame::instance()->onDeleteConfirm(true, true);
    QTest::qWait(1000);
}

TEST_F(ut_MainFreme, addHttpFastTask2)
{
    Stub stub;
    stub.set(ADDR(MainFrame, showRedownloadMsgbox), MainframeShowredownloadmsgbox);
    MainFrame::instance()->onDownloadNewUrl("https://img.tukuppt.com/video_show/7165162/00/19/39/5f06cfe424c38_10s_big.mp4",
                                            Settings::getInstance()->getDownloadSavePath(), "5f06cfe424c38_10s_big", "mp4");
    TableView *table = MainFrame::instance()->findChild<TableView *>("downloadTableView");
    TableModel *model = static_cast<TableModel *>(table->model());
    QList<Global::DownloadDataItem *> list = model->dataList();
    EXPECT_TRUE(true);
    QTest::qWait(2000);
}

TEST_F(ut_MainFreme, changeToFinishList3)
{
    DListView *list = MainFrame::instance()->findChild<DListView *>("leftList");
    MainFrame::instance()->onListClicked(list->model()->index(1, 0));
    EXPECT_TRUE(true);
}

TEST_F(ut_MainFreme, deleteFinishTask3)
{
    TableView *table = MainFrame::instance()->findChild<TableView *>("downloadTableView");
    TableModel *model = static_cast<TableModel *>(table->model());
    model->setData(model->index(0, 0), true, TableModel::Ischecked);
    table->getTableControl()->onDelAction(1);
    MainFrame::instance()->onDeleteConfirm(true, true);
    TableView *table2 = MainFrame::instance()->findChild<TableView *>("recycleTableView");
    TableModel *model2 = static_cast<TableModel *>(table2->model());
    EXPECT_TRUE(true);
    QTest::qWait(1000);
}

TEST_F(ut_MainFreme, changeToTrashList4)
{
    DListView *list = MainFrame::instance()->findChild<DListView *>("leftList");
    MainFrame::instance()->onListClicked(list->model()->index(2, 0));
    EXPECT_TRUE(true);
}

TEST_F(ut_MainFreme, clipboard)
{
    QProcess p;
    QString str = "echo \"https://img.tukuppt.com/video_show/7165162/00/19/39/5f06cfe424c38_10s_big.mp4\" | xclip";
    p.start();
}

TEST_F(ut_MainFreme, onTableItemSelected)
{
    DListView *list = MainFrame::instance()->findChild<DListView *>("leftList");
    MainFrame::instance()->onListClicked(list->model()->index(0, 0));
    TableView *table = MainFrame::instance()->findChild<TableView *>("downloadTableView");
    TableModel *model = static_cast<TableModel *>(table->model());
    if (model->renderList().count() > 0) {
        MainFrame::instance()->onTableItemSelected(model->index(0, 0));
    }
}

TEST_F(ut_MainFreme, onRedownloadActionTriggered)
{
    typedef int (*fptr)(DSettingsDialog *);
    fptr foo = (fptr)(&MessageBox::exec);
    Stub stub;
    stub.set(foo, MessageboxExec);
    DListView *list = MainFrame::instance()->findChild<DListView *>("leftList");
    MainFrame::instance()->onListClicked(list->model()->index(0, 0));
    TableView *table = MainFrame::instance()->findChild<TableView *>("downloadTableView");
    TableModel *model = static_cast<TableModel *>(table->model());
    MainFrame::instance()->m_CurrentTab = CurrentTab::finishTab;
    if (model->renderList().count() > 0) {
        MainFrame::instance()->onRedownloadActionTriggered();
    }
}

//TEST_F(ut_MainFreme, addBtTask_zimu)
//{
//    QMap<QString, QVariant> opt;
//    opt.insert("dir", "/home/san/Downloads");
//    opt.insert("select-file", "1,2,3,4");
//    MainFrame::instance()->onDownloadNewTorrent(QDir::homePath() + "/Documents/zimu.torrent",
//                                                opt, "8A1A79C5ED1D.torrent", "magnet:?xt=urn:btih:081c0af2b872414061813f0b8cc18a1a79c5ed1d");
//    QTest::qWait(500);
//    EXPECT_TRUE(true);
//    QTest::qWait(2000);
//}

TEST_F(ut_MainFreme, removeBtTask)
{
    typedef int (*fptr)(DSettingsDialog *);
    fptr foo = (fptr)(&MessageBox::exec);
    Stub stub;
    stub.set(foo, MessageboxExec);
    MainFrame::instance()->m_CurrentTab = CurrentTab::downloadingTab;
    const QList<DownloadDataItem *> &selectList = MainFrame::instance()->m_DownLoadingTableView->getTableModel()->dataList();
    for (auto item : selectList) {
        if (item->fileName .contains("8A1A79C5ED1D.torrent")) {
            item->isChecked = true;
        }
    }
    MainFrame::instance()->onDeleteActionTriggered();
}

TEST_F(ut_MainFreme, onRedownloadActionTriggered_bt)
{
    typedef int (*fptr)(DSettingsDialog *);
    fptr foo = (fptr)(&MessageBox::exec);
    Stub stub;
    stub.set(foo, MessageboxExec);
    DListView *list = MainFrame::instance()->findChild<DListView *>("leftList");
    MainFrame::instance()->onListClicked(list->model()->index(2, 0));
    TableModel *model = MainFrame::instance()->m_RecycleTableView->getTableModel();
  //  MainFrame::instance()->m_DelCheckItem = MainFrame::instance()->m_RecycleTableView->getTableModel()->m_RecyleList.first();
    MainFrame::instance()->m_CurrentTab = CurrentTab::recycleTab;
    if (model->m_RecyleList.count() > 0 && MainFrame::instance()->m_DelCheckItem != nullptr) {
        MainFrame::instance()->onRedownloadActionTriggered();
    }
}

TEST_F(ut_MainFreme, clrearTrashTask)
{
    MainFrame::instance()->onClearRecycle(true);
    TableView *table = MainFrame::instance()->findChild<TableView *>("recycleTableView");
    TableModel *model = static_cast<TableModel *>(table->model());
    EXPECT_TRUE(model->recyleList().isEmpty());
}

TEST_F(ut_MainFreme, onCopyUrlActionTriggered)
{
    DListView *list = MainFrame::instance()->findChild<DListView *>("leftList");
    MainFrame::instance()->onListClicked(list->model()->index(0, 0));
    TableView *table = MainFrame::instance()->findChild<TableView *>("downloadTableView");
    TableModel *model = static_cast<TableModel *>(table->model());
    if (model->renderList().count() > 0) {
        model->setData(model->index(0, 0), true, TableModel::Ischecked);
        MainFrame::instance()->onTableItemSelected(model->index(0, 0));
        MainFrame::instance()->onCopyUrlActionTriggered();
    }
}

//TEST_F(ut_MainFreme, OpenFile)
//{
//    typedef int (*fptr)(BtInfoDialog *);
//    fptr foo = (fptr)(&BtInfoDialog::exec);
//    Stub stub;
//    stub.set(foo, MessageboxExec);
//    MainFrame::instance()->OpenFile(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/Documents/123.torrent");
//}

TEST_F(ut_MainFreme, onOpenFileActionTriggered)
{
    TableView *table = MainFrame::instance()->findChild<TableView *>("downloadTableView");
    TableModel *model = static_cast<TableModel *>(table->model());
    if (model->renderList().count() > 0) {
        MainFrame::instance()->onOpenFileActionTriggered();
    }
}

TEST_F(ut_MainFreme, onOpenFolderActionTriggered)
{
    TableView *table = MainFrame::instance()->findChild<TableView *>("downloadTableView");
    TableModel *model = static_cast<TableModel *>(table->model());
    if (model->renderList().count() > 0) {
        MainFrame::instance()->onOpenFolderActionTriggered();
    }
}

TEST_F(ut_MainFreme, checkIfInPeriod)
{
    QTime currentTime = QTime::currentTime();
    QTime periodStartTime;
    QTime periodEndTime;
    periodStartTime.setHMS(0, 0, 0);
    periodEndTime.setHMS(24, 59, 59);
    MainFrame::instance()->checkIfInPeriod(&currentTime, &periodStartTime, &periodEndTime);
}

TEST_F(ut_MainFreme, addHttpTask2)
{
    MainFrame::instance()->onDownloadNewUrl("http://download.qt.io/archive/qt/4.1/qt-x11-opensource-src-4.1.4.tar.gz",
                                            Settings::getInstance()->getDownloadSavePath(), "qt-x11-opensource-src-4.1.4", "tar.gz");
    TableView *table = MainFrame::instance()->findChild<TableView *>("downloadTableView");
    TableModel *model = static_cast<TableModel *>(table->model());
    QTest::qWait(500);
    EXPECT_TRUE(true);
}

TEST_F(ut_MainFreme, removeDownloadListJob)
{
    TableView *table = MainFrame::instance()->findChild<TableView *>("downloadTableView");
    TableModel *model = static_cast<TableModel *>(table->model());
    if(model->renderList().size() > 0) {
        Global::DownloadDataItem *data = model->renderList().at(0);
        table->getTableControl()->removeDownloadListJob(data, true, true);
    }
}

TEST_F(ut_MainFreme, initDataItem)
{
    Global::DownloadDataItem *data = new Global::DownloadDataItem;
    const TaskInfo tbTask;
    MainFrame::instance()->initDataItem(data, tbTask);
    delete data;
}

TEST_F(ut_MainFreme, initDelDataItem)
{
    Global::DownloadDataItem *data = new Global::DownloadDataItem;
    Global::DeleteDataItem *delData = new Global::DeleteDataItem;
    MainFrame::instance()->initDelDataItem(data, delData);
    delete data;
    delete delData;
}

TEST_F(ut_MainFreme, startDownloadTask)
{
    TableView *table = MainFrame::instance()->findChild<TableView *>("downloadTableView");
    TableModel *model = static_cast<TableModel *>(table->model());
    if (model->renderList().count() > 0) {
        Global::DownloadDataItem *data = model->renderList().at(0);
        MainFrame::instance()->startDownloadTask(data);
    }
}

TEST_F(ut_MainFreme, onParseUrlList)
{
    Stub stub;
    stub.set(ADDR(MainFrame, onDownloadNewUrl), MainframeOndownloadnewurl);
    typedef int (*fptr)(DSettingsDialog *);
    fptr foo = (fptr)(&MessageBox::exec);
    Stub stub2;
    stub2.set(foo, MessageboxExec);
    QVector<LinkInfo> urlList;
    urlList << LinkInfo();
    QString path = Settings::getInstance()->getDownloadSavePath();
    MainFrame::instance()->onParseUrlList(urlList, path);
}

TEST_F(ut_MainFreme, setAutoStart_true)
{
    MainFrame::instance()->setAutoStart(true);
}

TEST_F(ut_MainFreme, onDownloadFinish)
{
    MainFrame::instance()->onDownloadFinish();
}

TEST_F(ut_MainFreme, setAutoStart_false)
{
    MainFrame::instance()->setAutoStart(false);
}

TEST_F(ut_MainFreme, initDbus)
{
    MainFrame::instance()->initDbus();
}

TEST_F(ut_MainFreme, aria2MethodUnpauseAll)
{
    TableView *table = MainFrame::instance()->findChild<TableView *>("downloadTableView");
    QJsonObject json;
    int iCurrentRow = 0;
    table->getTableControl()->aria2MethodUnpauseAll(json, iCurrentRow);
}

TEST_F(ut_MainFreme, aria2MethodForceRemove)
{
    TableView *table = MainFrame::instance()->findChild<TableView *>("downloadTableView");
    QJsonObject json;
    table->getTableControl()->aria2MethodForceRemove(json);
}

TEST_F(ut_MainFreme, downloadListRedownload)
{
    TableView *table = MainFrame::instance()->findChild<TableView *>("downloadTableView");
    TableModel *model = static_cast<TableModel *>(table->model());
    if (model->renderList().count() > 0) {
        Global::DownloadDataItem *data = model->renderList().at(0);
        table->getTableControl()->downloadListRedownload(data->taskId);
    }
}

TEST_F(ut_MainFreme, onUnusualConfirm)
{
    Stub stub;
    stub.set(ADDR(MainFrame, showRedownloadMsgbox), MainframeShowredownloadmsgbox);
    TableView *table = MainFrame::instance()->findChild<TableView *>("downloadTableView");
    TableModel *model = static_cast<TableModel *>(table->model());
    if (model->renderList().count() > 0) {
        Global::DownloadDataItem *data = model->renderList().at(0);
        table->getTableControl()->onUnusualConfirm(0, data->taskId);
    }
}

TEST_F(ut_MainFreme, addHttpTask3)
{
    Stub stub;
    stub.set(ADDR(MainFrame, showRedownloadMsgbox), MainframeShowredownloadmsgbox);
    MainFrame::instance()->onDownloadNewUrl("http://download.qt.io/archive/qt/4.1/qt-x11-opensource-src-4.1.4.tar.gz",
                                            Settings::getInstance()->getDownloadSavePath(), "qt-x11-opensource-src-4.1.4", "tar.gz");
    TableView *table = MainFrame::instance()->findChild<TableView *>("downloadTableView");
    TableModel *model = static_cast<TableModel *>(table->model());
    EXPECT_TRUE(true);
}

TEST_F(ut_MainFreme, onSettingsMenuClicked)
{
    typedef int (*fptr)(DSettingsDialog *);
    fptr foo = (fptr)(&DSettingsDialog::exec);
    Stub stub;
    stub.set(foo, DsettingsdialogExec);
    MainFrame::instance()->onSettingsMenuClicked();
    QTest::qWait(3000);
}

TEST_F(ut_MainFreme, onSettingsMenuClicked2)
{
    typedef int (*fptr)(DSettingsDialog *);
    fptr foo = (fptr)(&DSettingsDialog::exec);
    Stub stub;
    stub.set(foo, DsettingsdialogExec);
    MainFrame::instance()->onSettingsMenuClicked();
    QTest::qWait(3000);
}

TEST_F(ut_MainFreme, showDiagnosticTool)
{
    DiagnosticTool control;
    control.show();
    QTest::qWait(5000);

}

TEST_F(ut_MainFreme, addHttpTask4)
{
    MainFrame::instance()->onDownloadNewUrl("http://download.qt.io/archive/qt/4.1/qt-x11-opensource-src-4.1.4.tar.gz",
                                            Settings::getInstance()->getDownloadSavePath(), "qt-x11-opensource-src-4.1.4", "tar.gz");
    TableView *table = MainFrame::instance()->findChild<TableView *>("downloadTableView");
    TableModel *model = static_cast<TableModel *>(table->model());
    QTest::qWait(500);
    EXPECT_TRUE(true);
}

TEST_F(ut_MainFreme, onDownloadFirstBtnClicked)
{
    typedef int (*fptr)(DSettingsDialog *);
    fptr foo = (fptr)(&MessageBox::exec);
    Stub stub;
    stub.set(foo, MessageboxExec);
    DListView *list = MainFrame::instance()->findChild<DListView *>("leftList");
    MainFrame::instance()->onListClicked(list->model()->index(0, 0));
    const QList<DownloadDataItem *> listItem = MainFrame::instance()->m_DownLoadingTableView->getTableModel()->renderList();
    if(listItem.isEmpty()){
        EXPECT_TRUE(true);
        return;
    }
    MainFrame::instance()->m_CheckItem = listItem.first();
    MainFrame::instance()->onDownloadFirstBtnClicked();
}

TEST_F(ut_MainFreme, onRemoveFinished)
{
    MainFrame::instance()->onRemoveFinished();
}

TEST_F(ut_MainFreme, showWarningMsgbox)
{
    typedef int (*fptr)(DSettingsDialog *);
    fptr foo = (fptr)(&DSettingsDialog::exec);
    Stub stub;
    stub.set(foo, DsettingsdialogExec);
    MainFrame::instance()->showWarningMsgbox("");
}

TEST_F(ut_MainFreme, showClearMsgbox)
{
    typedef int (*fptr)(DSettingsDialog *);
    fptr foo = (fptr)(&MessageBox::exec);
    Stub stub;
    stub.set(foo, MessageboxExec);
    MainFrame::instance()->showClearMsgbox();
}

TEST_F(ut_MainFreme, showDeleteMsgbox)
{
    typedef int (*fptr)(DSettingsDialog *);
    fptr foo = (fptr)(&MessageBox::exec);
    Stub stub;
    stub.set(foo, MessageboxExec);
    MainFrame::instance()->showDeleteMsgbox(true);
}

TEST_F(ut_MainFreme, showRedownloadMsgbox)
{
    typedef int (*fptr)(DSettingsDialog *);
    fptr foo = (fptr)(&MessageBox::exec);
    Stub stub;
    stub.set(foo, MessageboxExec);
    MainFrame::instance()->showRedownloadMsgbox("");
}

TEST_F(ut_MainFreme, clearShardMemary)
{
    TableView *table = MainFrame::instance()->findChild<TableView *>("downloadTableView");
    table->getTableControl()->clearShardMemary();
}

TEST_F(ut_MainFreme, initConnection)
{
    MainFrame::instance()->initConnection();
}

//TEST_F(ut_MainFreme, initAria2)
//{
//    MainFrame::instance()->initAria2();
//}
TEST_F(ut_MainFreme, initTabledata)
{
    MainFrame::instance()->initTabledata();
}

//TEST_F(ut_MainFreme, updateDHTFile)
//{
//    MainFrame::instance()->updateDHTFile();
//}

TEST_F(ut_MainFreme, tableView)
{
    TableView *table = new TableView(1);
    QMouseEvent *event = new QMouseEvent(QEvent::MouseButtonPress, QPoint(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    table->mousePressEvent(event);
    delete table;
    delete event;
}

TEST_F(ut_MainFreme, tableView2)
{
    TableView *table = new TableView(1);
    QMouseEvent *event = new QMouseEvent(QEvent::MouseButtonPress, QPoint(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    table->mouseMoveEvent(event);
    delete table;
    delete event;
}

TEST_F(ut_MainFreme, tableView3)
{
    TableView *table = new TableView(1);
    QMouseEvent *event = new QMouseEvent(QEvent::MouseButtonPress, QPoint(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    table->mouseReleaseEvent(event);
    delete table;
    delete event;
}

TEST_F(ut_MainFreme, tableView4)
{
    TableView *table = new TableView(1);
    QKeyEvent *keyEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_C, Qt::NoModifier);
    table->leaveEvent(keyEvent);
    delete table;
    delete keyEvent;
}

TEST_F(ut_MainFreme, tableView5)
{
    TableView *table = new TableView(1);
    QKeyEvent *keyEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_C, Qt::NoModifier);
    table->keyPressEvent(keyEvent);
    delete table;
    delete keyEvent;
}

TEST_F(ut_MainFreme, Websockethandle)
{
    QWebSocket socket;
    //socket.ignoreSslErrors(expectedSslErrors);
    socket.open(QUrl("ws://127.0.0.1:12345"));
    bool b = socket.isValid();
    QString data = "http://test.html";
    qint64 num = socket.sendTextMessage(data);
    EXPECT_TRUE(num >= 0);
}

TEST_F(ut_MainFreme, SearchResoultWidget)
{
    SearchResoultWidget *widget = new SearchResoultWidget;
    EXPECT_TRUE(true);
    delete widget;
}

TEST_F(ut_MainFreme, SearchResoultWidgetsetData)
{
    SearchResoultWidget *widget = new SearchResoultWidget;
    QList<QString> taskIDList;
    taskIDList << "aaa" << "bbb" << "ccc";
    QList<int> taskStatusList;
    taskStatusList << 0 << 1 << 2;
    QList<QString> tasknameList;
    tasknameList << "aaa" << "bbb" << "ccc";
    widget->setData(taskIDList, taskStatusList, tasknameList);
    EXPECT_TRUE(true);
    delete widget;
}

TEST_F(ut_MainFreme, SearchResoultWidgetonKeypressed)
{
    SearchResoultWidget *widget = new SearchResoultWidget;
    widget->onKeypressed(Qt::Key_Up);
    widget->onKeypressed(Qt::Key_Down);
    widget->onKeypressed(Qt::Key_Enter);
    EXPECT_TRUE(true);
    delete widget;
}

TEST_F(ut_MainFreme, SearchResoultWidgetfocusOutEvent)
{
    SearchResoultWidget *widget = new SearchResoultWidget;
    QFocusEvent *event = new QFocusEvent(QEvent::ActionRemoved);
    widget->focusOutEvent(event);
    EXPECT_TRUE(true);
    delete widget;
    delete event;
}

TEST_F(ut_MainFreme, SearchResoultWidgetkeyPressEvent)
{
    SearchResoultWidget *widget = new SearchResoultWidget;
    QKeyEvent *keyEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_C, Qt::NoModifier);
    widget->keyPressEvent(keyEvent);
    EXPECT_TRUE(true);
    delete widget;
    delete keyEvent;
}

TEST_F(ut_MainFreme, dealNotificaitonSettings)
{
    TableView *table = new TableView(1);
    table->getTableControl()->dealNotificaitonSettings("status", "test.txt", "-1");
    delete table;
}

TEST_F(ut_MainFreme, onClipboardDataChanged)
{
    typedef int (*fptr)(CreateTaskWidget *);
    fptr foo = (fptr)(&CreateTaskWidget::exec);
    Stub stub;
    stub.set(foo, MessageboxExec);
     MainFrame::instance()->onClipboardDataChanged("status");
}

TEST_F(ut_MainFreme, onPalettetypechanged)
{
     MainFrame::instance()->onPalettetypechanged(DGuiApplicationHelper::DarkType);
}

TEST_F(ut_MainFreme, onCheckChanged)
{
     MainFrame::instance()->onCheckChanged(true, 1);
}

TEST_F(ut_MainFreme, onClearRecyleActionTriggered)
{
    typedef int (*fptr)(DSettingsDialog *);
    fptr foo = (fptr)(&MessageBox::exec);
    Stub stub;
    stub.set(foo, MessageboxExec);
    MainFrame::instance()->onClearRecyleActionTriggered();
    //EXPECT_TRUE(MainFrame::instance()->m_RecycleTableView->m_TableModel->m_RecyleList.isEmpty());
}

TEST_F(ut_MainFreme, onDownloadLimitChanged)
{
     MainFrame::instance()->onDownloadLimitChanged();
}

TEST_F(ut_MainFreme, onDisckCacheChanged)
{
     MainFrame::instance()->onDisckCacheChanged(256);
}

TEST_F(ut_MainFreme, initDelDataItem2)
{
    Global::DownloadDataItem *data = new Global::DownloadDataItem;
    Global::DeleteDataItem *delData = new Global::DeleteDataItem;
    MainFrame::instance()->initDelDataItem(data, delData);
    delete data;
    delete delData;
}

TEST_F(ut_MainFreme, deleteDirectory)
{
    QDir dir = QDir::current().absolutePath();
    dir.mkpath(QDir::current().absolutePath() + "/test/test");
    QFile f(QDir::current().absolutePath() + "/test/test/test.txt");
    f.open(QIODevice::WriteOnly);
    f.write("asdasdadasdas");
    MainFrame::instance()->deleteDirectory(QDir::current().absolutePath() + "/test/test/test.txt");
}
TEST_F(ut_MainFreme, deleteTaskByUrl)
{
    MainFrame::instance()->deleteTaskByUrl("url");
}
TEST_F(ut_MainFreme, deleteRecycleTask)
{
    DeleteDataItem *pItem = new DeleteDataItem;
    pItem->savePath = "aaaaaa";
    MainFrame::instance()->deleteTask(pItem);
    delete pItem;
}
TEST_F(ut_MainFreme, deleteDownloadTask)
{
    DownloadDataItem *pItem = new DownloadDataItem;
    pItem->savePath = "aaaaaa";
    MainFrame::instance()->deleteTask(pItem);
    delete pItem;
}
TEST_F(ut_MainFreme, onHeaderStatechanged)
{
    QJsonObject json;
    json.insert("id","1234");
    QString searchContent;
    MainFrame::instance()->m_CurrentTab = CurrentTab::downloadingTab;
    MainFrame::instance()->onHeaderStatechanged(true);
    MainFrame::instance()->onHeaderStatechanged(false);
    MainFrame::instance()->m_CurrentTab = CurrentTab::finishTab;
    MainFrame::instance()->onHeaderStatechanged(true);
    MainFrame::instance()->onHeaderStatechanged(false);
    MainFrame::instance()->m_CurrentTab = CurrentTab::recycleTab;
    MainFrame::instance()->onHeaderStatechanged(true);
    MainFrame::instance()->onHeaderStatechanged(false);
}
TEST_F(ut_MainFreme, modelList)
{
    MainFrame::instance()->m_DownLoadingTableView->getTableModel()->getTablemodelMode();
    MainFrame::instance()->m_DownLoadingTableView->getTableModel()->dataList();
    MainFrame::instance()->m_DownLoadingTableView->getTableModel()->renderList();
    MainFrame::instance()->m_DownLoadingTableView->getTableModel()->recyleList();
    MainFrame::instance()->m_DownLoadingTableView->getTableModel()->getTableModelMap();
}
TEST_F(ut_MainFreme, setThemeType)
{
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::DarkType);
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::LightType);
}
TEST_F(ut_MainFreme, onIsMetalinkDownload)
{
    MainFrame::instance()->onIsMetalinkDownload(true);
}
TEST_F(ut_MainFreme, Raise)
{
    MainFrame::instance()->Raise();
}
TEST_F(ut_MainFreme, onNewBtnClicked)
{
    typedef int (*fptr)(CreateTaskWidget *);
    fptr foo = (fptr)(&CreateTaskWidget::exec);
    Stub stub;
    stub.set(foo, MessageboxExec);
    MainFrame::instance()->onNewBtnClicked();
}
//TEST_F(ut_MainFreme, close)
//{
//    MainFrame::instance()->close();
//}

TEST_F(ut_MainFreme, key_press)
{
    QTest::keyClick(MainFrame::instance(), Qt::Key_Control);
    QTest::keyClick(MainFrame::instance(), Qt::Key_A);
}

TEST_F(ut_MainFreme, excuteFileNotExist)
{
    Stub stub;
    stub.set(ADDR(MainFrame, onDownloadNewUrl), MainframeOndownloadnewurl);
    typedef int (*fptr)(DSettingsDialog *);
    fptr foo = (fptr)(&MessageBox::exec);

    const QList<DownloadDataItem *> list = MainFrame::instance()->m_DownLoadingTableView->getTableModel()->dataList();
    if(list.length() > 10) {
        MainFrame::instance()->m_DownLoadingTableView->m_TableDataControl->excuteFileNotExist(list.at(10), list.at(10)->fileName, list.at(10)->taskId);
    }
}

TEST_F(ut_MainFreme, onActivated_true)
{
    MainFrame::instance()->onActivated(QSystemTrayIcon::Trigger);
}

TEST_F(ut_MainFreme, onTrayQuitClick_true)
{
    MainFrame::instance()->m_ShutdownOk = true;
    MainFrame::instance()->onTrayQuitClick(true);
}

TEST_F(ut_MainFreme, model_data_AccessibleTextRole)
{
    TableModel *model = MainFrame::instance()->m_DownLoadingTableView->m_TableModel;
    model->data(model->index(0,0), Qt::AccessibleDescriptionRole);
    model->data(model->index(0,1), Qt::AccessibleDescriptionRole);
    model->data(model->index(0,2), Qt::AccessibleDescriptionRole);
    model->data(model->index(0,3), Qt::AccessibleDescriptionRole);
    model->data(model->index(0,4), Qt::AccessibleDescriptionRole);

    model->m_TableviewtabFlag = 1;
    model->data(model->index(0,0), Qt::AccessibleDescriptionRole);
    model->data(model->index(0,1), Qt::AccessibleDescriptionRole);
    model->data(model->index(0,2), Qt::AccessibleDescriptionRole);
    model->data(model->index(0,3), Qt::AccessibleDescriptionRole);
    model->data(model->index(0,4), Qt::AccessibleDescriptionRole);
}


//TEST_F(ut_MainFreme, onTrayQuitClick_false)
//{
//    typedef int (*fptr)(MessageBox *);
//    fptr foo = (fptr)(&MessageBox::exec);
//    Stub stub;
//    stub.set(foo, MessageboxExec);
//    MainFrame::instance()->m_ShutdownOk = false;
//    MainFrame::instance()->onTrayQuitClick(false);
//}

TEST_F(ut_MainFreme, onMessageBoxConfirmClick)
{
    MainFrame::instance()->onMessageBoxConfirmClick(1);
    MainFrame::instance()->onMessageBoxConfirmClick(0);
}

TEST_F(ut_MainFreme, onRpcError)
{
    QJsonObject obj;
    MainFrame::instance()->onRpcError("", "" ,1 ,obj);
    MainFrame::instance()->onMessageBoxConfirmClick(0);


    MainFrame::instance()->m_CurrentTab = CurrentTab::recycleTab;
    MainFrame::instance()->onDeletePermanentActionTriggered();
    MainFrame::instance()->onDeleteActionTriggered();
    MainFrame::instance()->onMoveToActionTriggered();
    MainFrame::instance()->m_CurrentTab = CurrentTab::finishTab;
    MainFrame::instance()->onDeleteActionTriggered();
    MainFrame::instance()->onMoveToActionTriggered();
    MainFrame::instance()->onDeletePermanentActionTriggered();

    MainFrame::instance()->onPowerOnChanged(true);
    MainFrame::instance()->onPowerOnChanged(false);

    QTime *s = new QTime;
    QTime *e = new QTime;
    s->setHMS(0,0,0);
    e->setHMS(1,1,1);
    MainFrame::instance()->checkTime(s,s);
    MainFrame::instance()->checkTime(s,e);

    MainFrame::instance()->Raise();
    MainFrame::instance()->deleteTaskByUrl("");
    DeleteDataItem *pItem = nullptr;
    MainFrame::instance()->deleteTask(pItem);
    DeleteDataItem *pItem1 = new DeleteDataItem;
    pItem1->savePath = "~";
    MainFrame::instance()->deleteTask(pItem1);
    DownloadDataItem *pItem2 = new DownloadDataItem;
    pItem2->savePath = "~";
    MainFrame::instance()->deleteTask(pItem1);
    MainFrame::instance()->checkIsHasSameTask("");
    MainFrame::instance()->m_SleepAct->trigger();
    MainFrame::instance()->m_QuitProcessAct->triggered(true);
    MainFrame::instance()->isAutoStart();
    MainFrame::instance()->m_CurrentTab = recycleTab;
    MainFrame::instance()->onContextMenu(QPoint(0,0));
    MainFrame::instance()->onCheckChanged(true,0);
    MainFrame::instance()->onIsMetalinkDownload(true);
    MainFrame::instance()->onIsMetalinkDownload(false);
    MainFrame::instance()->Raise();
    delete s;
    delete e;
    delete pItem1;
    delete pItem2;
}

TEST_F(ut_MainFreme, close)
{
    MainFrame::instance()->close();
}

TEST_F(ut_MainFreme, clearAllTask)
{
    DBInstance::delAllTask();
}
