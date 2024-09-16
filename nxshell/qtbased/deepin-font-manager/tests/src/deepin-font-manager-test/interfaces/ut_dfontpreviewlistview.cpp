// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfontpreviewlistview.h"
#include "dfontinfomanager.h"
#include "dfontpreviewitemdelegate.h"
#include "globaldef.h"
#include "dfmxmlwrapper.h"
#include "dcomworker.h"
#include "views/dfontmgrmainwindow.h"
#include "commonheaderfile.h"

#include <DLog>
#include <DMenu>
#include <DGuiApplicationHelper>
#include <DApplication>
#include <DMessageManager>

#include <QSignalSpy>
#include <QFontDatabase>
#include <QSet>
#include <QScroller>
#include <QTest>

#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"

namespace {
class TestDFontPreviewListView : public testing::Test
{

protected:
    void SetUp()
    {
        fmmw = new DFontMgrMainWindow();
        listview = new DFontPreviewListView(fmmw);
        fmmw->d_func()->settingsQsPtr->setValue(FTM_MWSIZE_W_KEY, DEFAULT_WINDOWS_WIDTH);
        fmmw->d_func()->settingsQsPtr->setValue(FTM_MWSIZE_H_KEY, DEFAULT_WINDOWS_HEIGHT);
        fmmw->m_winHight = DEFAULT_WINDOWS_HEIGHT;
        fmmw->m_winWidth = DEFAULT_WINDOWS_WIDTH;
    }
    void TearDown()
    {
        SAFE_DELETE_ELE(fmmw)
    }
    QString filepath = "/home/zhaogongqiang/Desktop/1048字体/ArkanaScriptRough.otf";
    DFontPreviewListView *listview;
    DFontMgrMainWindow *fmmw;
};

//参数化测试
class TestUpdateCurrentFontGroup : public::testing::TestWithParam<int>
{
public:
    DFontMgrMainWindow *mw = new DFontMgrMainWindow;
    DFontPreviewListView *listview = new DFontPreviewListView();
};

bool stub_False()
{
    return false;
}

bool stub_True()
{
    qDebug() << "!!!!!!!!!!!11111" << endl;
    return true;
}

void  stub_Return()
{
    return ;
}

QStringList  stub_currentFontList()
{
    QStringList strlist;
    return strlist;
}


QStringList  stub_currentFontListvalue()
{
    QStringList strlist;
    strlist << "a" << "b" << "c";
    return strlist;
}


QList<DFontPreviewItemData>  stub_DFontPreviewItemDatavalue()
{
    DFontPreviewItemData dpitemdata;
    dpitemdata.fontInfo.filePath = "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttcc";

    DFontPreviewItemData dpitemdata1;
    dpitemdata1.fontInfo.filePath = "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc";
    dpitemdata1.fontInfo.isSystemFont = false;
    dpitemdata1.fontInfo.familyName = "b";

    DFontPreviewItemData dpitemdata2;
    dpitemdata2.fontInfo.filePath = "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc";
    dpitemdata2.fontInfo.isSystemFont = false;
    dpitemdata2.fontInfo.familyName = "a";

    QList<DFontPreviewItemData> itemdata;
    itemdata.append(dpitemdata);
    itemdata.append(dpitemdata1);
    itemdata.append(dpitemdata2);
    return itemdata;
}

bool  stub_Returnfalse()
{
    return false;
}

int  stub_count()
{
    return 25;
}

QAction  *stub_QMenuAction()
{
    return nullptr;
}

QModelIndex stub_QModelIndexLast()
{
    QModelIndex index;
    return index;
}

QPoint stub_pos()
{
    return QPoint(626, 22);
}

QModelIndex stub_indexAt(const QPoint &)
{
    return QModelIndex(0, 0, nullptr, nullptr);
}
QList<DFontPreviewItemData> stub_getFontModelList()
{
    QList<DFontPreviewItemData> list;

    DFontPreviewItemData data;
    data.fontInfo.filePath = "first";

    list << data;
    return list;
}

DFontPreviewItemData stub_getFontData()
{
    DFontPreviewItemData data;
    data.fontInfo.filePath = "first";

    return data;
}

DFontPreviewItemData stub_getFontDataPath()
{
    DFontPreviewItemData data;
    data.fontInfo.filePath = "/usr/share/fonts/opentype/noto/NotoSansCJK-Bold.ttcc";

    return data;
}

DFontPreviewItemData stub_returnSysItemdata()
{
    DFontPreviewItemData data;
    data.fontInfo.filePath = "first";
    data.fontInfo.isSystemFont = true;
    return data;
}

DFontPreviewItemData stub_returnNotSysItemdata(const FontData &fontData)
{
    DFontPreviewItemData data;
    if (fontData.strFontName == QLatin1String("curfont")) {
        data.fontInfo.filePath = "curfontpath";
    } else {
        data.fontInfo.filePath = "first";
    }
    data.fontInfo.isSystemFont = false;
    data.fontData.strFontName = "second";
    return data;
}

IconStatus stub_getHoverState()
{
    return IconStatus::IconPress;
}

int stub_height()
{
    return 5 * FTM_PREVIEW_ITEM_HEIGHT;
}

void stub_start(QThread::Priority)
{

}
}

TEST_F(TestDFontPreviewListView, checkInitFontListData)
{
    QSignalSpy spy(listview, SIGNAL(onLoadFontsStatus(int)));
    listview->initFontListData();
    EXPECT_TRUE(spy.count() == 1) << spy.count() << "checkInitFontListData++";
}

TEST_F(TestDFontPreviewListView, checkOnItemRemoved)
{
    DFontPreviewItemData data;
    data.fontInfo.filePath = filepath;
    data.appFontId = 1;
    listview->onItemRemoved(data);
}

TEST_F(TestDFontPreviewListView, checkOnItemRemovedSys)
{
    DFontPreviewItemData data;
    data.fontInfo.filePath = filepath;
    data.appFontId = 1;
}


INSTANTIATE_TEST_CASE_P(HandleTrueReturn, TestUpdateCurrentFontGroup, testing::Values(0, 1, 2, 3, 4, 5, 6));


TEST_F(TestDFontPreviewListView, checkCount)
{
    qDebug() << listview->model()->insertRow(0);

    listview->count();
    EXPECT_TRUE(1 == listview->count());
}

