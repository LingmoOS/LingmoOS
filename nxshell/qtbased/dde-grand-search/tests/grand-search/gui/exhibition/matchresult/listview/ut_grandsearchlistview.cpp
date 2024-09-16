// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gui/exhibition/matchresult/listview/grandsearchlistview.h"
#include "gui/exhibition/matchresult/listview/grandsearchlistmodel.h"

#include "global/matcheditem.h"
#include "global/builtinsearch.h"
#include "utils/utils.h"

#include "stubext.h"

#include <DStandardPaths>

#include <gtest/gtest.h>

#include <QPaintEvent>
#include <QEvent>
#include <QWidget>
#include <QSignalSpy>

using namespace testing;
using namespace GrandSearch;
DCORE_USE_NAMESPACE

TEST(GrandSearchListViewTest, constructor)
{
    GrandSearchListView *w = new GrandSearchListView;

    ASSERT_TRUE(w);
    ASSERT_TRUE(w->m_model);
    ASSERT_TRUE(w->m_delegate);

    delete w;
}

TEST(GrandSearchListViewTest, setMatchedItems)
{
    GrandSearchListView w;

    QString searchGroupName(GRANDSEARCH_GROUP_FOLDER);
    MatchedItem item;
    MatchedItems items{item, item};

    w.setMatchedItems(items);
    EXPECT_EQ(items, w.m_matchedItems);
}

TEST(GrandSearchListViewTest, addRow)
{
    GrandSearchListView w;

    stub_ext::StubExt stu;

    bool ut_call_setData = false;
    stu.set_lamda(ADDR(GrandSearchListView, setData), [&](){
        ut_call_setData = true;
    });

    MatchedItem item;
    w.addRow(item);
    EXPECT_TRUE(ut_call_setData);
}

TEST(GrandSearchListViewTest, addRow_level)
{
    GrandSearchListView w;

    stub_ext::StubExt stu;

    bool ut_call_insertRow = false;
    stu.set_lamda(ADDR(GrandSearchListView, insertRow), [&](){
        ut_call_insertRow = true;
        return 0;
    });

    MatchedItem item;
    w.addRow(item, 1);
    EXPECT_TRUE(ut_call_insertRow);
}

TEST(GrandSearchListViewTest, addRows)
{
    GrandSearchListView w;

    stub_ext::StubExt stu;

    bool ut_call_addRow = false;
    stu.set_lamda((void(GrandSearchListView::*)(const MatchedItem &))ADDR(GrandSearchListView, addRow), [&](){
        ut_call_addRow = true;
    });

    MatchedItem item;
    MatchedItems items{item, item};

    w.addRows(items);
    EXPECT_TRUE(ut_call_addRow);
}

TEST(GrandSearchListViewTest, addRows_level)
{
    GrandSearchListView w;

    stub_ext::StubExt stu;

    bool ut_call_insertRows = false;
    stu.set_lamda(ADDR(GrandSearchListView, insertRows), [&](){
        ut_call_insertRows = true;
    });

    MatchedItem item;
    MatchedItems items{item, item};

    w.addRows(items, 1);
    EXPECT_TRUE(ut_call_insertRows);
}

TEST(GrandSearchListViewTest, insertRow)
{
    GrandSearchListView w;

    stub_ext::StubExt stu;

    bool ut_call_addRow = false;
    stu.set_lamda((void(GrandSearchListView::*)(const MatchedItem &))ADDR(GrandSearchListView, addRow), [&](){
        ut_call_addRow = true;
    });

    bool ut_call_setData = false;
    stu.set_lamda(ADDR(GrandSearchListView, setData), [&](){
        ut_call_setData = true;
    });

    MatchedItem item;

    w.insertRow(100, item);
    EXPECT_TRUE(ut_call_addRow);
    EXPECT_FALSE(ut_call_setData);

    QStandardItem *newItem = new QStandardItem;
    w.m_model->insertRow(0, newItem);

    ut_call_addRow = false;
    ut_call_setData = false;

    w.insertRow(0, item);
    EXPECT_FALSE(ut_call_addRow);
    EXPECT_TRUE(ut_call_setData);
}

