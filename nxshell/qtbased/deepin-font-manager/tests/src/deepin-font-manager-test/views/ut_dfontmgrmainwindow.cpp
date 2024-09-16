// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:
*
* Maintainer:
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "views/dfontmgrmainwindow.h"

#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"
#include "../third-party/stub/addr_pri.h"
#include "views/dfinstallnormalwindow.h"
#include "commonheaderfile.h"
#include "fontmanagercore.h"

#include "globaldef.h"
#include "interfaces/dfontmenumanager.h"
#include "utils.h"
#include "views/dfdeletedialog.h"
#include "views/dfontinfodialog.h"
#include "views/dfquickinstallwindow.h"

#include <QTest>
#include <QRegion>
#include <QSignalSpy>
#include <QHBoxLayout>
#include <QShortcut>
#include <QFileSystemWatcher>
#include <QDBusConnection>

#include "dobject.h"
#include <DApplication>
#include <DApplicationHelper>
#include <DFontSizeManager>
#include <DFileDialog>
#include <DIconButton>
#include <DLabel>
#include <DLineEdit>
#include <DLog>
#include <QDialog>
#include <DMenu>
#include <DSearchEdit>
#include <DSlider>
#include <DSplitter>
#include <DTitlebar>
#include <DWidgetUtil>
#include <DDesktopServices>
#include <DMessageManager>
#include <unistd.h>

namespace {

typedef int (*fptr)(QDialog *);
auto QDialog_exec = (fptr)(&QDialog::exec);

class TestDFontMgrMainWindow : public testing::Test
{

protected:
    void SetUp()
    {
        fm = new DFontMgrMainWindow();
    }
    void TearDown()
    {
        SAFE_DELETE_ELE(fm)
    }

    DFontMgrMainWindow *fm;
};

static int cnt = 0;

bool stub_false()
{
    return false;
}

bool stub_true()
{
    return true;
}

bool stub_hasfource()
{
    if (cnt == 0) {
        cnt++;
        return false;
    }
    return true;
}

void stub_return()
{
    return;
}

qint64 stub_getDiskSpace()
{

    return 1000;
}

qint64 stub_getSizeL()
{
    return 1500;
}

qint64 stub_getSizeS()
{
    return 500;
}

qint64 stub_bytesAvailable()
{
    return 1500;
}


void stub_show()
{
    return;
}

void  stub_onLeftSiderBarItemClicked()
{
    return ;
}


QList<QUrl> stub_urlssin()
{
    QList<QUrl> list;
    list << QUrl();

    return list;
}

QList<QUrl> stub_urls()
{
    QList<QUrl> list;
    list << QUrl() << QUrl();

    return list;
}

QStringList stub_checkFilesSpace()
{

    QStringList list;
    list << "first";

    return list;
}

DFontPreviewItemData stub_currModelData()
{
    DFontPreviewItemData data;
    data.strFontId = 1;
    data.fontInfo.filePath = "first";

    return data;
}


Qt::WindowStates stub_windowStateMax()
{
    return Qt::WindowMaximized;
}

Qt::WindowStates stub_windowStateNo()
{
    return Qt::WindowNoState;
}

QRegion stub_getRegin()
{

    QRect t(QPoint(42, 41), QSize(10, 10));
    QRegion re(t);
    return re;
}

DFontPreviewItemData stub_getFontData()
{
    DFontPreviewItemData data;

    data.fontData.strFontName = "first";

    data.fontInfo.filePath = "first";
    data.fontInfo.isSystemFont = false;
    return data;
}

QStringList stub_getInstalledFontsPath()
{

    QStringList list;
    list << "first";

    return list;

}

QAction *stub_exec(const QPoint &pos, QAction *at = nullptr)
{
    Q_UNUSED(pos)
    Q_UNUSED(at)

    return nullptr;
}

//QDialog::exec打桩替换函数
int stub_dialogExec()
{
    return 1;
}

void hideSpinner_stub()
{
}

QStringList checkFilesSpace_stub()
{
    QStringList strlist;
    strlist << "1233";
    return strlist;
}

void stub_onExportFont(QStringList &fontList)
{
    Q_UNUSED(fontList)
}
static QStringList g_m_installFiles;
void stub_onFileSelected(const QStringList &fileList)
{
    Q_UNUSED(fileList)
    g_m_installFiles.append("first");
}

DMenu *stub_createRightKeyMenu()
{
    DMenu *menu = new DMenu();
    menu->setTitle("test");
    return menu;
}

static DFontMenuManager::MenuAction g_menuaction;
void stub_handleAddFontEvent()
{
    g_menuaction = DFontMenuManager::M_AddFont;
}

void stub_exportFont()
{
    g_menuaction = DFontMenuManager::M_ExportFont;
}
void stub_onEnableBtnClicked()
{
    g_menuaction = DFontMenuManager::M_EnableOrDisable;
}
void stub_onCollectBtnClicked()
{
    g_menuaction = DFontMenuManager::M_Faverator;
}
void stub_showFontFilePostion()
{
    g_menuaction = DFontMenuManager::M_ShowFontPostion;
}

static QString g_windowstate;
void stub_showNormal()
{
    g_windowstate = "showNormal";
}
void stub_showMaximized()
{
    g_windowstate = "showMaximized";
}
void stub_delCurrentFont()
{
    g_menuaction = DFontMenuManager::M_DeleteFont;
    g_windowstate = "delCurrentFont";
}
void stub_onShowMessage(int successCount)
{
    Q_UNUSED(successCount)
    g_windowstate = "onShowMessage";
}
}


