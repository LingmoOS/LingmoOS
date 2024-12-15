// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfontpreviewitemdelegate.h"
#include "dfontpreviewlistview.h"

#include "globaldef.h"
#include "utils.h"
#include "commonheaderfile.h"

#include <DApplication>
#include <DApplicationHelper>
#include <DStyleHelper>
#include <DPalette>
#include <DLog>
#include <DCheckBox>
#include <DLabel>
#include <DFontSizeManager>

#include <QAbstractItemView>
#include <QPainter>
#include <QHoverEvent>

#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"

namespace {
class TestDFontPreviewItemDelegate : public testing::Test
{

protected:
    void SetUp()
    {
        listview = new DFontPreviewListView();
        DfpDelegate = new DFontPreviewItemDelegate(listview);
    }
    void TearDown()
    {
        delete DfpDelegate;
        delete w;
        delete p;
    }
    // Some expensive resource shared by all tests.
    QWidget *w = new QWidget;
    DFontPreviewListView *listview;
    DFontPreviewItemDelegate *DfpDelegate;
    QPainter *p = new QPainter;
    QStyleOptionViewItem option;
    FontData f;

};

QList<DFontPreviewItemData> stub_getFontModeList()
{

    return DFontPreviewListDataThread::instance()->m_fontModelList;
}

bool stub_getIsTabFocus()
{
    return true;
}

QString stub_string()
{
    return "first";
}

}

TEST_F(TestDFontPreviewItemDelegate, checkPaint)
{
    QStandardItemModel *m_fontPreviewItemModel = new QStandardItemModel;
    m_fontPreviewItemModel->setColumnCount(1);
    DFontPreviewProxyModel *fpm = new DFontPreviewProxyModel;
    fpm->setSourceModel(m_fontPreviewItemModel);

    listview->setModel(fpm);
    m_fontPreviewItemModel->insertRows(0, 5);
    listview->m_fontPreviewProxyModel->insertRows(0, 5);

    listview->selectAll();

    QModelIndex index = listview->selectionModel()->selectedRows().first();

    FontData fdata("aaa", true, true, true, true, FontType::OTF, true);
    DFontPreviewItemData itemdata;
    itemdata.fontData = fdata;
    itemdata.fontData.strFontName = "aaa";
    itemdata.fontInfo.filePath = "/home/zhaogongqiang/Desktop/1048字体/ArkanaScriptRough.otf";
    DFontPreviewListDataThread::instance()->m_fontModelList.append(itemdata);

    m_fontPreviewItemModel->setData(index, QVariant::fromValue(fdata), Qt::DisplayRole);

    m_fontPreviewItemModel->setData(index, QVariant(qint8(30)), DFontPreviewItemDelegate::FontPreviewRole);

    Stub s1;
    s1.set(ADDR(DFontPreviewListDataThread, getFontModelList), stub_getFontModeList);

    Stub s2;
    s2.set(ADDR(QVariant, toString), stub_string);

    option.state.setFlag(QStyle::State_Selected);
    DfpDelegate->paint(p, option, index);

    Stub s3;
    s3.set(ADDR(DFontPreviewListView, getIsTabFocus), stub_getIsTabFocus);

    option.state.setFlag(QStyle::State_Selected);
    DfpDelegate->paint(p, option, index);

    option.state.setFlag(QStyle::State_None);
    DfpDelegate->paint(p, option, index);
}


TEST_F(TestDFontPreviewItemDelegate, checkEventFilter)
{
    QPointF p1;
    QPointF p2;
    QHoverEvent *e = new QHoverEvent(QEvent::HoverLeave, p1, p2);
    DfpDelegate->eventFilter(listview, e);
    SAFE_DELETE_ELE(e)
}



TEST_F(TestDFontPreviewItemDelegate, paintForegroundCollectIcon)
{
    QPainter painter;
    QStyleOptionViewItem stypelopeitem;
    FontData fontData;
    fontData.setHoverState(IconStatus::IconHover);
    DfpDelegate->paintForegroundCollectIcon(&painter, stypelopeitem, fontData);

    fontData.setHoverState(IconStatus::IconPress);
    DfpDelegate->paintForegroundCollectIcon(&painter, stypelopeitem, fontData);

    fontData.setHoverState(IconStatus::IconNormal);
    fontData.setCollected(true);
    DfpDelegate->paintForegroundCollectIcon(&painter, stypelopeitem, fontData);
}


TEST_F(TestDFontPreviewItemDelegate, adjustPreviewRect)
{
    DfpDelegate->adjustPreviewRect(QRect(0, 0, 0, 0));
}


TEST_F(TestDFontPreviewItemDelegate, paintBackground)
{
    QPainter painter;
    QStyleOptionViewItem stypelopeitem;
    stypelopeitem.state.setFlag(QStyle::State_Enabled);
    QModelIndex modelIndex;
    DfpDelegate->paintBackground(&painter, stypelopeitem, modelIndex);
}

