// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfontpreviewproxymodel.h"
#include "dfontpreviewlistdatathread.h"

#include "views/dfontmgrmainwindow.h"
#include "dfontpreviewitemdef.h"
#include "views/dsplitlistwidget.h"
#include "dfontpreviewlistdatathread.h"
#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"
#include "commonheaderfile.h"

#include <DLog>

namespace {
class TestDFontPreviewProxyModel : public testing::Test
{

protected:
    void SetUp()
    {
        fpm = new DFontPreviewProxyModel();
    }
    void TearDown()
    {
        delete w;
        delete fpm;
    }
    // Some expensive resource shared by all tests.
    QWidget *w = new QWidget;
    DFontPreviewProxyModel *fpm;
};
}

TEST_F(TestDFontPreviewProxyModel, checkSetFilterGroup)
{
    fpm->setFilterGroup(2);
    EXPECT_TRUE(fpm->m_filterGroup == 2);
    EXPECT_FALSE(fpm->m_useSystemFilter);
}

TEST_F(TestDFontPreviewProxyModel, checkSetFilterFontNamePattern)
{
    fpm->setFilterFontNamePattern("first");
    EXPECT_TRUE(fpm->m_fontNamePattern == "first");
    EXPECT_FALSE(fpm->m_useSystemFilter);
}

TEST_F(TestDFontPreviewProxyModel, checkIsFontNameContainsPattern)
{
    fpm->m_fontNamePattern = "";
    EXPECT_TRUE(fpm->isFontNameContainsPattern("aaa"));

    fpm->m_fontNamePattern = "qqq";
    EXPECT_TRUE(fpm->isFontNameContainsPattern("aqqqa"));
}

TEST_F(TestDFontPreviewProxyModel, checkFilterAcceptsRow)
{
    QStandardItemModel *m_fontPreviewItemModel = new QStandardItemModel;
    m_fontPreviewItemModel->setColumnCount(1);
    fpm->setSourceModel(m_fontPreviewItemModel);
    DFontPreviewListView *listview = new DFontPreviewListView;
    listview->setModel(fpm);
    m_fontPreviewItemModel->insertRow(0, QModelIndex());

    EXPECT_TRUE(fpm->filterAcceptsRow(0, QModelIndex()));

    fpm->setFilterGroup(1);
    EXPECT_FALSE(fpm->filterAcceptsRow(0,  QModelIndex()));

    SAFE_DELETE_ELE(m_fontPreviewItemModel)
    SAFE_DELETE_ELE(listview)
}

TEST_F(TestDFontPreviewProxyModel, checkFilterAcceptsRowAllFonts)
{
    QStandardItemModel *m_fontPreviewItemModel = new QStandardItemModel;
    m_fontPreviewItemModel->setColumnCount(1);
    fpm->setSourceModel(m_fontPreviewItemModel);
    DFontPreviewListView *listview = new DFontPreviewListView;
    listview->setModel(fpm);
    m_fontPreviewItemModel->insertRow(0, QModelIndex());

    QModelIndex index = m_fontPreviewItemModel->index(0, 0);
    FontData fdata("aaa", false, false, false, false, FontType::OTF, true);
    m_fontPreviewItemModel->setData(index, QVariant::fromValue(fdata), Qt::DisplayRole);
    EXPECT_TRUE(fpm->filterAcceptsRow(0,  QModelIndex()));
    fpm->setFilterGroup(0);
    fpm->setFilterFontNamePattern("aaa");
    EXPECT_TRUE(fpm->filterAcceptsRow(0,  QModelIndex()));

    SAFE_DELETE_ELE(m_fontPreviewItemModel)
    SAFE_DELETE_ELE(listview)
}