TEST_F(TestDFontMgrMainWindow, checkSetNextTabFocus)
{
    fm->setNextTabFocus(fm->m_ptr->addFontButton);

//    代码setfocus成功，但是检测焦点状态有问题
//    EXPECT_TRUE(fm->m_ptr->searchFontEdit->lineEdit()->hasFocus());
    fm->setNextTabFocus(fm->m_ptr->searchFontEdit->lineEdit());
    fm->setNextTabFocus(fm->m_fontPreviewListView);
    fm->setNextTabFocus(fm->m_ptr->leftSiderBar);

    Stub s;
    s.set(ADDR(QWidget, isVisible), stub_true);
    fm->setNextTabFocus(fm->m_ptr->leftSiderBar);


    fm->setNextTabFocus(fm->m_ptr->fontScaleSlider);
    fm->setNextTabFocus(fm->m_ptr->fontShowArea);
    s.reset(ADDR(QWidget, isVisible));
}

TEST_F(TestDFontMgrMainWindow, checkKeyPressEvent)
{
    Stub s;
    s.set(ADDR(QWidget, hasFocus), stub_true);

    fm->m_ptr->fontScaleSlider->setValue(10);
    QTest::keyPress(fm, Qt::Key_Down);
    EXPECT_TRUE(fm->m_ptr->fontScaleSlider->value() == 9);

    QTest::keyPress(fm, Qt::Key_Up);
    EXPECT_TRUE(fm->m_ptr->fontScaleSlider->value() == 10);
}

TEST_F(TestDFontMgrMainWindow, checkEventFilterKeypress)
{
    //没有可检测的数据
    QTest::keyPress(fm->m_ptr->searchFontEdit->lineEdit(), Qt::Key_Tab);
    QTest::keyPress(fm->m_ptr->textInputEdit->lineEdit(), Qt::Key_Tab);
    QTest::keyPress(fm->m_ptr->leftSiderBar, Qt::Key_Tab);

    QTest::keyPress(fm->m_ptr->addFontButton, Qt::Key_Right);

    QTest::keyPress(fm->m_ptr->searchFontEdit->lineEdit(), Qt::Key_Escape);
    EXPECT_TRUE(fm->m_ptr->searchFontEdit->lineEdit()->text().isEmpty());

    QTest::keyPress(fm->m_ptr->textInputEdit->lineEdit(), Qt::Key_Escape);
    EXPECT_TRUE(fm->m_ptr->textInputEdit->lineEdit()->text().isEmpty());
}

TEST_F(TestDFontMgrMainWindow, checkOnInstallWindowDestroyedNoFile)
{
    Stub s1;
    s1.set(ADDR(DFontMgrMainWindow, hideSpinner), stub_return);

    Stub s2;
    s2.set(ADDR(DFontPreviewListDataThread, onAutoDirWatchers), stub_return);

    QSignalSpy spy(DFontPreviewListDataThread::instance(), SIGNAL(requestAutoDirWatchers()));

    fm->m_installOutFileList.clear();
    SignalManager::m_isDataLoadFinish = true;
    fm->onInstallWindowDestroyed();
    EXPECT_TRUE(spy.count() == 1);

    EXPECT_TRUE(fm->m_installFinish);
    EXPECT_FALSE(fm->m_fIsInstalling);

    SignalManager::m_isDataLoadFinish = false;
    fm->onInstallWindowDestroyed();
    EXPECT_TRUE(fm->m_fontPreviewListView->getFontLoadTimer()->isActive());
    EXPECT_TRUE(spy.count() == 2);
}

TEST_F(TestDFontMgrMainWindow, checkOnInstallWindowDestroyed)
{
    fm->onInstallWindowDestroyed();

    Stub s;
    s.set(ADDR(FontManagerCore, needCache), stub_true);

    fm->m_installOutFileList.clear();
    fm->m_installOutFileList << "first";

    QSignalSpy spy(DFontPreviewListDataThread::instance(), SIGNAL(requestAutoDirWatchers()));

    fm->onInstallWindowDestroyed();

    Stub s1;
    s1.set(ADDR(QThread, isFinished), stub_false);
    fm->onInstallWindowDestroyed();

    EXPECT_TRUE(fm->m_needWaitThreadStop);
    EXPECT_TRUE(spy.count() == 2);
}

