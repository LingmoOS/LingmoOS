// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfontpreviewlistdatathread.h"
#include "dfontpreviewlistview.h"
#include "dfmxmlwrapper.h"
#include "dcomworker.h"

#include <QWidget>
#include <QDir>
#include <QSignalSpy>
#include <QList>
#include <QMutexLocker>
#include <QFontDatabase>
#include <QApplication>
#include <QFileSystemWatcher>
#include <QFile>

#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"

namespace {
class TestDFontPreviewListDataThread : public testing::Test
{

protected:
    void SetUp()
    {
        listview = new DFontPreviewListView(w);
        dfdatathead = DFontPreviewListDataThread::instance(listview);
    }
    void TearDown()
    {

    }
    QWidget *w = new QWidget;
    DFontPreviewListView *listview;
    DFontPreviewListDataThread *dfdatathead;
public:
    bool stub_exists();


};

void stub_updateChangedDir()
{
}

QList<DFontPreviewItemData> stub_getAllFontInfo()
{
    DFontPreviewItemData data;
    QString filepath = "/home/zhaogongqiang/Desktop/1048字体/ArkanaScriptRough.otf";
    data.fontInfo.filePath = filepath;
    data.fontInfo.type = "Truetype";
    QList<DFontPreviewItemData> s;
    s << data;
    return s;

}

void stub_return()
{
    return ;
}

bool TestDFontPreviewListDataThread::stub_exists()
{
    return false;
}

QStringList stub_getFontDisableFontlist()
{
    QStringList list;
    list << "first";
    return list;
}

void stub_getFontListInSequence()
{
}

void stub_copyFiles(CopyFontThread::OPType, QStringList &)
{
}
void stub_onMultiItemsAdded(QList<DFontPreviewItemData> &, DFontSpinnerWidget::SpinnerStyles)
{
}
void stub_addFontInfo(const QList<DFontPreviewItemData> &)
{
}
bool stub_addPath(const QString &)
{
    return true;
}
}
TEST_F(TestDFontPreviewListDataThread, checkInitFileSystemWatcher)
{
    Stub s;
    s.set(ADDR(QFileSystemWatcher, addPath), stub_addPath);

    QSignalSpy spy1(dfdatathead->m_fsWatcher, SIGNAL(fileChanged(const QString &, QPrivateSignal)));
    QSignalSpy spy2(dfdatathead->m_fsWatcher, SIGNAL(directoryChanged(const QString &, QPrivateSignal)));

    dfdatathead->initFileSystemWatcher();

    Q_EMIT dfdatathead->m_fsWatcher->fileChanged(QString("1/2/3"), QFileSystemWatcher::QPrivateSignal());
    Q_EMIT dfdatathead->m_fsWatcher->directoryChanged(QString("1/2/3"), QFileSystemWatcher::QPrivateSignal());
//    EXPECT_EQ(spy1.count(), 1); //TODO:信号无法发送成功
//    EXPECT_EQ(spy2.count(), 1);
}


TEST_F(TestDFontPreviewListDataThread, checkUpdateChangedDir)
{
    Stub s1;
    s1.set(ADDR(DFontPreviewListView, updateChangedDir), stub_updateChangedDir);

    QString filepath = "/home/zhaogongqiang/Desktop/1048字体";
    dfdatathead->updateChangedDir();
}

