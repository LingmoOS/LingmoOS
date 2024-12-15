// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gui/exhibition/matchresult/levelitemgroupwidget.h"
#include "gui/exhibition/matchresult/listview/grandsearchlistview.h"
#include "global/matcheditem.h"
#include "utils/utils.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QPaintEvent>
#include <QEvent>
#include <QWidget>

using namespace testing;
using namespace GrandSearch;

TEST(LevelItemGroupWidgetTest, constructor)
{
    LevelItemGroupWidget *w = new LevelItemGroupWidget;

    ASSERT_TRUE(w);

    delete w;
}

TEST(LevelItemGroupWidgetTest, appendMatchedItems)
{
    LevelItemGroupWidget w;

    stub_ext::StubExt stu;

    bool ut_call_setMatchedItems = false;
    stu.set_lamda(ADDR(GrandSearchListView, setMatchedItems), [&](){
        ut_call_setMatchedItems = true;
    });

    bool ut_call_addRows = false;
    stu.set_lamda((void(GrandSearchListView::*)(const MatchedItems&))ADDR(GrandSearchListView, addRows), [&](){
        ut_call_addRows = true;
    });

    // 1.测试空数据
    QString searchGroupName(GRANDSEARCH_GROUP_FOLDER);
    MatchedItem item;
    MatchedItems items;
    w.appendMatchedItems(items, searchGroupName);
    EXPECT_FALSE(ut_call_setMatchedItems);
    EXPECT_FALSE(ut_call_addRows);

    // 2.测试折叠添加
    ut_call_setMatchedItems = false;
    ut_call_addRows = false;

    w.m_bListExpanded = false;

    // 添加普通元素
    items << item;

    // 添加分级元素
    QVariantHash showLevelHash({{GRANDSEARCH_PROPERTY_ITEM_LEVEL, GRANDSEARCH_PROPERTY_ITEM_LEVEL_FIRST}});
    item.extra = QVariant::fromValue(showLevelHash);

    // 先添加部分元素，让m_firstFiveItems存在内容，便于覆盖for循环内的语句
    items << item << item;
    w.appendMatchedItems(items, searchGroupName);

    QVariantHash otherLevelHash({{GRANDSEARCH_PROPERTY_ITEM_LEVEL, GRANDSEARCH_PROPERTY_ITEM_LEVEL_SECOND}});
    item.extra = QVariant::fromValue(otherLevelHash);
    items.clear();
    items << item << item;

    w.appendMatchedItems(items, searchGroupName);
    EXPECT_TRUE(ut_call_setMatchedItems);
    EXPECT_FALSE(ut_call_addRows);

    // 3.测试全量添加
    ut_call_setMatchedItems = false;
    ut_call_addRows = false;

    w.m_bListExpanded = true;

    w.appendMatchedItems(items, searchGroupName);

    EXPECT_FALSE(ut_call_setMatchedItems);
    EXPECT_TRUE(ut_call_addRows);
}

TEST(LevelItemGroupWidgetTest, clear)
{
    LevelItemGroupWidget w;

    w.clear();
    EXPECT_TRUE(w.m_firstFiveItems.isEmpty());
    EXPECT_TRUE(w.m_restShowItems.isEmpty());
    EXPECT_TRUE(w.m_cacheItems.isEmpty());
}

TEST(LevelItemGroupWidgetTest, onMoreBtnClicked)
{
    LevelItemGroupWidget w;

    QString searchGroupName(GRANDSEARCH_GROUP_FOLDER);
    MatchedItem item;
    MatchedItems items;

    // 添加分级元素
    QVariantHash showLevelHash({{GRANDSEARCH_PROPERTY_ITEM_LEVEL, GRANDSEARCH_PROPERTY_ITEM_LEVEL_FIRST}});
    item.extra = QVariant::fromValue(showLevelHash);

    // 需要超过五个数据
    items << item << item << item << item << item << item;
    w.appendMatchedItems(items, searchGroupName);

    w.onMoreBtnClicked();
    EXPECT_TRUE(w.m_levelCacheItems.isEmpty());
}

TEST(LevelItemGroupWidgetTest, takeItemFromLevelCache)
{
    LevelItemGroupWidget w;

    QString searchGroupName(GRANDSEARCH_GROUP_FOLDER);
    MatchedItem item;
    MatchedItems items;

    w.m_firstFiveItems.clear();

    // 添加空数据
    w.m_levelCacheItems.insert(1, items);

    // 添加分级元素
    QVariantHash showLevelHash({{GRANDSEARCH_PROPERTY_ITEM_LEVEL, GRANDSEARCH_PROPERTY_ITEM_LEVEL_SECOND}});
    item.extra = QVariant::fromValue(showLevelHash);

    items << item << item;
    w.m_levelCacheItems.insert(2, items);

    w.takeItemFromLevelCache();
    EXPECT_FALSE(w.m_firstFiveItems.isEmpty());
}

TEST(LevelItemGroupWidgetTest, takeItemFromGeneralCache)
{
    LevelItemGroupWidget w;

    QString searchGroupName(GRANDSEARCH_GROUP_FOLDER);
    MatchedItem item;
    MatchedItems items{item, item, item};

    w.m_firstFiveItems.clear();
    w.m_cacheItems.append(items);

    w.takeItemFromGeneralCache();
    EXPECT_FALSE(w.m_firstFiveItems.isEmpty());
}

TEST(LevelItemGroupWidgetTest, splitLevelData)
{
    LevelItemGroupWidget w;

    QString searchGroupName(GRANDSEARCH_GROUP_FOLDER);
    MatchedItem item;
    MatchedItems items{item, item};

    // 添加分级元素
    QVariantHash showLevelHash({{GRANDSEARCH_PROPERTY_ITEM_LEVEL, GRANDSEARCH_PROPERTY_ITEM_LEVEL_FIRST}});
    item.extra = QVariant::fromValue(showLevelHash);

    items << item;

    QMap<int, MatchedItems> newLevelItems;
    MatchedItems newGeneralItems;

    w.splitLevelData(items, newLevelItems, newGeneralItems);
    EXPECT_EQ(newLevelItems.count(), 1);
    EXPECT_EQ(newLevelItems.first().count(), 1);
    EXPECT_EQ(newGeneralItems.count(), 2);
}