TEST_F(TestDFontMgrMainWindow, checkEventFilterFocusOut)
{
    QFocusEvent *e = new QFocusEvent(QEvent::FocusOut, Qt::ActiveWindowFocusReason);
    fm->eventFilter(fm->m_ptr->leftSiderBar, e);
    EXPECT_FALSE(fm->m_leftListViewTabFocus);

    Stub s;
    s.set(ADDR(DSplitListWidget, IsTabFocus), stub_true);

    fm->eventFilter(fm->m_ptr->leftSiderBar, e);
    EXPECT_TRUE(fm->m_currentStatus.m_IsFirstFocus);

    fm->eventFilter(fm->m_fontPreviewListView, e);
    EXPECT_FALSE(fm->m_previewListViewTabFocus);

    fm->eventFilter(fm->m_ptr->fontScaleSlider, e);
    EXPECT_TRUE(fm->m_fontPreviewListView->m_isGetFocusFromSlider);
    SAFE_DELETE_ELE(e)
}

TEST_F(TestDFontMgrMainWindow, checkEventFilterFocusIn)
{
    QSignalSpy spy(fm->m_signalManager, SIGNAL(setLostFocusState(bool)));


    QFocusEvent *e = new QFocusEvent(QEvent::FocusIn, Qt::ActiveWindowFocusReason);
    fm->m_leftListViewTabFocus = true;
    fm->eventFilter(fm->m_ptr->leftSiderBar, e);
    EXPECT_FALSE(fm->m_leftListViewTabFocus);

    fm->m_leftListViewTabFocus = false;
    fm->eventFilter(fm->m_ptr->leftSiderBar, e);
    EXPECT_TRUE(fm->m_ptr->leftSiderBar->m_IsHalfWayFocus);

    fm->eventFilter(fm->m_fontPreviewListView, e);
    EXPECT_TRUE(spy.count() == 1);
    EXPECT_FALSE(fm->m_fontPreviewListView->m_IsTabFocus);

    fm->eventFilter(fm->m_ptr->searchFontEdit->lineEdit(), e);
    EXPECT_FALSE(fm->m_isSearchLineEditMenuPoped);

    fm->eventFilter(fm->m_ptr->textInputEdit->lineEdit(), e);
    EXPECT_FALSE(fm->m_isSearchLineEditMenuPoped);

    SAFE_DELETE_ELE(e)
}

TEST_F(TestDFontMgrMainWindow, checkEventFilterMouse)
{
    QSignalSpy spy(fm->m_signalManager, SIGNAL(setLostFocusState(bool)));

    QTest::mousePress(fm->m_ptr->searchFontEdit->lineEdit(), Qt::RightButton);
    EXPECT_TRUE(fm->m_isSearchLineEditMenuPoped);

    QTest::mousePress(fm->m_ptr->textInputEdit->lineEdit(), Qt::RightButton);
    EXPECT_TRUE(fm->m_isSearchLineEditMenuPoped);

    QTest::mousePress(fm->m_ptr->addFontButton, Qt::RightButton);
}

TEST_F(TestDFontMgrMainWindow, checkCheckFilesSpace)
{
    Stub s;
    s.set(ADDR(DFontMgrMainWindow, getDiskSpace), stub_getDiskSpace);

    Stub s1;
    s1.set(ADDR(QFileInfo, size), stub_getSizeL);

    QStringList list;
    list << "first";

    EXPECT_TRUE(fm->checkFilesSpace(list).isEmpty());

    Stub s2;
    s2.set(ADDR(QFileInfo, size), stub_getSizeS);
    EXPECT_TRUE(fm->checkFilesSpace(list).contains("first"));
}

TEST_F(TestDFontMgrMainWindow, checkGetDiskSpace)
{
    Stub s;
    s.set(ADDR(QStorageInfo, bytesAvailable), stub_bytesAvailable);

    EXPECT_TRUE(1500 == fm->getDiskSpace());
    EXPECT_TRUE(1500 == fm->getDiskSpace(false));
}

TEST_F(TestDFontMgrMainWindow, checkOnPreviewTextChanged)
{
    fm->m_fontPreviewListView->m_bLoadDataFinish = true;
//    fm->onPreviewTextChanged();

    fm->m_fontPreviewListView->getFontPreviewProxyModel()->insertRows(0, 5);
    fm->onPreviewTextChanged();
    DFontPreviewProxyModel *filterModel = fm->m_fontPreviewListView->getFontPreviewProxyModel();
    QModelIndex modelIndex = filterModel->index(0, 0);
    QString itemPreviewTxt = filterModel->data(modelIndex, DFontPreviewItemDelegate::FontSizeRole).toString();
    EXPECT_TRUE(itemPreviewTxt == QLatin1String("30"));

    fm->onPreviewTextChanged("first");
    fm->onPreviewTextChanged();
    filterModel = fm->m_fontPreviewListView->getFontPreviewProxyModel();
    modelIndex = filterModel->index(0, 0);
    itemPreviewTxt = filterModel->data(modelIndex, DFontPreviewItemDelegate::FontPreviewRole).toString();
    EXPECT_TRUE(itemPreviewTxt == QLatin1String("first"));
}

TEST_F(TestDFontMgrMainWindow, checkWaitForInsert)
{
    fm->m_waitForInstall.clear();

    fm->waitForInsert();

    fm->m_waitForInstall.append("first");

    Stub s;
    s.set(ADDR(DFontMgrMainWindow, installFont), stub_true);
    fm->waitForInsert();

    EXPECT_TRUE(fm->m_waitForInstall.isEmpty());
}