TEST_F(TestDFontPreviewListView, checkCancelDel)
{
    listview->cancelDel();
    EXPECT_TRUE(listview->m_selectAfterDel == -1);
}

TEST_F(TestDFontPreviewListView, checkViewChanged)
{
    listview->viewChanged();

    EXPECT_TRUE(listview->m_selectAfterDel == -1);
}

TEST_F(TestDFontPreviewListView, checkMarkPositionBeforeRemoved_isdelete)
{
    listview->model()->insertRow(0);
    QModelIndex index = listview->model()->index(0, 0);

    Stub s;
    s.set(ADDR(DFontPreviewListDataThread, getFontData), stub_returnSysItemdata);

    listview->selectionModel()->select(index, QItemSelectionModel::Select);
    listview->markPositionBeforeRemoved();
    EXPECT_TRUE(listview->m_selectAfterDel == -2);
    s.reset(ADDR(DFontPreviewListDataThread, getFontData));

    s.set(ADDR(DFontPreviewListDataThread, getFontData), stub_returnNotSysItemdata);
    listview->markPositionBeforeRemoved();
    EXPECT_TRUE(listview->m_selectAfterDel == 0);
}

TEST_F(TestDFontPreviewListView, checkGetOnePageCount)
{
    //返回默认值
    int count = listview->getOnePageCount();
    EXPECT_TRUE(count == 12);

    Stub s;
    s.set(ADDR(QWidget, height), stub_height); //TODO: gerrit这里打桩进不去桩函数

    listview->m_fontPreviewProxyModel->insertRows(0, 5);
    count = listview->getOnePageCount();
//    EXPECT_TRUE(count == 5);
}

TEST_F(TestDFontPreviewListView, checkAppendFilePath)
{
    QStringList list;
    list << "aaa" << "bbb";
    listview->appendFilePath(&list, "ccc");
    EXPECT_TRUE(list.count() == 3);

    listview->appendFilePath(&list, "aaa");
    EXPECT_TRUE(list.count() == 3);
}

TEST_F(TestDFontPreviewListView, checkGetCurFontStrName)
{
    FontData d;
    d.setEnabled(true);

    listview->m_curFontData = d;

    EXPECT_TRUE(listview->getCurFontData().isEnabled());
}


TEST_F(TestDFontPreviewListView, checkSetRecoveryTabFocusState)
{
    listview->setRecoveryTabFocusState(false);
    EXPECT_FALSE(listview->m_recoveryTabFocusState);
}

TEST_F(TestDFontPreviewListView, checkSetIsTabFocus)
{
    listview->setIsTabFocus(false);
    EXPECT_FALSE(listview->getIsTabFocus());
}

TEST_F(TestDFontPreviewListView, checkUpdateSpinner)
{
    QSignalSpy s(listview, SIGNAL(requestShowSpinner(bool, bool, DFontSpinnerWidget::SpinnerStyles)));
    listview->m_curTm = QDateTime::currentMSecsSinceEpoch();
    usleep(360000);
    DFontSpinnerWidget::SpinnerStyles style = DFontSpinnerWidget::SpinnerStyles::StartupLoad;
    listview->updateSpinner(style, true);
    EXPECT_TRUE(s.count() == 1) << s.count();
}

TEST_F(TestDFontPreviewListView, checkUpdateModel)
{

    QSignalSpy s1(listview, SIGNAL(requestShowSpinner(bool, bool, DFontSpinnerWidget::SpinnerStyles)));
    QSignalSpy s2(listview, SIGNAL(rowCountChanged()));
    QSignalSpy s3(listview, SIGNAL(deleteFinished()));
    QSignalSpy s4(listview->m_signalManager, SIGNAL(fontSizeRequestToSlider()));


    listview->updateModel(false, true);

    EXPECT_TRUE(s1.count() == 1) << s1.count();
    EXPECT_TRUE(s2.count() == 1) << s2.count();
    EXPECT_TRUE(s3.count() == 1) << s3.count();
    EXPECT_TRUE(s4.count() == 1) << s4.count();
}

TEST_F(TestDFontPreviewListView, checkOnUpdateCurrentFont)
{
    listview->m_fontChanged = true;
    listview->onUpdateCurrentFont();
    EXPECT_FALSE(listview->m_fontChangeTimer->isActive());

    Stub s1;
    s1.set(ADDR(DFontInfoManager, getCurrentFontFamily), stub_currentFontList);
    listview->onUpdateCurrentFont();
    s1.reset(ADDR(DFontInfoManager, getCurrentFontFamily));
    s1.set(ADDR(DFontInfoManager, getCurrentFontFamily), stub_currentFontListvalue);
    listview->m_fontChanged = true;
    listview->m_currentFont << "a" << "b" << "c";
    listview->m_tryCnt = 2;
    listview->onUpdateCurrentFont();
    listview->m_fontChanged = false;
    listview->onUpdateCurrentFont();

    listview->m_fontChanged = true;
    listview->m_tryCnt = 0;
    listview->m_currentFont << "a" << "b";
    s1.set(ADDR(DFontPreviewListDataThread, getFontModelList), stub_DFontPreviewItemDatavalue);

    listview->onUpdateCurrentFont();


}


////UpdateSelection maek
TEST_F(TestDFontPreviewListView, checkUpdateSelection)
{
    QStandardItemModel *m_fontPreviewItemModel = new QStandardItemModel(listview);
    m_fontPreviewItemModel->setColumnCount(1);
    DFontPreviewProxyModel *fpm = new DFontPreviewProxyModel(listview);
    fpm->setSourceModel(m_fontPreviewItemModel);

    listview->setModel(fpm);
    listview->m_fontPreviewProxyModel->insertRows(0, 5);

    listview->m_selectAfterDel = 1;

    listview->updateSelection();

    listview->m_selectAfterDel = 5;
    listview->updateSelection();


}