TEST(GrandSearchListViewTest, insertRows)
{
    GrandSearchListView w;

    stub_ext::StubExt stu;

    bool ut_call_addRows = false;
    stu.set_lamda((void(GrandSearchListView::*)(const MatchedItems &))ADDR(GrandSearchListView, addRows), [&](){
        ut_call_addRows = true;
    });

    bool ut_call_setData = false;
    stu.set_lamda(ADDR(GrandSearchListView, setData), [&](){
        ut_call_setData = true;
    });

    MatchedItem item;
    MatchedItems items{item, item};

    w.insertRows(100, items);
    EXPECT_TRUE(ut_call_addRows);
    EXPECT_FALSE(ut_call_setData);

    QStandardItem *newItem = new QStandardItem;
    w.m_model->insertRow(0, newItem);

    ut_call_addRows = false;
    ut_call_setData = false;

    w.insertRows(0, items);
    EXPECT_FALSE(ut_call_addRows);
    EXPECT_TRUE(ut_call_setData);
}

TEST(GrandSearchListViewTest, removeRows)
{
    GrandSearchListView w;

    QString searchGroupName(GRANDSEARCH_GROUP_FOLDER);
    MatchedItem item;
    MatchedItems items{item, item};

    w.setMatchedItems(items);

    int origin = w.m_model->rowCount();
    EXPECT_NE(origin, 0);

    w.removeRows(0, 2);
    origin = w.m_model->rowCount();
    EXPECT_EQ(origin, 0);
}

TEST(GrandSearchListViewTest, levelItemCount)
{
    GrandSearchListView w;

    QString searchGroupName(GRANDSEARCH_GROUP_FOLDER);
    MatchedItem item;
    MatchedItems items;

    // 添加分级元素
    QVariantHash showLevelHash({{GRANDSEARCH_PROPERTY_ITEM_LEVEL, GRANDSEARCH_PROPERTY_ITEM_LEVEL_SECOND}});
    item.extra = QVariant::fromValue(showLevelHash);
    items << item << item;

    // 添加普通数据
    item.extra.clear();
    items << item << item;

    w.setMatchedItems(items);

    int count = w.levelItemCount(GRANDSEARCH_PROPERTY_ITEM_LEVEL_SECOND);
    EXPECT_EQ(count, 2);
}

TEST(GrandSearchListViewTest, rowCount)
{
    GrandSearchListView w;

    QString searchGroupName(GRANDSEARCH_GROUP_FOLDER);
    MatchedItem item;
    MatchedItems items{item, item};

    w.setMatchedItems(items);

    int actual = w.rowCount();
    int expect = w.m_model->rowCount();
    EXPECT_EQ(actual, expect);
}

TEST(GrandSearchListViewTest, getThemeType)
{
    GrandSearchListView w;

    int actual = w.getThemeType();
    int expect = w.m_themeType;
    EXPECT_EQ(actual, expect);
}

TEST(GrandSearchListViewTest, clear)
{
    GrandSearchListView w;

    QString searchGroupName(GRANDSEARCH_GROUP_FOLDER);
    MatchedItem item;
    MatchedItems items{item, item};

    w.setMatchedItems(items);

    w.clear();
    EXPECT_EQ(w.m_model->rowCount(), 0);
}

TEST(GrandSearchListViewTest, updatePreviewItemState)
{
    GrandSearchListView list;
    bool preview = true;
    list.updatePreviewItemState(preview);
    EXPECT_TRUE(list.m_isPreviewItem);
}

TEST(GrandSearchListViewTest, isPreviewItem)
{
    GrandSearchListView list;
    list.m_isPreviewItem = true;
    bool result = list.isPreviewItem();
    EXPECT_TRUE(result);
}