TEST_F(TestDFontMgrMainWindow, checkHideSpinner)
{
    fm->m_cacheFinish = true;
    fm->m_installFinish = true;
    fm->m_ptr->leftSiderBar->m_isIstalling = true;
    fm->m_isNoResultViewShow = true;

    fm->hideSpinner();

    EXPECT_FALSE(fm->m_isNoResultViewShow);
    EXPECT_FALSE(fm->m_ptr->leftSiderBar->m_isIstalling);
    //定时器触发lambda函数，无法测试到
}



TEST_F(TestDFontMgrMainWindow, checkShowInstalledFiles)
{
    Stub s;
    s.set(ADDR(DFontMgrMainWindow, onLeftSiderBarItemClicked), stub_onLeftSiderBarItemClicked);

    fm->showInstalledFiles();
    EXPECT_TRUE(fm->m_ptr->leftSiderBar->m_LastPageNumber == 2);
}

TEST_F(TestDFontMgrMainWindow, checkResizeEvent)
{

    QResizeEvent *e = new QResizeEvent(QSize(), QSize());

    fm->resizeEvent(e);
    EXPECT_TRUE(fm->m_winHight == 480);
    EXPECT_TRUE(fm->m_winWidth == 640);
    EXPECT_FALSE(fm->m_IsWindowMax);

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenRect =  screen->availableVirtualGeometry();
    fm->resize(screenRect.size());
    fm->resizeEvent(e);
    EXPECT_TRUE(fm->m_IsWindowMax);

    SAFE_DELETE_ELE(e)
}

TEST_F(TestDFontMgrMainWindow, checkDropEvent)
{
    QPointF p(300, 300);
    QMimeData data;

    QSignalSpy spy(fm, SIGNAL(fileSelected(const QStringList &, bool)));

    QDropEvent *e = new QDropEvent(p, Qt::CopyAction, &data, Qt::LeftButton, Qt::NoModifier);
    fm->dropEvent(e);
    EXPECT_FALSE(e->isAccepted());

    Stub s;
    s.set(ADDR(QMimeData, hasUrls), stub_true);

    Stub s1;
    s1.set(ADDR(QMimeData, urls), stub_urls);

    Stub s2;
    s2.set(ADDR(Utils, isFontMimeType), stub_true);

    Stub s3;
    s3.set(ADDR(DFontMgrMainWindow, installFont), stub_true);

    fm->dropEvent(e);
    EXPECT_TRUE(e->isAccepted());

    SAFE_DELETE_ELE(e)
}

TEST_F(TestDFontMgrMainWindow, checkDragEnterEvent)
{

    QPoint p(300, 300);
    QMimeData data;

    QDragEnterEvent *e = new QDragEnterEvent(p, Qt::CopyAction, &data, Qt::LeftButton, Qt::NoModifier);
    fm->dropEvent(e);
    EXPECT_FALSE(e->isAccepted());

    Stub s;
    s.set(ADDR(QMimeData, hasUrls), stub_true);

    Stub s1;
    s1.set(ADDR(QMimeData, urls), stub_urls);

    Stub s2;
    s2.set(ADDR(Utils, isFontMimeType), stub_true);

    Stub s3;
    s3.set(ADDR(DFontMgrMainWindow, installFont), stub_true);

    fm->dragEnterEvent(e);
    EXPECT_TRUE(e->isAccepted());

    Stub s4;
    s4.set(ADDR(QMimeData, urls), stub_urlssin);
    fm->dragEnterEvent(e);
    EXPECT_TRUE(e->isAccepted());

    SAFE_DELETE_ELE(e)
}

TEST_F(TestDFontMgrMainWindow, checkShowExportFontMessage)
{
    Stub s1;
    s1.set((void(DMessageManager::*)(QWidget *, const QIcon &, const QString &)) ADDR(DMessageManager, sendMessage), stub_show);

    fm->showExportFontMessage(1, 0);

    fm->showExportFontMessage(2, 0);

    fm->showExportFontMessage(1, 1);

    fm->showExportFontMessage(1, 2);
}

TEST_F(TestDFontMgrMainWindow, checkExportFont)
{
    QSignalSpy spy(DFontPreviewListDataThread::instance(), SIGNAL(requestExportFont(const QStringList &)));

    Stub s;
    s.set((void(DMessageManager::*)(QWidget *, const QIcon &, const QString &)) ADDR(DMessageManager, sendMessage), stub_show);

    fm->exportFont();

    Stub s1;
    s1.set(ADDR(DFontMgrMainWindow, checkFilesSpace), stub_checkFilesSpace);
    s1.set(ADDR(DFontPreviewListDataThread, onExportFont), stub_onExportFont);

    fm->exportFont();

    EXPECT_TRUE(spy.count() == 1);
}