TEST_F(TestDFontPreviewListView, checkScrollWithTheSelected)
{
    QStandardItemModel *m_fontPreviewItemModel = new QStandardItemModel;
    m_fontPreviewItemModel->setColumnCount(1);
    DFontPreviewProxyModel *fpm = new DFontPreviewProxyModel;
    fpm->setSourceModel(m_fontPreviewItemModel);
    DFontPreviewListView *listview = new DFontPreviewListView;
    listview->setModel(fpm);
    m_fontPreviewItemModel->insertRows(0, 5);
    listview->m_fontPreviewProxyModel->insertRows(0, 5);

    listview->scrollWithTheSelected();
    EXPECT_TRUE(listview->m_currentSelectedRow == 0);

    listview->selectAll();
    listview->scrollWithTheSelected();
}

TEST_F(TestDFontPreviewListView, checkChangeFontFileForce)
{
    QStandardItemModel *m_fontPreviewItemModel = new QStandardItemModel;
    m_fontPreviewItemModel->setColumnCount(1);
    DFontPreviewProxyModel *fpm = new DFontPreviewProxyModel;
    fpm->setSourceModel(m_fontPreviewItemModel);
    DFontPreviewListView *listview = new DFontPreviewListView;
    listview->setModel(fpm);
    m_fontPreviewItemModel->insertRows(0, 5);
    listview->m_fontPreviewProxyModel->insertRows(0, 5);

    listview->changeFontFile("", true);
}

TEST_F(TestDFontPreviewListView, checkChangeFontFileUnForce)
{
    Stub s;
    s.set(ADDR(QFileInfo, isDir), stub_True);

    Stub s1;
    s1.set(ADDR(DFontPreviewListDataThread, getFontModelList), stub_getFontModelList);


    listview->changeFontFile("first", false);

}

TEST_F(TestDFontPreviewListView, checkDeleteCurFonts)
{
    QSignalSpy spy(listview, SIGNAL(requestUpdateModel(int, bool)));

    QStringList list;

    listview->deleteCurFonts(list, false);
    qDebug() << spy.count();
    EXPECT_TRUE(spy.count() == 1);

    Stub s1;
    s1.set(ADDR(DFontPreviewListDataThread, getFontModelList), stub_getFontModelList);

    list << "first";

    listview->deleteCurFonts(list, true);
    qDebug() << spy.count() << endl;
    EXPECT_TRUE(spy.count() == 2);
    list.clear();
    list << "";
    listview->deleteCurFonts(list, true);

}

TEST_F(TestDFontPreviewListView, checkUpdateChangedDir)
{
    QSignalSpy spy(listview, SIGNAL(itemRemoved(const DFontPreviewItemData &)));
    QSignalSpy spy2(listview, SIGNAL(rowCountChanged()));

    QList<DFontPreviewItemData> list;

    DFontPreviewItemData data;
    data.fontInfo.filePath = "first";

    list << data;

    listview->m_dataThread->m_fontModelList.clear();
    listview->m_dataThread->m_fontModelList = list;

    listview->updateChangedDir();

    qDebug() << spy.count() << "!!!!!!!!!!" << spy2.count() << endl;
    EXPECT_TRUE(spy.count() == 1);
    EXPECT_TRUE(spy2.count() == 1);
}

TEST_F(TestDFontPreviewListView, checkUpdateChangedFile)
{
    Stub s1;
    s1.set(ADDR(DFontPreviewListView, changeFontFile), stub_Return);


    QSignalSpy spy(listview, SIGNAL(rowCountChanged()));

    QStringList list;
    list << "first";

    listview->updateChangedFile(list);

    EXPECT_TRUE(spy.count() == 1);

}

TEST_F(TestDFontPreviewListView, checkClearHoverState)
{
    QStandardItemModel *m_fontPreviewItemModel = new QStandardItemModel;
    m_fontPreviewItemModel->setColumnCount(1);

    DFontPreviewProxyModel *fpm = new DFontPreviewProxyModel;
    fpm->setSourceModel(m_fontPreviewItemModel);

    DFontPreviewListView *listview = new DFontPreviewListView;
    listview->setModel(fpm);

    m_fontPreviewItemModel->insertRows(0, 5);
    listview->m_fontPreviewProxyModel->insertRows(0, 5);

    listview->m_hoverModelIndex = listview->m_fontPreviewProxyModel->index(2, 0);

    FontData data;
    data.setHoverState(IconHover);

    listview->m_fontPreviewProxyModel->setData(listview->m_fontPreviewProxyModel->index(2, 0), QVariant::fromValue(data), Qt::DisplayRole);

    listview->clearHoverState();
    EXPECT_TRUE(listview->m_hoverModelIndex == QModelIndex());

    FontData itemData =
        qvariant_cast<FontData>(listview->m_fontPreviewProxyModel->data(listview->m_fontPreviewProxyModel->index(2, 0)));

    EXPECT_TRUE(itemData.getHoverState() == IconNormal);
}

TEST_F(TestDFontPreviewListView, checkClearPressStateMoveClear)
{
    QStandardItemModel *m_fontPreviewItemModel = new QStandardItemModel;
    m_fontPreviewItemModel->setColumnCount(1);

    DFontPreviewProxyModel *fpm = new DFontPreviewProxyModel;
    fpm->setSourceModel(m_fontPreviewItemModel);

    DFontPreviewListView *listview = new DFontPreviewListView;
    listview->setModel(fpm);

    m_fontPreviewItemModel->insertRows(0, 5);
    listview->m_fontPreviewProxyModel->insertRows(0, 5);

    listview->m_previousPressPos = 1;

    FontData data;
    data.setHoverState(IconHover);

    QModelIndex index = listview->m_fontPreviewProxyModel->index(1, 0);
    listview->m_fontPreviewProxyModel->setData(index, QVariant::fromValue(data));

    QModelIndex index2 = listview->m_fontPreviewProxyModel->index(2, 0);
    listview->m_pressModelIndex = index2;
    listview->m_fontPreviewProxyModel->setData(index2, QVariant::fromValue(data));

    listview->clearPressState(DFontPreviewListView::MoveClear, 0);
    EXPECT_TRUE(listview->m_pressModelIndex == QModelIndex());
    FontData itemData =
        qvariant_cast<FontData>(listview->m_fontPreviewProxyModel->data(listview->m_pressModelIndex));
    EXPECT_TRUE(itemData.getHoverState() == IconNormal);
}