TEST_F(TestDFontPreviewListDataThread, checkRefreshFontListData)
{
    Stub s;
    s.set(ADDR(DFMDBManager, getAllFontInfo), stub_getAllFontInfo);
    s.set(ADDR(DFMDBManager, commitUpdateFontInfo), stub_return);
    s.set((void(DFMDBManager::*)(const QList<DFontPreviewItemData> &))ADDR(DFMDBManager, addFontInfo), stub_addFontInfo);
    s.set(ADDR(FontManager, getFontListInSequence), stub_getFontListInSequence);

    QSignalSpy spy(dfdatathead->m_view, SIGNAL(itemsSelected(const QStringList & files, bool isFirstInstall = false)));
    QSignalSpy spy2(dfdatathead->m_view, SIGNAL(multiItemsAdded(QList<DFontPreviewItemData> &, DFontSpinnerWidget::SpinnerStyles)));

    dfdatathead->refreshFontListData(true, QStringList());
    qDebug() << spy2.count() << endl;
    EXPECT_TRUE(dfdatathead->m_allFontPathList.isEmpty());
    EXPECT_TRUE(spy2.count() == 1);
    EXPECT_FALSE(dfdatathead->m_fontModelList.isEmpty());

    dfdatathead->m_fontModelList.clear();
    dfdatathead->refreshFontListData(false, QStringList());
    EXPECT_TRUE(dfdatathead->m_allFontPathList.isEmpty());
    EXPECT_TRUE(spy2.count() == 2);
    EXPECT_TRUE(dfdatathead->m_fontModelList.isEmpty());

    SignalManager::m_isDataLoadFinish = false;
    DFontPreviewItemData dpitemd;
    dfdatathead->m_delFontInfoList.append(dpitemd);
    dfdatathead->m_startModelList.append(dpitemd);

    dfdatathead->refreshFontListData(true, QStringList("12345"));
    EXPECT_TRUE(dfdatathead->m_allFontPathList.contains(QLatin1String("12345")));


    dfdatathead->m_allFontPathList.clear();
    dfdatathead->m_fontModelList.clear();
    SignalManager::m_isDataLoadFinish = true;
    dfdatathead->m_delFontInfoList.append(dpitemd);
    dfdatathead->refreshFontListData(true, QStringList("12345"));
    EXPECT_FALSE(dfdatathead->m_fontModelList.isEmpty());

    dfdatathead->m_allFontPathList.clear();
    dfdatathead->m_fontModelList.clear();
    dfdatathead->refreshFontListData(false, QStringList("12345"));
    EXPECT_TRUE(dfdatathead->m_allFontPathList.isEmpty());
}

TEST_F(TestDFontPreviewListDataThread, checkRemovePathWatcher)
{
    QString FONTS_DIR = QDir::homePath() + "/.local/share/fonts/";
    dfdatathead->m_fsWatcher->addPath(FONTS_DIR);
    dfdatathead->removePathWatcher(FONTS_DIR);
    EXPECT_FALSE(dfdatathead->m_fsWatcher->directories().contains(FONTS_DIR));
}

TEST_F(TestDFontPreviewListDataThread, checkOnRemoveFileWatchers)
{
    QString FONTS_DIR = QDir::homePath() + "/.local/share/fonts/";
    QString FONTS_UP_DIR = QDir::homePath() + "/.local/share/";
    dfdatathead->m_fsWatcher->addPath(FONTS_DIR);
    dfdatathead->m_fsWatcher->addPath(FONTS_UP_DIR);
    dfdatathead->onRemoveFileWatchers(QStringList());
    EXPECT_FALSE(dfdatathead->m_fsWatcher->directories().contains(FONTS_DIR));
    EXPECT_FALSE(dfdatathead->m_fsWatcher->directories().contains(FONTS_UP_DIR));
}

TEST_F(TestDFontPreviewListDataThread, checkOnExportFont)
{
    Stub s;
    s.set(ADDR(DCopyFilesManager, copyFiles), stub_copyFiles);

    QString desktopPath = QString("%1/%2/").arg(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation))
                          .arg(QApplication::translate("DFontMgrMainWindow", "Fonts"));

    QDir d(desktopPath);
    d.removeRecursively();

    QString filepath = "|home|zhaogongqiang|Desktop|1048字体|ArkanaScriptRough.otf";
    QStringList l;
    l << filepath;
    QSignalSpy spy(dfdatathead, SIGNAL(exportFontFinished(int)));

    dfdatathead->onExportFont(l);
    EXPECT_TRUE(spy.count() == 1);

    dfdatathead->onExportFont(l);
    EXPECT_TRUE(spy.count() == 2);
}