TEST_F(TestDFontMgrMainWindow, checkDelCurrentFont)
{
    fm->m_fontPreviewListView->m_bLoadDataFinish = true;
    fm->m_fontPreviewListView->getFontPreviewProxyModel()->insertRows(0, 5);

    Stub s;
    s.set(QDialog_exec, stub_dialogExec);

    fm->m_fontPreviewListView->selectAll();
    fm->m_fIsDeleting = DFontMgrMainWindow::UnDeleting;

    Stub s1;
    s1.set(ADDR(DFontPreviewListDataThread, getFontData), stub_getFontData);

    fm->delCurrentFont(false);
    EXPECT_TRUE(fm->m_fIsDeleting == DFontMgrMainWindow::UnDeleting);
    EXPECT_FALSE(fm->m_fontPreviewListView->m_userFontInUseSelected);
}

TEST_F(TestDFontMgrMainWindow, checkOnconfirmDelDlgAccept)
{
    QSignalSpy spy(DFontPreviewListDataThread::instance(), SIGNAL(requestRemoveFileWatchers(const QStringList &)));

    fm->onconfirmDelDlgAccept();
    EXPECT_TRUE(FontManagerCore::instance()->m_type == FontManagerCore::UnInstall);
    EXPECT_TRUE(spy.count() == 1);
}


TEST_F(TestDFontMgrMainWindow, checkOnShowMessage)
{
    QSignalSpy spy(DFontPreviewListDataThread::instance(), SIGNAL(requestExportFont(const QStringList &)));

    Stub s;
    s.set((void(DMessageManager::*)(QWidget *, const QIcon &, const QString &)) ADDR(DMessageManager, sendMessage), stub_show);

    fm->exportFont();

    fm->onShowMessage(1);

    fm->onShowMessage(2);
}

TEST_F(TestDFontMgrMainWindow, checkOnLoadStatus0)
{
    Stub s;
    s.set(ADDR(QWidget, isVisible), stub_true);

    Stub s1;
    s1.set(ADDR(QWidget, show), stub_show);

    fm->onLoadStatus(0);
    EXPECT_FALSE(fm->m_fontLoadingSpinner->isHidden());
}

TEST_F(TestDFontMgrMainWindow, checkOnLoadStatus1)
{
    Stub s;
    s.set(ADDR(QWidget, isVisible), stub_false);

    Stub s1;
    s1.set(ADDR(QWidget, show), stub_show);

    Stub s2;
    s2.set(ADDR(DFontMgrMainWindow, onFontListViewRowCountChanged), stub_show);

    fm->m_fontPreviewListView->m_bLoadDataFinish = true;

    fm->m_leftIndex = 1;

    fm->m_ptr->searchFontEdit->lineEdit()->insert("first");

    QSignalSpy spy(fm->m_ptr->searchFontEdit, SIGNAL(textChanged(const QString &)));
    fm->onLoadStatus(1);
    EXPECT_FALSE(fm->m_openfirst);
    EXPECT_TRUE(spy.count() == 1) << spy.count();
}

TEST_F(TestDFontMgrMainWindow, checkOnFontListViewRowCountChanged0)
{

//    isHidden
    Stub s;
    s.set(ADDR(QWidget, isHidden), stub_true);

    Stub s1;
    s1.set(ADDR(QWidget, isVisible), stub_true);

    fm->m_fontPreviewListView->m_bLoadDataFinish = true;

    fm->m_fontPreviewListView->getFontPreviewProxyModel()->insertRows(0, 5);
    fm->onFontListViewRowCountChanged();
    EXPECT_FALSE(fm->m_isNoResultViewShow);
}


TEST_F(TestDFontMgrMainWindow, checkOnFontListViewRowCountChanged1)
{

//    isHidden
    Stub s;
    s.set(ADDR(QWidget, isHidden), stub_true);

    Stub s1;
    s1.set(ADDR(QWidget, isVisible), stub_true);

    fm->m_fontPreviewListView->m_bLoadDataFinish = true;
    fm->m_searchTextStatusIsEmpty = false;
    fm->m_isNoResultViewShow = false;
    fm->m_fIsInstalling = false;

    fm->onFontListViewRowCountChanged();
    EXPECT_TRUE(fm->m_isNoResultViewShow);

    fm->m_isNoResultViewShow = true;
    fm->onFontListViewRowCountChanged();
    EXPECT_TRUE(fm->m_isNoResultViewShow);
}

TEST_F(TestDFontMgrMainWindow, checkOnFontListViewRowCountChanged2)
{

//    isHidden
    Stub s;
    s.set(ADDR(QWidget, isHidden), stub_true);

    Stub s1;
    s1.set(ADDR(QWidget, isVisible), stub_true);

    fm->m_fontPreviewListView->m_bLoadDataFinish = true;
    fm->onFontListViewRowCountChanged();

    EXPECT_FALSE(fm->m_isNoResultViewShow);
}

TEST_F(TestDFontMgrMainWindow, checkOnUninstallFcCacheFinish)
{
    fm->m_fIsDeleting = 1;
    fm->onUninstallFcCacheFinish();

    EXPECT_TRUE(fm->m_fIsDeleting == 0) << fm->m_fIsDeleting;
}