TEST_F(TestDFontPreviewListView, checkClearPressStatePreviousClear)
{

    listview->m_fontPreviewProxyModel->insertRows(0, 5);

    listview->m_previousPressPos = 1;

    FontData data;
    data.setHoverState(IconHover);

    QModelIndex index = listview->m_fontPreviewProxyModel->index(1, 0);
    listview->m_fontPreviewProxyModel->setData(index, QVariant::fromValue(data));

    listview->clearPressState(DFontPreviewListView::PreviousClear, 0);

    FontData itemData =
        qvariant_cast<FontData>(listview->m_fontPreviewProxyModel->data(index));
    EXPECT_TRUE(itemData.getHoverState() == IconNormal);

}

TEST_F(TestDFontPreviewListView, checkCurrModelData)
{
    Stub s;
    s.set(ADDR(DFontPreviewListDataThread, getFontData), stub_getFontData);

    listview->m_fontPreviewProxyModel->insertRows(0, 5);
    listview->m_previousPressPos = 1;

    FontData data;
    data.strFontName = "first";

    QModelIndex index = listview->m_fontPreviewProxyModel->index(1, 0);
    listview->m_fontPreviewProxyModel->setData(index, QVariant::fromValue(data));
    listview->selectionModel()->select(index, QItemSelectionModel::Select);

    DFontPreviewItemData data2 = listview->currModelData();
    EXPECT_TRUE(data2.fontInfo.filePath == "first");
}

TEST_F(TestDFontPreviewListView, checkCurrModelIndex)
{
    Stub s;
    s.set(ADDR(DFontPreviewListDataThread, getFontData), stub_getFontData);

    listview->m_fontPreviewProxyModel->insertRows(0, 5);

    QModelIndex index = listview->m_fontPreviewProxyModel->index(0, 0);
    listview->selectionModel()->select(index, QItemSelectionModel::Select);
    index = listview->m_fontPreviewProxyModel->index(1, 0);
    listview->selectionModel()->select(index, QItemSelectionModel::Select);

    QModelIndex currentIndex = listview->currModelIndex();

    EXPECT_TRUE(currentIndex.row() == 0);

}

TEST_F(TestDFontPreviewListView, checkOnCollectBtnClicked)
{
    QSignalSpy spy(listview, SIGNAL(rowCountChanged()));

    QModelIndexList list;

    listview->onCollectBtnClicked(list, true, true);
    EXPECT_TRUE(spy.count() == 0);

    Stub s;
    s.set(ADDR(DFontPreviewListDataThread, updateItemStatus), stub_Return);
    listview->m_fontPreviewProxyModel->insertRows(0, 5);

    FontData fdata;

    fdata.strFontName = "first";
    fdata.fontState = 0x01;

    listview->selectAll();

    listview->m_fontPreviewProxyModel->setData(listview->m_fontPreviewProxyModel->index(1, 0), QVariant::fromValue(fdata), Qt::DisplayRole);

    list.clear();
    list << listview->m_fontPreviewProxyModel->index(1, 0);

    listview->onCollectBtnClicked(list, true, true);

    QModelIndex index = listview->m_fontPreviewProxyModel->index(1, 0);

    FontData itemData =
        qvariant_cast<FontData>(listview->m_fontPreviewProxyModel->data(index));
    //mark
    qDebug() << itemData.fontState << endl;
    EXPECT_TRUE(itemData.fontState == 0x03);

    qDebug() << spy.count();
    EXPECT_TRUE(spy.count() == 1);
}

TEST_F(TestDFontPreviewListView, checkOnEnableBtnClickedEnable)
{
    QSignalSpy spy(listview, SIGNAL(rowCountChanged()));

    Stub s;
    s.set(ADDR(DFontPreviewListDataThread, getFontData), stub_getFontDataPath);

    QModelIndexList list;
    listview->onEnableBtnClicked(list, 0, 0, false, true);
    EXPECT_TRUE(spy.count() == 0);

    listview->m_fontPreviewProxyModel->insertRows(0, 5);

    Stub s1;
    s1.set(ADDR(DFontPreviewListDataThread, updateItemStatus), stub_Return);

    FontData fdata;
    fdata.strFontName = "first";
    fdata.fontState = 0x01;

    listview->selectAll();

    listview->m_fontPreviewProxyModel->setData(listview->m_fontPreviewProxyModel->index(1, 0), QVariant::fromValue(fdata), Qt::DisplayRole);

    list.clear();

    QModelIndex index = listview->m_fontPreviewItemModel->index(1, 0);
    list << index;
    listview->onEnableBtnClicked(list, 0, 1, true, true);

    FontData itemData =
        qvariant_cast<FontData>(listview->m_fontPreviewProxyModel->data(index));
    qDebug() << itemData.fontState << endl;
    EXPECT_TRUE(itemData.fontState == 0x00);

    DFontMgrMainWindow *mw = new DFontMgrMainWindow;
    listview->setParent(mw);
    listview->m_parentWidget = mw;
    listview->onEnableBtnClicked(list, 1, 1, false, true);
    listview->onEnableBtnClicked(list, 0, 0, false, false);
    listview->onEnableBtnClicked(list, 1, 1, false, false);
    listview->onEnableBtnClicked(list, 1, 0, false, false);

    index = listview->m_fontPreviewItemModel->index(1, 0);
    list << index;
    listview->onEnableBtnClicked(list, 0, 1, true, true);


}

TEST_F(TestDFontPreviewListView, checkOnRightMenuShortCutActivated)
{
    Stub s1;
    s1.set(ADDR(QWidget, isVisible), stub_True);

    listview->m_fontPreviewProxyModel->insertRows(0, 1);

    FontData data;
    data.strFontName = "first";

    QModelIndex index = listview->m_fontPreviewProxyModel->index(0, 0);
    listview->m_fontPreviewProxyModel->setData(index, QVariant::fromValue(data));

    DFontMgrMainWindow *mw = new DFontMgrMainWindow;
    listview->m_parentWidget = mw;
    listview->setParent(mw);
    listview->selectAll();
    listview->m_rightMenu = new QMenu(listview);
    listview->onRightMenuShortCutActivated();
}