TEST_F(TestDFontPreviewProxyModel, checkFilterAcceptsRowAllSysFonts)
{
    QStandardItemModel *m_fontPreviewItemModel = new QStandardItemModel;
    m_fontPreviewItemModel->setColumnCount(1);
    fpm->setSourceModel(m_fontPreviewItemModel);
    DFontPreviewListView *listview = new DFontPreviewListView;
    listview->setModel(fpm);
    m_fontPreviewItemModel->insertRow(0, QModelIndex());

    QModelIndex index = m_fontPreviewItemModel->index(0, 0);
    FontData fdata("aaa", false, false, false, false, FontType::OTF, true);
    DFontPreviewItemData itemdata;
    itemdata.fontData = fdata;
    itemdata.fontInfo.filePath = "/usr/share/fonts/";
    itemdata.fontData.strFontName = "aaa";
    DFontPreviewListDataThread::instance()->m_fontModelList.append(itemdata);

    m_fontPreviewItemModel->setData(index, QVariant::fromValue(fdata), Qt::DisplayRole);
    fpm->setFilterGroup(1);
    EXPECT_TRUE(fpm->filterAcceptsRow(0,  QModelIndex()));
    fpm->setFilterFontNamePattern("aaa");
    EXPECT_TRUE(fpm->filterAcceptsRow(0,  QModelIndex()));
    DFontPreviewListDataThread::instance()->m_fontModelList.removeLast();

    SAFE_DELETE_ELE(m_fontPreviewItemModel)
    SAFE_DELETE_ELE(listview)
}

TEST_F(TestDFontPreviewProxyModel, checkFilterAcceptsRowAllUserFonts)
{
    QStandardItemModel *m_fontPreviewItemModel = new QStandardItemModel;
    m_fontPreviewItemModel->setColumnCount(1);
    fpm->setSourceModel(m_fontPreviewItemModel);
    DFontPreviewListView *listview = new DFontPreviewListView;
    listview->setModel(fpm);
    m_fontPreviewItemModel->insertRow(0, QModelIndex());

    QModelIndex index = m_fontPreviewItemModel->index(0, 0);
    FontData fdata("aaa", false, false, false, false, FontType::OTF, true);
    DFontPreviewItemData itemdata;
    itemdata.fontData = fdata;
    itemdata.fontInfo.filePath = "/.local/share/fonts/";
    itemdata.fontData.strFontName = "aaa";
    DFontPreviewListDataThread::instance()->m_fontModelList.append(itemdata);

    m_fontPreviewItemModel->setData(index, QVariant::fromValue(fdata), Qt::DisplayRole);
    fpm->setFilterGroup(1);
    qDebug() << fpm->filterAcceptsRow(0,  QModelIndex());
    EXPECT_TRUE(fpm->filterAcceptsRow(0,  QModelIndex()));
    fpm->setFilterFontNamePattern("aaa");
    qDebug() << fpm->filterAcceptsRow(0,  QModelIndex());
    EXPECT_TRUE(fpm->filterAcceptsRow(0,  QModelIndex()));
    DFontPreviewListDataThread::instance()->m_fontModelList.removeLast();

    SAFE_DELETE_ELE(m_fontPreviewItemModel);
    SAFE_DELETE_ELE(listview);
}


TEST_F(TestDFontPreviewProxyModel, checkFilterAcceptsRowAllCollectFonts)
{
    QStandardItemModel *m_fontPreviewItemModel = new QStandardItemModel;
    m_fontPreviewItemModel->setColumnCount(1);
    fpm->setSourceModel(m_fontPreviewItemModel);
    DFontPreviewListView *listview = new DFontPreviewListView;
    listview->setModel(fpm);
    m_fontPreviewItemModel->insertRow(0, QModelIndex());

    QModelIndex index = m_fontPreviewItemModel->index(0, 0);
    FontData fdata("aaa", false, true, false, false, FontType::OTF, true);
    DFontPreviewItemData itemdata;
    itemdata.fontData = fdata;
    itemdata.fontData.strFontName = "aaa";
    DFontPreviewListDataThread::instance()->m_fontModelList.append(itemdata);

    m_fontPreviewItemModel->setData(index, QVariant::fromValue(fdata), Qt::DisplayRole);
    fpm->setFilterGroup(3);
    EXPECT_TRUE(fpm->filterAcceptsRow(0,  QModelIndex()));
    fpm->setFilterFontNamePattern("aaa");
    EXPECT_TRUE(fpm->filterAcceptsRow(0,  QModelIndex()));
    fpm->setFilterGroup(4);
    EXPECT_FALSE(fpm->filterAcceptsRow(0,  QModelIndex()));
    DFontPreviewListDataThread::instance()->m_fontModelList.removeLast();

    SAFE_DELETE_ELE(m_fontPreviewItemModel);
    SAFE_DELETE_ELE(listview);
}