TEST_F(TestDFontMgrMainWindow, checkOnFontInstallFinished)
{
    fm->onFontInstallFinished(QStringList());

    EXPECT_TRUE(fm->m_isInstallOver);
}

TEST_F(TestDFontMgrMainWindow, checkOnLeftSiderBarItemClicked)
{
    Stub s;
    s.set(ADDR(QWidget, hasFocus), stub_true);

    Stub s1;
    s1.set(ADDR(QWidget, isVisible), stub_true);

    fm->m_fontPreviewListView->m_bLoadDataFinish = false;
    fm->onLeftSiderBarItemClicked(2);
    EXPECT_TRUE(fm->m_leftIndex == 2);

    fm->m_fontPreviewListView->m_bLoadDataFinish = true;
    fm->onLeftSiderBarItemClicked(2);
    EXPECT_TRUE(fm->m_leftIndex == 2);
    EXPECT_TRUE(fm->filterGroup  == DSplitListWidget::UserFont);
}

//static 重载函数打桩
TEST_F(TestDFontMgrMainWindow, checkShowFontFilePostion)
{
    Stub s;
    s.set(ADDR(DFontPreviewListView, currModelData), stub_currModelData);

    Stub s1;
    typedef bool (*FP)(QString, const QString &);

    s1.set(FP ADDR(DDesktopServices, showFileItem), stub_return);
    fm->showFontFilePostion();
}

TEST_F(TestDFontMgrMainWindow, checkOnFontSizeChanged)
{
    {
        fm->m_fontPreviewListView->m_bLoadDataFinish = false;
        fm->onFontSizeChanged(20);
        DFontPreviewProxyModel *filterModel = fm->m_fontPreviewListView->getFontPreviewProxyModel();
        EXPECT_FALSE(filterModel == nullptr);
    }

    {
        fm->m_fontPreviewListView->m_bLoadDataFinish = true;
        fm->m_fontPreviewListView->getFontPreviewProxyModel()->insertRows(0, 5);
        fm->onFontSizeChanged(30);
        DFontPreviewProxyModel *filterModel = fm->m_fontPreviewListView->getFontPreviewProxyModel();
        QModelIndex modelIndex = filterModel->index(4, 0);
        EXPECT_TRUE(filterModel->data(modelIndex, DFontPreviewItemDelegate::FontSizeRole).toInt() == 30);
    }
}

TEST_F(TestDFontMgrMainWindow, checkOnPreviewTextChangedR)
{
    fm->onPreviewTextChanged("first");

    EXPECT_TRUE(fm->m_previewText == "first");
}

TEST_F(TestDFontMgrMainWindow, checkOnSearchTextChanged)
{
    fm->m_fontPreviewListView->m_bLoadDataFinish = false;
    fm->onSearchTextChanged("first");

    fm->m_fontPreviewListView->m_bLoadDataFinish = true;
    fm->onSearchTextChanged("first");
    DFontPreviewProxyModel *filterModel = fm->m_fontPreviewListView->getFontPreviewProxyModel();

    EXPECT_TRUE(filterModel->filterKeyColumn() == 0);
    EXPECT_FALSE(fm->m_searchTextStatusIsEmpty);
}

TEST_F(TestDFontMgrMainWindow, checkSetDeleteFinish)
{
    fm->setDeleteFinish();

    EXPECT_FALSE(fm->m_fIsDeleting);
}

TEST_F(TestDFontMgrMainWindow, checkForceNoramlInstalltionQuitIfNeeded)
{
    Stub s;
    s.set(ADDR(DFInstallNormalWindow, breakInstalltion), stub_return);

    fm->m_fIsInstalling = true;
    fm->m_fontPreviewListView->m_IsTabFocus = true;
    fm->m_dfNormalInstalldlg = new DFInstallNormalWindow(QStringList(), fm);
    fm->forceNoramlInstalltionQuitIfNeeded();
    EXPECT_FALSE(fm->m_fontPreviewListView->m_IsTabFocus);
}

//这个函数中主要是完全无用的代码，暂时标注掉
TEST_F(TestDFontMgrMainWindow, checkInitQuickWindowIfNeeded)
{
    Stub s;
    s.set(ADDR(QWidget, show), stub_return);

    Stub s1;
    s.set(ADDR(QWidget, raise), stub_return);

    Stub s2;
    s.set(ADDR(QWidget, activateWindow), stub_return);
    s.set(ADDR(DFQuickInstallWindow, onFileSelected), stub_onFileSelected);
    fm->InitQuickWindowIfNeeded();
    QStringList files;
    Q_EMIT fm->quickModeInstall(files);
    EXPECT_TRUE(g_m_installFiles.first() == QLatin1String("first"));

}

//setQuickInstallMode
TEST_F(TestDFontMgrMainWindow, checkSetQuickInstallMode)
{
    fm->setQuickInstallMode(false);

    EXPECT_FALSE(fm->m_isQuickMode);
}

TEST_F(TestDFontMgrMainWindow, checkInitRightKeyMenu)
{
    Stub s;
    s.set(ADDR(DFontMenuManager, createRightKeyMenu), stub_createRightKeyMenu);
    fm->initRightKeyMenu();
    EXPECT_TRUE(fm->d_func()->rightKeyMenu->title() == "test");
    SAFE_DELETE_ELE(fm->d_func()->rightKeyMenu)
}