bool stub_contains(void *, const QPoint &, bool)
{
    return true;
}
TEST_F(TestDFontPreviewListView, checkHoverState)
{
    Stub s;
    s.set(ADDR(QWidget, mapFromGlobal), stub_pos);
    typedef  QModelIndex(*fptr)(const QPoint &);
    fptr DFontPreviewListView_indexAt = (fptr)(&DFontPreviewListView::indexAt);
    s.set(DFontPreviewListView_indexAt, stub_indexAt);
    typedef bool (*fptr2)(QRect *, const QPoint &, bool);
    fptr2 QRect_contains = (fptr2)((bool(QRect::*)(const QPoint &, bool) const)&QRect::contains);  //获取虚函数地址
    s.set(QRect_contains, stub_contains);

    listview->m_fontPreviewProxyModel->insertRows(0, 1);

    listview->checkHoverState();

    EXPECT_TRUE(listview->m_hoverModelIndex.row() == 0);
}

TEST_F(TestDFontPreviewListView, checkUpdateShiftSelect)
{
    listview->m_fontPreviewProxyModel->insertRows(0, 5);

    listview->m_currentSelectedRow = 1;

    QModelIndex index = listview->m_fontPreviewProxyModel->index(1, 0);
    listview->updateShiftSelect(index);

    EXPECT_TRUE(listview->selectedIndexes().count() == 1);

    index = listview->m_fontPreviewProxyModel->index(2, 0);
    listview->updateShiftSelect(index);
    EXPECT_TRUE(listview->selectedIndexes().count() == 2);

    listview->m_currentSelectedRow = -1;
    index = listview->m_fontPreviewProxyModel->index(3, 0);
    listview->updateShiftSelect(index);
    EXPECT_TRUE(listview->selectedIndexes().count() == 4);
}

TEST_F(TestDFontPreviewListView, checkDisableFonts)
{
    Stub s1;
    s1.set(ADDR(DFMXmlWrapper, createFontConfigFile), stub_True);
    listview->disableFont("first");
    listview->disableFonts();
    EXPECT_TRUE(listview->m_disableFontList.count() == 0);

    Stub s2;
    s2.set(ADDR(DFMXmlWrapper, createFontConfigFile), stub_False);

    listview->disableFont("first");
    listview->disableFonts();
    EXPECT_TRUE(listview->m_disableFontList.count() == 1);
}

TEST_F(TestDFontPreviewListView, checkEnableFonts)
{
    Stub s1;
    s1.set(ADDR(DFMXmlWrapper, createFontConfigFile), stub_True);
    listview->enableFont("first");
    listview->enableFonts();
    EXPECT_TRUE(listview->m_enableFontList.count() == 0);

    Stub s2;
    s2.set(ADDR(DFMXmlWrapper, createFontConfigFile), stub_False);

    listview->enableFont("first");
    listview->enableFonts();
    EXPECT_TRUE(listview->m_enableFontList.count() == 1);
}

TEST_F(TestDFontPreviewListView, checkEventFilterFocusOut)
{
    QSignalSpy spy(listview->m_signalManager, SIGNAL(setLostFocusState(bool)));

    QEvent *e = new QEvent(QEvent::FocusOut);

    listview->m_isLostFocusOfLeftKey = true;
    listview->eventFilter(listview, e);
    EXPECT_FALSE(listview->m_IsTabFocus);

    SAFE_DELETE_ELE(e)
}

TEST_F(TestDFontPreviewListView, checkEventFilterFocusIn)
{
    QSignalSpy spy(listview->m_signalManager, SIGNAL(requestSetTabFocusToAddBtn()));

    QFocusEvent *e = new QFocusEvent(QEvent::FocusIn, Qt::TabFocusReason);

    listview->m_isGetFocusFromSlider = true;

    listview->eventFilter(listview, e);
    EXPECT_TRUE(spy.count() == 1);
    SAFE_DELETE_ELE(e)
}

TEST_F(TestDFontPreviewListView, checkKeyPressEventFilterEmpty)
{
    listview->m_fontPreviewProxyModel->insertRows(0, 5);

    listview->keyPressEventFilter(QModelIndexList(), true, false, false);
    EXPECT_TRUE(listview->selectedIndexes().count() == 1);
    EXPECT_TRUE(listview->selectedIndexes().first().row() == 4);

    listview->keyPressEventFilter(QModelIndexList(), false, true, false);
    EXPECT_TRUE(listview->selectedIndexes().count() == 1);
    EXPECT_TRUE(listview->selectedIndexes().first().row() == 0);
}

TEST_F(TestDFontPreviewListView, checkKeyPressEventFilterUp)
{
    listview->m_fontPreviewProxyModel->insertRows(0, 5);

    QModelIndex index = listview->m_fontPreviewProxyModel->index(1, 0);
    QModelIndexList indexList;
    indexList.append(index);

    listview->m_currentSelectedRow = 0;
    listview->keyPressEventFilter(indexList, true, false, true);
    EXPECT_TRUE(listview->selectedIndexes().count() == 1);
    EXPECT_TRUE(listview->selectedIndexes().first().row() == 0);

    listview->m_currentSelectedRow = 1;
    listview->keyPressEventFilter(indexList, true, false, true);
    EXPECT_TRUE(listview->selectedIndexes().count() == 1);
    EXPECT_TRUE(listview->selectedIndexes().first().row() == 0);

    indexList.clear();
    index = listview->m_fontPreviewProxyModel->index(0, 0);
    indexList.append(index);
    listview->keyPressEventFilter(indexList, true, false, false);

    listview->m_currentSelectedRow = 4;
    indexList.clear();
    index = listview->m_fontPreviewProxyModel->index(3, 0);
    indexList.append(index);
    listview->keyPressEventFilter(indexList, true, false, false);
    EXPECT_TRUE(listview->selectedIndexes().first().row() == 3);
    EXPECT_TRUE(listview->m_currentSelectedRow == 3);
}