TEST_F(TestDFontPreviewListDataThread, checkGetFontData)
{
    FontData data;
    data.strFontName = "aaa";

    dfdatathead->m_fontModelList.clear();

    DFontPreviewItemData data1;
    data1.appFontId = 2;

    FontData data2;
    data2.strFontName = "aaa";
    data1.fontData = data2;
    dfdatathead->m_fontModelList.append(data1);


    DFontPreviewItemData da;

    da = DFontPreviewListDataThread::getFontData(data);
    EXPECT_TRUE(da.appFontId == 2);

    data.strFontName = "bbb";
    da = DFontPreviewListDataThread::getFontData(data);
    EXPECT_TRUE(da.appFontId == -1);

}


TEST_F(TestDFontPreviewListDataThread, checkSyncFontEnableDisableStatusData)
{
    Stub s;
    s.set(ADDR(DFMDBManager, commitUpdateFontInfo), stub_return);

    Stub s1;
    s1.set((void (DFMDBManager::*)(const DFontPreviewItemData &, const QString &))ADDR(DFMDBManager, updateFontInfo), stub_return);

    QStringList disableFontPathList;
    disableFontPathList << "first";

    DFontPreviewItemData data;
    data.fontData.setEnabled(false);
    data.fontInfo.filePath = "first";
    dfdatathead->m_fontModelList.clear();
    dfdatathead->m_fontModelList.append(data);
    dfdatathead->syncFontEnableDisableStatusData(disableFontPathList);
    EXPECT_FALSE(dfdatathead->m_fontModelList.first().fontData.isEnabled());

    data.fontData.setEnabled(true);
    data.fontInfo.filePath = "first";
    dfdatathead->m_fontModelList.clear();
    dfdatathead->m_fontModelList.append(data);
    dfdatathead->syncFontEnableDisableStatusData(disableFontPathList);
    EXPECT_FALSE(dfdatathead->m_fontModelList.first().fontData.isEnabled());
}

TEST_F(TestDFontPreviewListDataThread, checkDoWork)
{
    Stub s;
    s.set(ADDR(DFMXmlWrapper, getFontConfigDisableFontPathList), stub_getFontDisableFontlist);
    s.set(ADDR(FontManager, getStartFontList), stub_return);
    s.set(ADDR(DFMDBManager, commitUpdateFontInfo), stub_return);
    s.set(ADDR(DFontPreviewListView, onMultiItemsAdded), stub_onMultiItemsAdded);
    s.set(ADDR(DFontPreviewListDataThread, initFileSystemWatcher), stub_return);
    s.set(ADDR(DFontPreviewListDataThread, withoutDbRefreshDb), stub_return);
    s.set(ADDR(DFontPreviewListDataThread, refreshStartupFontListData), stub_return);

    QSignalSpy spy(dfdatathead->m_view, SIGNAL(multiItemsAdded(QList<DFontPreviewItemData> &, DFontSpinnerWidget::SpinnerStyles)));

    dfdatathead->m_fontModelList.clear();
    dfdatathead->m_allFontPathList.clear();
    dfdatathead->m_allFontPathList << "second";

    dfdatathead->m_startModelList.clear();
    dfdatathead->doWork();
    EXPECT_TRUE(spy.count() == 1);

    DFontPreviewItemData data;
    dfdatathead->m_fontModelList << data;
    dfdatathead->doWork();
    EXPECT_TRUE(spy.count() == 2);

    Stub st;
    st.set((bool (QList<DFontPreviewItemData>::*)() const)ADDR(QList<DFontPreviewItemData>, isEmpty), ADDR(TestDFontPreviewListDataThread, stub_exists));
    dfdatathead->m_fontModelList << data;
    dfdatathead->doWork();
    EXPECT_TRUE(spy.count() == 2);
}