TEST(GrandSearchListViewTest, mousePressEvent)
{
    GrandSearchListView w;

    QSignalSpy spyItemChanged(&w, &GrandSearchListView::sigCurrentItemChanged);
    QSignalSpy spyItemClicked(&w, &GrandSearchListView::sigItemClicked);

    QMouseEvent event1(QEvent::MouseButtonPress, QPointF(0, 0), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    w.mousePressEvent(&event1);
    EXPECT_EQ(spyItemChanged.count(), 0);
    EXPECT_EQ(spyItemClicked.count(), 0);

    QString searchGroupName(GRANDSEARCH_GROUP_FOLDER);
    MatchedItem item;
    MatchedItems items{item, item};

    w.setMatchedItems(items);
    QModelIndex index = w.model()->index(0, 0);
    w.setCurrentIndex(index);

    stub_ext::StubExt stu;
    bool ut_call_open = false;
    stu.set_lamda(&Utils::openMatchedItem, [&](){
        ut_call_open = true;
        return true;
    });

    index = w.model()->index(1, 0);
    stu.set_lamda(VADDR(GrandSearchListView, indexAt), [&](){
        return index;
    });

    w.mousePressEvent(&event1);
    EXPECT_EQ(spyItemChanged.count(), 1);
    EXPECT_EQ(spyItemClicked.count(), 0);

    w.mousePressEvent(&event1);
    EXPECT_EQ(spyItemChanged.count(), 1);
    EXPECT_EQ(spyItemClicked.count(), 1);

    QMouseEvent event2(QEvent::MouseButtonPress, QPointF(0, 0), Qt::LeftButton, Qt::NoButton, Qt::ControlModifier);
    stu.set_lamda(&Utils::openMatchedItemWithCtrl, [](){
        return true;
    });

    w.mousePressEvent(&event2);
    EXPECT_EQ(spyItemChanged.count(), 1);
    EXPECT_EQ(spyItemClicked.count(), 2);
}

TEST(GrandSearchListViewTest, onSetThemeType)
{
    GrandSearchListView w;

    int actual = 0;
    w.onSetThemeType(actual);

    int expect = w.m_themeType;

    EXPECT_EQ(actual, expect);
}

TEST(GrandSearchListViewTest, cacheDir)
{
    GrandSearchListView w;

    QString expect = DStandardPaths::standardLocations(QStandardPaths::CacheLocation).value(0);
    QString actual = w.cacheDir();
    EXPECT_EQ(expect, actual);
}

TEST(GrandSearchListViewTest, event)
{
    GrandSearchListView w;

    QEvent event(QEvent::FocusIn);
    w.event(&event);
}

TEST(GrandSearchListViewTest, setData)
{
    GrandSearchListView w;

    QString searchGroupName(GRANDSEARCH_GROUP_FOLDER);
    MatchedItem item;
    w.addRow(item);

    QModelIndex index;
    w.setData(index, item);

    item.name = "QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ"
                "QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ";
    item.icon = "/test/abc.png";

    index = w.model()->index(0, 0);
    w.setData(index, item);

    QString tips = w.m_model->data(index, Qt::ToolTipRole).toString();
    EXPECT_EQ(tips, item.name);
    w.m_model->setData(index, QString(), Qt::ToolTipRole);

    item.name = "WW";
    item.icon = "abc.png";
    w.setData(index, item);
    tips = w.m_model->data(index, Qt::ToolTipRole).toString();
    EXPECT_TRUE(tips.isEmpty());
}

TEST(GrandSearchListViewTest, levelItemLastRow)
{
    GrandSearchListView w;

    QString searchGroupName(GRANDSEARCH_GROUP_FOLDER);
    MatchedItem item;
    MatchedItems items;

    // 添加分级元素
    QVariantHash showLevelHash({{GRANDSEARCH_PROPERTY_ITEM_LEVEL, GRANDSEARCH_PROPERTY_ITEM_LEVEL_SECOND}});
    item.extra = QVariant::fromValue(showLevelHash);
    items << item << item;

    // 添加普通数据
    item.extra.clear();
    items << item << item;

    w.setMatchedItems(items);

    int allCount = w.levelItemLastRow(0);
    int expect = w.m_model->rowCount();
    EXPECT_EQ(allCount, expect);


    allCount = w.levelItemLastRow(GRANDSEARCH_PROPERTY_ITEM_LEVEL_SECOND);
    EXPECT_EQ(allCount, 2);
}