TEST_F(TestDFontPreviewListView, checkKeyPressEventFilterDown)
{
    listview->m_fontPreviewProxyModel->insertRows(0, 5);

    QModelIndex index = listview->m_fontPreviewProxyModel->index(1, 0);
    QModelIndexList indexList;
    indexList.append(index);

    listview->m_currentSelectedRow = 2;
    listview->keyPressEventFilter(indexList, false, true, true);
    EXPECT_TRUE(listview->selectedIndexes().count() == 1);
    EXPECT_TRUE(listview->selectedIndexes().first().row() == 2);

    index = listview->m_fontPreviewProxyModel->index(3, 0);
    indexList.clear();
    indexList.append(index);
    listview->keyPressEventFilter(indexList, false, true, true);
    EXPECT_TRUE(listview->selectedIndexes().count() == 2);
    EXPECT_TRUE(listview->selectedIndexes().at(1).row() == 4);

    listview->keyPressEventFilter(indexList, false, true, false);
    EXPECT_TRUE(listview->selectedIndexes().count() == 1);
    EXPECT_TRUE(listview->selectedIndexes().first().row() == 3);

}

TEST_F(TestDFontPreviewListView, checkkeyPressEventKey_End)
{
    listview->m_fontPreviewProxyModel->insertRows(0, 5);

    QTest::keyPress(listview->viewport(), Qt::Key_End, Qt::NoModifier);
    EXPECT_TRUE(listview->currentIndex().row() == 4);

    listview->m_currentSelectedRow = 0;
    QTest::keyPress(listview->viewport(), Qt::Key_End, Qt::ShiftModifier);
    EXPECT_TRUE(listview->selectedIndexes().count() == 5);

    listview->m_currentSelectedRow = -1;
    QTest::keyPress(listview->viewport(), Qt::Key_End, Qt::ShiftModifier);
    EXPECT_TRUE(listview->m_currentSelectedRow == 0);

}

TEST_F(TestDFontPreviewListView, checkkeyPressEventKey_Home)
{
    listview->m_fontPreviewProxyModel->insertRows(0, 5);

    QTest::keyPress(listview->viewport(), Qt::Key_Home, Qt::NoModifier);
    EXPECT_TRUE(listview->currentIndex().row() == 0);

    listview->m_currentSelectedRow = 4;
    QTest::keyPress(listview->viewport(), Qt::Key_Home, Qt::ShiftModifier);
    EXPECT_TRUE(listview->selectedIndexes().count() == 5);
}

TEST_F(TestDFontPreviewListView, checkkeyPressEventKey_UpDown)
{
    listview->m_fontPreviewProxyModel->insertRows(0, 5);
    listview->selectAll();

    QSignalSpy spy(listview->m_signalManager, SIGNAL(requestSetLeftSiderBarFocus()));


    QTest::keyPress(listview->viewport(), Qt::Key_Up, Qt::ShiftModifier);

    QTest::keyPress(listview->viewport(), Qt::Key_Down, Qt::ShiftModifier);

    QTest::keyPress(listview->viewport(), Qt::Key_Down, Qt::ControlModifier);

    QTest::keyPress(listview->viewport(), Qt::Key_Up, Qt::NoModifier);

    QTest::keyPress(listview->viewport(), Qt::Key_Down, Qt::NoModifier);

    QTest::keyPress(listview->viewport(), Qt::Key_Left, Qt::NoModifier);

    EXPECT_TRUE(spy.count() == 1);
}

TEST_F(TestDFontPreviewListView, checkrowsAboutToBeRemoved)
{
    listview->m_fontPreviewProxyModel->insertRows(0, 5);
    listview->selectAll();

    QModelIndex index = listview->m_fontPreviewProxyModel->index(1, 0);

    listview->rowsAboutToBeRemoved(index, 1, 1);

    EXPECT_TRUE(listview->selectionModel()->currentIndex().row() == 1);
}

TEST_F(TestDFontPreviewListView, checkIfHasSelection)
{
    listview->m_fontPreviewProxyModel->insertRows(0, 5);
    listview->checkIfHasSelection();
    EXPECT_TRUE(listview->m_currentSelectedRow == -1);
}