TEST_F(TestDFontMgrMainWindow, checkInstallFontFromSys)
{
    Stub s;
    s.set(ADDR(DFontMgrMainWindow, installFont), stub_return);

    QSignalSpy spy(fm->m_signalManager, SIGNAL(installDuringPopErrorDialog(const QStringList &)));

    QStringList list;
    list << "first";

    fm->m_fontPreviewListView->m_bLoadDataFinish = false;
    fm->m_bLoadLeftFontsFinsihFlag = true;
    fm->installFontFromSys(list);
    EXPECT_TRUE(fm->m_waitForInstall.contains("first"));

    fm->m_fontPreviewListView->m_bLoadDataFinish = true;
    fm->m_fIsDeleting = true;
    fm->m_bLoadLeftFontsFinsihFlag = true;
    fm->installFontFromSys(list);
    EXPECT_TRUE(fm->m_waitForInstall.contains("first"));

    fm->m_fontPreviewListView->m_bLoadDataFinish = true;
    fm->m_fIsDeleting = false;
    fm->m_isPopInstallErrorDialog = true;
    fm->m_bLoadLeftFontsFinsihFlag = true;
    fm->installFontFromSys(list);
    EXPECT_TRUE(spy.count() == 1);

    fm->m_fontPreviewListView->m_bLoadDataFinish = true;
    fm->m_fIsDeleting = false;
    fm->m_isPopInstallErrorDialog = false;
    fm->m_bLoadLeftFontsFinsihFlag = true;
    fm->installFontFromSys(list);
}


TEST_F(TestDFontMgrMainWindow, checkHandleMenuEvent)
{
    Stub s;
    Stub s2;

    QList<QAction *> actionList = fm->m_ptr->rightKeyMenu->actions();

    s.set(ADDR(DFontMgrMainWindow, handleAddFontEvent), stub_handleAddFontEvent);
    fm->handleMenuEvent(actionList.first());
    EXPECT_TRUE(g_menuaction == DFontMenuManager::M_AddFont);

    s.set(ADDR(DFontPreviewListView, onEnableBtnClicked), stub_onEnableBtnClicked);
    s2.set(ADDR(DFontPreviewListView, syncTabStatus), stub_return);
    fm->handleMenuEvent(actionList.at(2));
    EXPECT_TRUE(g_menuaction == DFontMenuManager::M_EnableOrDisable);

    s.set(ADDR(DFontMgrMainWindow, delCurrentFont), stub_delCurrentFont);
    fm->handleMenuEvent(actionList.at(3));
    EXPECT_TRUE(g_menuaction == DFontMenuManager::M_DeleteFont);

    s.set(ADDR(DFontMgrMainWindow, exportFont), stub_exportFont);
    fm->handleMenuEvent(actionList.at(4));
    EXPECT_TRUE(g_menuaction == DFontMenuManager::M_ExportFont);

    s.set(ADDR(DFontPreviewListView, onCollectBtnClicked), stub_onCollectBtnClicked);
    fm->handleMenuEvent(actionList.at(5));
    EXPECT_TRUE(g_menuaction == DFontMenuManager::M_Faverator);

//    s.set(ADDR(DFontMgrMainWindow, showFontFilePostion), stub_showFontFilePostion);
//    fm->handleMenuEvent(actionList.last());
//    EXPECT_TRUE(g_menuaction == DFontMenuManager::M_ShowFontPostion);
}

TEST_F(TestDFontMgrMainWindow, checkinitShortcutsFontSize)
{
    fm->m_previewFontSize = 10;

    emit fm->m_scZoomIn->activated();
    EXPECT_TRUE(fm->m_previewFontSize == 11);

    emit fm->m_scZoomOut->activated();
    EXPECT_TRUE(fm->m_previewFontSize == 10);

    emit fm->m_scDefaultSize->activated();
    EXPECT_TRUE(fm->m_previewFontSize == DFontMgrMainWindow::DEFAULT_FONT_SIZE);
}


TEST_F(TestDFontMgrMainWindow, checkInlineFunction)
{
    fm->filterGroup = DSplitListWidget::AllFont;
    EXPECT_TRUE(fm->currentFontGroup() == DSplitListWidget::AllFont);

    fm->m_winHight = 10;
    EXPECT_TRUE(fm->getWinHeight() == 10);

    fm->m_winWidth = 10;
    EXPECT_TRUE(fm->getWinWidth() == 10);

    fm->m_IsWindowMax = false;
    EXPECT_FALSE(fm->getIsMaximized());
}


TEST_F(TestDFontMgrMainWindow, checkinitShortcutsReSize)
{
    Stub s;
    s.set(ADDR(QWidget, windowState), stub_windowStateMax);
    s.set(ADDR(QWidget, showNormal), stub_showNormal);
    emit fm->m_scWndReize->activated();
    EXPECT_TRUE("showNormal" == g_windowstate);

    Stub s1;
    s1.set(ADDR(QWidget, windowState), stub_windowStateNo);
    s1.set(ADDR(QWidget, showMaximized), stub_showMaximized);
    emit fm->m_scWndReize->activated();
    EXPECT_TRUE("showMaximized" == g_windowstate);

}