TEST_F(TestDFontPreviewProxyModel, checkFilterAcceptsRowAllActiviteFonts)
{
    QStandardItemModel *m_fontPreviewItemModel = new QStandardItemModel;
    m_fontPreviewItemModel->setColumnCount(1);
    fpm->setSourceModel(m_fontPreviewItemModel);
    DFontPreviewListView *listview = new DFontPreviewListView;
    listview->setModel(fpm);
    m_fontPreviewItemModel->insertRow(0, QModelIndex());

    QModelIndex index = m_fontPreviewItemModel->index(0, 0);
    FontData fdata("aaa", true, true, true, true, FontType::OTF, true);
    DFontPreviewItemData itemdata;
    itemdata.fontData = fdata;
    itemdata.fontData.strFontName = "aaa";
    DFontPreviewListDataThread::instance()->m_fontModelList.append(itemdata);

    m_fontPreviewItemModel->setData(index, QVariant::fromValue(fdata), Qt::DisplayRole);
    fpm->setFilterGroup(4);
    EXPECT_TRUE(fpm->filterAcceptsRow(0,  QModelIndex()));
    fpm->setFilterFontNamePattern("aaa");
    EXPECT_TRUE(fpm->filterAcceptsRow(0,  QModelIndex()));
    DFontPreviewListDataThread::instance()->m_fontModelList.removeLast();

    SAFE_DELETE_ELE(m_fontPreviewItemModel);
    SAFE_DELETE_ELE(listview);
}

TEST_F(TestDFontPreviewProxyModel, checkFilterAcceptsRowAllChineseFonts)
{
    QStandardItemModel *m_fontPreviewItemModel = new QStandardItemModel;
    m_fontPreviewItemModel->setColumnCount(1);
    fpm->setSourceModel(m_fontPreviewItemModel);
    DFontPreviewListView *listview = new DFontPreviewListView;
    listview->setModel(fpm);
    m_fontPreviewItemModel->insertRow(0, QModelIndex());

    QModelIndex index = m_fontPreviewItemModel->index(0, 0);
    FontData fdata("aaa", true, true, true, true, FontType::OTF, true);
    DFontPreviewItemData itemdata;
    itemdata.fontData = fdata;
    itemdata.fontData.strFontName = "aaa";
    DFontPreviewListDataThread::instance()->m_fontModelList.append(itemdata);
    m_fontPreviewItemModel->setData(index, QVariant::fromValue(fdata), Qt::DisplayRole);
    fpm->setFilterGroup(5);
    EXPECT_TRUE(fpm->filterAcceptsRow(0,  QModelIndex()));
    fpm->setFilterFontNamePattern("aaa");
    EXPECT_TRUE(fpm->filterAcceptsRow(0,  QModelIndex()));
    DFontPreviewListDataThread::instance()->m_fontModelList.removeLast();

    SAFE_DELETE_ELE(m_fontPreviewItemModel);
    SAFE_DELETE_ELE(listview);
}

TEST_F(TestDFontPreviewProxyModel, checkFilterAcceptsRowAllMonoFonts)
{
    QStandardItemModel *m_fontPreviewItemModel = new QStandardItemModel;
    m_fontPreviewItemModel->setColumnCount(1);
    fpm->setSourceModel(m_fontPreviewItemModel);
    DFontPreviewListView *listview = new DFontPreviewListView;
    listview->setModel(fpm);
    m_fontPreviewItemModel->insertRow(0, QModelIndex());

    QModelIndex index = m_fontPreviewItemModel->index(0, 0);
    FontData fdata("aaa", true, true, true, true, FontType::OTF, true);
    DFontPreviewItemData itemdata;
    itemdata.fontData = fdata;
    itemdata.fontData.strFontName = "aaa";
    DFontPreviewListDataThread::instance()->m_fontModelList.append(itemdata);

    m_fontPreviewItemModel->setData(index, QVariant::fromValue(fdata), Qt::DisplayRole);
    fpm->setFilterGroup(6);
    EXPECT_TRUE(fpm->filterAcceptsRow(0,  QModelIndex()));
    fpm->setFilterFontNamePattern("aaa");
    EXPECT_TRUE(fpm->filterAcceptsRow(0,  QModelIndex()));
    DFontPreviewListDataThread::instance()->m_fontModelList.removeLast();

    SAFE_DELETE_ELE(m_fontPreviewItemModel);
    SAFE_DELETE_ELE(listview);
}