TEST_F(TestDFontPreviewListView, checkMousePressEventLeft)
{
    listview->m_fontPreviewProxyModel->insertRows(0, 5);
    QMouseEvent *e = new QMouseEvent(QEvent::MouseButtonPress, QPoint(), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    listview->mousePressEvent(e);
    delete  e;
}


TEST_F(TestDFontPreviewListView, checkMousePressEventRight)
{
    Stub s;
    s.set(ADDR(DFontPreviewListView, onMouseRightBtnPressed), stub_Return);

    listview->m_fontPreviewProxyModel->insertRows(0, 5);
    QMouseEvent *e = new QMouseEvent(QEvent::MouseButtonPress, QPoint(), Qt::RightButton, Qt::NoButton, Qt::NoModifier);
    listview->mousePressEvent(e);
    delete  e;
}

TEST_F(TestDFontPreviewListView, checkMousePressEventMid)
{
    listview->m_fontPreviewProxyModel->insertRows(0, 5);
    listview->selectAll();
    QMouseEvent *e = new QMouseEvent(QEvent::MouseButtonPress, QPoint(), Qt::MidButton, Qt::NoButton, Qt::NoModifier);
    listview->mousePressEvent(e);
    EXPECT_TRUE(listview->selectionModel()->selectedRows().count() == 1);
    SAFE_DELETE_ELE(e);

    listview->clearSelection();
    QMouseEvent *e2 = new QMouseEvent(QEvent::MouseButtonPress, QPoint(), Qt::MidButton, Qt::NoButton, Qt::ShiftModifier);
    listview->mousePressEvent(e2);
    EXPECT_FALSE(listview->m_IsTabFocus);
    SAFE_DELETE_ELE(e2);

    listview->selectAll();
    QMouseEvent *e3 = new QMouseEvent(QEvent::MouseButtonPress, QPoint(), Qt::MidButton, Qt::NoButton, Qt::ShiftModifier);
    listview->mousePressEvent(e3);
    SAFE_DELETE_ELE(e3);

    QMouseEvent *e4 = new QMouseEvent(QEvent::MouseButtonPress, QPoint(), Qt::MidButton, Qt::NoButton, Qt::ControlModifier);
    listview->mousePressEvent(e4);
    SAFE_DELETE_ELE(e4);

    Stub s;
    s.set(ADDR(QModelIndex, isValid), stub_False);
    listview->selectAll();
    QMouseEvent *e5 = new QMouseEvent(QEvent::MouseButtonPress, QPoint(), Qt::MidButton, Qt::NoButton, Qt::NoModifier);
    listview->mousePressEvent(e5);
    EXPECT_TRUE(listview->selectionModel()->selectedRows().count() == 0);

    SAFE_DELETE_ELE(e5);
}

TEST_F(TestDFontPreviewListView, checkLoadLeftFonts)
{
    Stub s;
    s.set(ADDR(DFontPreviewListView, isListDataLoadFinished), stub_True);
    s.set(ADDR(QThread, start), stub_start);

    QSignalSpy spy(listview, SIGNAL(loadUserAddFont()));

    listview->loadLeftFonts();
    QList<DFontPreviewItemData> str;

    emit listview->m_dataLoadThread->dataLoadFinish(str);

    qDebug() << spy.count() << endl;
    EXPECT_TRUE(spy.count() == 1);
}

TEST_F(TestDFontPreviewListView, checkSelectFonts)
{
    Stub s1;
    s1.set(ADDR(DFontPreviewProxyModel, setFilterGroup), stub_Return);

    Stub s2;
    s2.set(ADDR(DFontPreviewListDataThread, getFontData), stub_getFontData);

    QSignalSpy spy(listview, SIGNAL(requestInstFontsUiAdded()));
    listview->m_fontPreviewProxyModel->insertRows(0, 5);

    //传入参数为空
    listview->selectFonts(QStringList());
    EXPECT_TRUE(spy.count() == 0);

    QStringList list;
    list << "first";

    listview->selectFonts(list);
    EXPECT_TRUE(spy.count() == 1);
}
bool stub_isValid()
{
    return true;
}
TEST_F(TestDFontPreviewListView, checkmouseMoveEvent)
{
    Stub s;
//    typedef  QModelIndex(*fptr)(const QPoint &);
//    fptr DFontPreviewListView_indexAt = (fptr)(&DFontPreviewListView::indexAt);
//    s.set(DFontPreviewListView_indexAt, stub_indexAt);
//    s.set(ADDR(QModelIndex, isValid), stub_isValid);

    listview->m_fontPreviewProxyModel->insertRows(0, 5);
    QMouseEvent *e = new QMouseEvent(QEvent::MouseMove, QPoint(623, 23), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    listview->mouseMoveEvent(e);
//    EXPECT_TRUE(listview->m_hoverModelIndex.row() == 0);

    listview->m_isMousePressNow = true;
    QMouseEvent *e2 = new QMouseEvent(QEvent::MouseMove, QPoint(623, 23), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    listview->mouseMoveEvent(e2);
//    EXPECT_TRUE(listview->m_previousPressPos == 0);

    QMouseEvent *e3 = new QMouseEvent(QEvent::MouseMove, QPoint(423, 23), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    listview->mouseMoveEvent(e3);

    SAFE_DELETE_ELE(e);
    SAFE_DELETE_ELE(e2);
    SAFE_DELETE_ELE(e3);
}

TEST_F(TestDFontPreviewListView, checkonMouseLeftBtnPressed)
{
    Stub s1;
    s1.set(ADDR(DFontPreviewListView, updateShiftSelect), stub_Return);

    Stub s2;
    s2.set(ADDR(DFontPreviewListView, setIsTabFocus), stub_Return);

    listview->m_fontPreviewProxyModel->insertRows(0, 5);
    listview->selectAll();
    QModelIndex index = listview->selectionModel()->selectedIndexes().first();
    listview->clearSelection();

    listview->onMouseLeftBtnPressed(index, QPoint(623, 23), true, false);

    listview->onMouseLeftBtnPressed(index, QPoint(623, 23), false, true);

    listview->onMouseLeftBtnPressed(index, QPoint(596, 10), false, false);
}

TEST_F(TestDFontPreviewListView, checkOnMouseRightBtnPressedNotShift)
{
    Stub s1;
    s1.set(ADDR(DFontPreviewListView, onListViewShowContextMenu), stub_Return);

    Stub s2;
    s2.set(ADDR(FontData, getHoverState), stub_getHoverState);

    QSignalSpy spy(listview->m_signalManager, SIGNAL(menuHidden()));

    listview->m_fontPreviewProxyModel->insertRows(0, 5);
    listview->selectAll();
    QModelIndex index = listview->selectionModel()->selectedIndexes().first();
    listview->clearSelection();

    listview->onMouseRightBtnPressed(index, false);

    EXPECT_TRUE(spy.count() == 1);
}

TEST_F(TestDFontPreviewListView, checkOnMouseRightBtnPressedShift)
{
    Stub s1;
    s1.set(ADDR(DFontPreviewListView, onListViewShowContextMenu), stub_Return);

    Stub s2;
    s2.set(ADDR(FontData, getHoverState), stub_getHoverState);

    QSignalSpy spy(listview->m_signalManager, SIGNAL(onMenuHidden()));

    listview->m_fontPreviewProxyModel->insertRows(0, 5);
    listview->selectAll();
    QModelIndex index = listview->selectionModel()->selectedIndexes().first();
    QModelIndex index2 = listview->selectionModel()->selectedIndexes().last();
    listview->clearSelection();

    listview->m_currentSelectedRow = 2;
    listview->onMouseRightBtnPressed(index, true);

    listview->m_currentSelectedRow = 0;
    listview->onMouseRightBtnPressed(index2, true);
}

TEST_F(TestDFontPreviewListView, checkMouseReleaseEvent)
{
    Stub s;
    s.set(ADDR(DFontMgrMainWindow, currentFontGroup), stub_Return);

    listview->m_fontPreviewProxyModel->insertRows(0, 5);
    listview->selectAll();
    QModelIndex index = listview->selectionModel()->selectedIndexes().first();
    listview->onMouseLeftBtnReleased(index, QPoint(623, 23));

    listview->onMouseLeftBtnReleased(index, QPoint(32, 32));

    s.set(ADDR(DFontPreviewListView, onMouseLeftBtnReleased), stub_Return);

    QMouseEvent *e1 = new QMouseEvent(QEvent::MouseButtonRelease, QPoint(623, 23), Qt::MidButton, Qt::NoButton, Qt::NoModifier);
    listview->mouseReleaseEvent(e1);

    QMouseEvent *e2 = new QMouseEvent(QEvent::MouseButtonRelease, QPoint(623, 23), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    listview->mouseReleaseEvent(e2);

    SAFE_DELETE_ELE(e1);
    SAFE_DELETE_ELE(e2);
}

TEST_F(TestDFontPreviewListView, checkSelectedFontsSystemFont)
{
    DFontPreviewItemData data;
    data.fontInfo.psname = "first";

    listview->m_fontPreviewProxyModel->insertRows(0, 5);
    listview->selectAll();
    QModelIndexList indexlist = listview->selectionModel()->selectedIndexes();
    QModelIndex index = indexlist.first();

    int deleteCnt;
    int disableSysCnt;
    int systemCnt;
    qint8 curFontCnt;
    int disableCnt;
    QStringList list;
    QModelIndexList disableIndexList;
    QStringList allMinusSysFontList;

    Stub s;
    s.set(ADDR(DFontPreviewListDataThread, getFontData), stub_returnSysItemdata);

    listview->selectedFonts(data, &deleteCnt, &disableSysCnt, &systemCnt, &curFontCnt, &disableCnt,
                            &list, &indexlist, &disableIndexList, &allMinusSysFontList);

    EXPECT_TRUE(systemCnt == 5);
    EXPECT_TRUE(disableCnt == 5);
    EXPECT_TRUE(disableIndexList.count() == 5);
}

TEST_F(TestDFontPreviewListView, checkSelectedFontsNotSystem)
{
    DFontPreviewItemData data;
    data.fontInfo.psname = "first";

    listview->m_fontPreviewProxyModel->insertRows(0, 5);
    listview->selectAll();
    QModelIndexList indexlist = listview->selectionModel()->selectedIndexes();
    QModelIndex index = indexlist.first();

    int deleteCnt;
    int disableSysCnt;
    int systemCnt;
    qint8 curFontCnt;
    int disableCnt;
    QStringList list;
    QModelIndexList disableIndexList;
    QStringList allMinusSysFontList;

    Stub s;
    s.set(ADDR(DFontPreviewListDataThread, getFontData), stub_returnNotSysItemdata);

    listview->selectedFonts(data, &deleteCnt, &disableSysCnt, &systemCnt, &curFontCnt, &disableCnt,
                            &list, &indexlist, &disableIndexList, &allMinusSysFontList);

    EXPECT_TRUE(allMinusSysFontList.count() == 1);
}

TEST_F(TestDFontPreviewListView, checkSelectedFontsNotElse)
{
    DFontPreviewItemData data;
    data.fontInfo.psname = "first";

    listview->m_fontPreviewProxyModel->insertRows(0, 5);
    listview->selectAll();
    QModelIndexList indexlist = listview->selectionModel()->selectedIndexes();
    QModelIndex index = indexlist.first();

    int deleteCnt;
    int disableSysCnt;
    int systemCnt;
    qint8 curFontCnt;
    int disableCnt;
    QStringList list;
    QModelIndexList disableIndexList;
    QStringList allMinusSysFontList;
    listview->m_curFontData.strFontName = "curfont";
    Stub s;
    s.set(ADDR(DFontPreviewListDataThread, getFontData), stub_returnNotSysItemdata);

    listview->selectedFonts(data, &deleteCnt, &disableSysCnt, &systemCnt, &curFontCnt, &disableCnt,
                            &list, &indexlist, &disableIndexList, &allMinusSysFontList);

    qDebug() << deleteCnt << disableSysCnt << systemCnt << curFontCnt << disableCnt << list.count() << disableIndexList.count() << allMinusSysFontList.count() << endl;

    EXPECT_TRUE(deleteCnt == 5);
    EXPECT_TRUE(disableCnt == 5);
    EXPECT_TRUE(list.count() == 1);
}

//FontManager类,依附于previewlistview,再此进行测试
TEST_F(TestDFontPreviewListView, checkgetStartFontList)
{
    DFontPreviewListDataThread *thread = DFontPreviewListDataThread::instance();

    FontManager::instance()->getStartFontList();
    EXPECT_FALSE(thread->m_allFontPathList.isEmpty());
    EXPECT_FALSE(thread->m_monoSpaceFontPathList.isEmpty());
    FontManager::instance()->getChineseAndMonoFont();
}


TEST_F(TestDFontPreviewListView, onMultiItemsAdded)
{
    DFontPreviewItemData previewItmenData;
    previewItmenData.fontInfo.sp3FamilyName = "?";
    previewItmenData.fontInfo.isSystemFont = false;
    QList<DFontPreviewItemData> data;
    data.append(previewItmenData);
    DFontSpinnerWidget::SpinnerStyles styles;

    listview->onMultiItemsAdded(data, styles);
}


TEST_F(TestDFontPreviewListView, onRefreshListview)
{
    DFontPreviewItemData previewItmenData;
    previewItmenData.fontInfo.fullname = "04b_09";
    QList<DFontPreviewItemData> data;
    data.append(previewItmenData);


    listview->onRefreshListview(data);
}

TEST_F(TestDFontPreviewListView, onFontChanged)
{
    QFont font;
    listview->m_curAppFont = font;

    listview->onFontChanged(font);
}

TEST_F(TestDFontPreviewListView, selectItemAfterRemoved)
{
    listview->m_selectAfterDel = 1;
    Stub s;
    s.set(ADDR(DFontPreviewListView, count), stub_count);

    listview->selectItemAfterRemoved(false, true, false, false);

    listview->m_selectAfterDel = 2;
    listview->selectItemAfterRemoved(false, false, false, false);

    listview->m_selectAfterDel = 16;
    listview->selectItemAfterRemoved(false, false, false, false);


    listview->m_selectAfterDel = 13;
    listview->m_userFontInUseSelected = true;
    listview->selectItemAfterRemoved(true, false, false, false);
}


TEST_F(TestDFontPreviewListView, deleteFontFiles)
{
    QStringList strlist;
    strlist << "11";
    listview->deleteFontFiles(strlist, true);
}

TEST_F(TestDFontPreviewListView, onListViewShowContextMenu)
{
    Stub s;
    s.set((QAction * (QMenu::*)(const QPoint & pos, QAction * at))ADDR(QMenu, exec), stub_QMenuAction);
    listview->m_rightMenu = new QMenu;
    listview->onListViewShowContextMenu();
    SAFE_DELETE_ELE(listview->m_rightMenu);
}