TEST_F(TestDFontMgrMainWindow, checkinitShortcutsDel)
{
    Stub s;
    Stub s1;

    s.set(ADDR(DFontPreviewListView, syncRecoveryTabStatus), stub_return);
    s1.set(ADDR(DFontMgrMainWindow, delCurrentFont), stub_delCurrentFont);

    fm->m_cacheFinish = false;
    fm->m_installFinish = false;

    emit fm->m_scDeleteFont->activated();
    EXPECT_TRUE("delCurrentFont" == g_windowstate);
}

TEST_F(TestDFontMgrMainWindow, checkinitShortcutsMenu)
{
    Stub s;
    Stub s1;

    QSignalSpy spy(fm->m_scShowMenu, SIGNAL(activated()));
    s.set(ADDR(QWidget, hasFocus), stub_hasfource);
    s1.set(ADDR(QApplication, sendEvent), stub_return);

    fm->m_isSearchLineEditMenuPoped = false;

    emit fm->m_scShowMenu->activated();
    EXPECT_TRUE(spy.count() == 1);
}

TEST_F(TestDFontMgrMainWindow, checkinitShortcutsAdd)
{
    Stub s;
    Stub s1;

    QSignalSpy spy(fm->m_scAddNewFont, SIGNAL(activated()));

    s.set(ADDR(DFontPreviewListView, syncRecoveryTabStatus), stub_return);
    s1.set(ADDR(DFontMgrMainWindow, handleAddFontEvent), stub_return);

    emit fm->m_scAddNewFont->activated();
    EXPECT_TRUE(spy.count() == 1);
}


TEST_F(TestDFontMgrMainWindow, checkinitShortcutsElse)
{
    Stub s;
    s.set(ADDR(DFontMgrMainWindow, showAllShortcut), stub_return);

    Stub s1;
    s1.set(ADDR(DFontPreviewListView, syncRecoveryTabStatus), stub_return);

    fm->m_fontPreviewListView->m_bLoadDataFinish = true;

    fm->m_fontPreviewListView->getFontPreviewProxyModel()->insertRows(0, 5);

    emit fm->m_scShowAllSC->activated();

    emit fm->m_scPageUp->activated();

    emit fm->m_scPageDown->activated();

    emit fm->m_scFindFont->activated();

    emit fm->m_scFontInfo->activated();
}

TEST_F(TestDFontMgrMainWindow, checkonRightMenuShortCutActivated)
{
    Stub s;
    s.set((QAction * (QMenu::*)(const QPoint &, QAction *))ADDR(QMenu, exec), stub_exec);

    fm->m_fontPreviewListView->getFontPreviewProxyModel()->insertRows(0, 5);
    fm->m_fontPreviewListView->selectAll();

    fm->m_fontPreviewListView->onRightMenuShortCutActivated();
    EXPECT_FALSE(fm->m_fontPreviewListView->m_isMousePressNow);

    Stub s1;
    s1.set(ADDR(QWidget, visibleRegion), stub_getRegin);
    fm->m_fontPreviewListView->onRightMenuShortCutActivated();
    EXPECT_FALSE(fm->m_fontPreviewListView->m_isMousePressNow);
}

TEST_F(TestDFontMgrMainWindow, checkononCacheFinish)
{
    Stub s;
    s.set(ADDR(DFontMgrMainWindow, hideSpinner), hideSpinner_stub);

    fm->onCacheFinish();
    ASSERT_EQ(fm->m_cacheFinish, true);
}

TEST_F(TestDFontMgrMainWindow, handleAddFontEvent)
{
    Stub s;
    s.set(ADDR(DFontMgrMainWindow, onShowMessage), stub_onShowMessage);

    QStringList filelist;
    fm->m_fIsInstalling = true;
    fm->installFont(filelist, false);
    EXPECT_TRUE(g_windowstate == "onShowMessage");

    s.set(ADDR(DFontMgrMainWindow, checkFilesSpace), checkFilesSpace_stub);
    fm->m_cacheFinish = true;
    fm->m_installFinish = true;
    fm->installFont(filelist, false);
    EXPECT_TRUE(fm->m_cacheFinish);
    EXPECT_TRUE(fm->m_installFinish);

    s.set(ADDR(DFontPreviewListView, currModelData), stub_currModelData);
    fm->onRightMenuAboutToShow();
    EXPECT_FALSE(fm->m_hasMenuTriggered);
    EXPECT_TRUE(fm->m_menuCurData.fontInfo.filePath == QLatin1String("first"));

    fm->onRightMenuAboutToHide();
    fm->onRequestInstFontsUiAdded();
    EXPECT_TRUE(fm->m_installFinish);

    fm->onFontChanged();
    EXPECT_TRUE(fm->d_func()->fontSizeLabel->width() == 65);
    EXPECT_TRUE(fm->d_func()->fontSizeLabel->text() == QLatin1String("30px"));
}