TEST_F(TestDFontPreviewListDataThread, checkGetFontModelList)
{
    dfdatathead->m_fontModelList.clear();
    DFontPreviewItemData data;
    data.fontData.setEnabled(true);
    dfdatathead->m_fontModelList << data;
    dfdatathead->m_view->onFinishedDataLoad();

    QList<DFontPreviewItemData> list = dfdatathead->getFontModelList();
    EXPECT_TRUE(list.count() == 1);
    EXPECT_TRUE(list.first().fontData.isEnabled());

    dfdatathead->m_view->m_bLoadDataFinish = false;
    list = dfdatathead->getFontModelList();
    EXPECT_TRUE(list.count() == 0);

}

TEST_F(TestDFontPreviewListDataThread, refreshStartupFontListData)
{
    Stub s;
    s.set(ADDR(DFontPreviewListView, onMultiItemsAdded), stub_onMultiItemsAdded);
    s.set(ADDR(DFontPreviewListView, enableFonts), stub_return);

    dfdatathead->m_delFontInfoList.clear();
    dfdatathead->m_startModelList.clear();

    DFontPreviewItemData data;
    data.fontData.setChinese(true);

    dfdatathead->m_delFontInfoList << data;
    dfdatathead->m_startModelList << data;

    dfdatathead->refreshStartupFontListData();
    EXPECT_FALSE(dfdatathead->m_view->m_enableFontList.isEmpty());
    EXPECT_TRUE(dfdatathead->m_delFontInfoList.isEmpty());
}

TEST_F(TestDFontPreviewListDataThread, checkOnFileDeleted)
{
    Stub s;
    s.set(ADDR(DFontPreviewListView, deleteCurFonts), stub_return);

    QStringList list;
    dfdatathead->onFileDeleted(list);
}

TEST_F(TestDFontPreviewListDataThread, checkOnFileAdded)
{
    QStringList list;
    dfdatathead->onFileAdded(list);

    list << "first";

    Stub s;
    s.set(ADDR(DFontPreviewListDataThread, refreshFontListData), stub_return);
    dfdatathead->onFileAdded(list);
}

TEST_F(TestDFontPreviewListDataThread, checkUpdateItemStatus)
{
    DFontPreviewItemData data;
    data.fontData.setChinese(true);
    dfdatathead->m_fontModelList.append(data);

    DFontPreviewItemData data2;
    data2.fontData.setChinese(false);
    dfdatathead->updateItemStatus(0, data2);

    EXPECT_TRUE(dfdatathead->m_fontModelList.first() == data2);
}

TEST_F(TestDFontPreviewListDataThread, checkForceDeleteFiles)
{
    Stub s;
    s.set(ADDR(DFontPreviewListView, deleteFontFiles), stub_return);

    QSignalSpy spy(dfdatathead, SIGNAL(requestBatchReInstallContinue()));

    QStringList list;
    list << "first";

    dfdatathead->forceDeleteFiles(list);

    EXPECT_TRUE(spy.count() == 1);
}



TEST_F(TestDFontPreviewListDataThread, onRefreshUserAddFont)
{
    Stub s;
    s.set(ADDR(DFontPreviewListDataThread, insertFontItemData), stub_return);
    s.set(ADDR(DFMDBManager, commitAddFontInfo), stub_return);

    QList<DFontInfo> fontInfoList;

    DFontInfo fontInfo;
    fontInfoList.append(fontInfo);
    DFontPreviewItemData dfpid;
    dfdatathead->m_diffFontModelList.append(dfpid);
    dfdatathead->m_isAllLoaded = false;
    dfdatathead->onRefreshUserAddFont(fontInfoList);
    EXPECT_TRUE(dfdatathead->m_diffFontModelList.isEmpty());
    EXPECT_TRUE(dfdatathead->m_isAllLoaded);
}
