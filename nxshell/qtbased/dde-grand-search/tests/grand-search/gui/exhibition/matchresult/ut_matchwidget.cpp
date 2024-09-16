// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/matcheditem.h"
#include "gui/exhibition/matchresult/groupwidget.h"
#include "gui/exhibition/matchresult/matchwidget.h"
#include "gui/exhibition/matchresult/matchwidget_p.h"
#include "gui/exhibition/matchresult/listview/grandsearchlistview.h"
#include "utils/utils.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <DScrollArea>

#include <QSignalSpy>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QResizeEvent>

using namespace testing;
using namespace GrandSearch;
DWIDGET_USE_NAMESPACE

TEST(MatchWidgetTest, constructor)
{
    MatchWidget *w = new MatchWidget;

    ASSERT_TRUE(w);
    ASSERT_TRUE(w->d_p);
    ASSERT_TRUE(w->m_scrollArea);
    ASSERT_TRUE(w->m_scrollAreaContent);
    ASSERT_TRUE(w->m_vScrollLayout);
    ASSERT_FALSE(w->m_groupHashShowOrder.isEmpty());

    delete w;
}

TEST(MatchWidgetTest, appendMatchedData)
{
    MatchWidget w;

    MatchedItemMap itemMap;
    MatchedItems items;
    MatchedItem item;
    QString searchGroupName;

    // 测试搜索组名为空
    stub_ext::StubExt stu;

    bool ut_call_relayout = false;
    stu.set_lamda(ADDR(MatchWidget, reLayout), [&](){
        ut_call_relayout = true;
    });

    items << item;
    itemMap.insert(searchGroupName, items);
    w.appendMatchedData(itemMap);
    EXPECT_FALSE(ut_call_relayout);

    // 测试存在WEB搜索结果
    ut_call_relayout = false;
    searchGroupName = GRANDSEARCH_GROUP_WEB;
    itemMap.clear();
    itemMap.insert(searchGroupName, items);
    w.appendMatchedData(itemMap);
    EXPECT_TRUE(ut_call_relayout);

    // 测试存在其他搜索结果
    ut_call_relayout = false;
    searchGroupName = "test";
    itemMap.clear();
    itemMap.insert(searchGroupName, items);
    w.m_customSelected = false;
    w.appendMatchedData(itemMap);
    EXPECT_TRUE(ut_call_relayout);
}

TEST(MatchWidgetTest, clearMatchedData)
{
    MatchWidget w;

    QString searchGroupName(GRANDSEARCH_GROUP_FOLDER);
    MatchedItem item;
    MatchedItems items{item};
    MatchedItemMap itemMap{{searchGroupName, items}};
    w.appendMatchedData(itemMap);

    w.clearMatchedData();
    GroupWidget *groupW = w.m_groupWidgetMap.value(searchGroupName);
    ASSERT_TRUE(groupW);

    int count = groupW->itemCount();
    EXPECT_EQ(count, 0);

    EXPECT_FALSE(w.m_customSelected);
}

TEST(MatchWidgetTest, onSearchCompleted)
{
    MatchWidget w;

    QSignalSpy spy(&w, &MatchWidget::sigShowNoContent);
    w.onSearchCompleted();
    EXPECT_EQ(spy.count(), 1);

    QString searchGroupName(GRANDSEARCH_GROUP_FOLDER);
    MatchedItem item;
    MatchedItems items{item};
    MatchedItemMap itemMap{{searchGroupName, items}};
    w.appendMatchedData(itemMap);

    spy.clear();
    w.onSearchCompleted();
    EXPECT_EQ(spy.count(), 0);
}

TEST(MatchWidgetTest, onShowMore)
{
    MatchWidget w;

    stub_ext::StubExt stu;

    bool ut_call_relayout = false;
    stu.set_lamda(ADDR(MatchWidget, reLayout), [&](){
        ut_call_relayout = true;
    });

    w.onShowMore();
    EXPECT_TRUE(ut_call_relayout);
}

TEST(MatchWidgetTest, selectNextItem)
{
    MatchWidget w;

    stub_ext::StubExt stu;

    bool ut_hide = true;
    stu.set_lamda(ADDR(QWidget, isHidden), [&](){
        return ut_hide;
    });

    bool ut_call_adjust = false;
    stu.set_lamda(ADDR(MatchWidget, adjustScrollBar), [&](){
        ut_call_adjust = true;
    });

    w.selectNextItem();
    EXPECT_FALSE(ut_call_adjust);

    // 窗口显示的情况下，添加数据才会触发刷新，否则会忽略
    ut_hide = false;

    // 1. 测试当前项是最后一组的最后一个
    ut_call_adjust = false;

    QString searchGroupName(GRANDSEARCH_GROUP_FOLDER);
    MatchedItem item;
    MatchedItems items{item};
    MatchedItemMap itemMap{{searchGroupName, items}};
    w.appendMatchedData(itemMap);

    w.selectNextItem();
    EXPECT_TRUE(ut_call_adjust);

    // 2.1 测试当前项是当前组的最后一个，且后面还有其它的组存在有效项
    ut_call_adjust = false;

    searchGroupName = GRANDSEARCH_GROUP_FILE;
    itemMap.clear();
    itemMap.insert(searchGroupName, items);
    w.appendMatchedData(itemMap);

    bool ut_get_selectFirstItem = true;
    stu.set_lamda(&MatchWidget::selectFirstItem, [&](){
        return  ut_get_selectFirstItem;
    });

    w.selectNextItem();
    EXPECT_TRUE(ut_call_adjust);
    stu.reset(&MatchWidget::selectFirstItem);

    // 2.2 测试当前项是当前组的最后一个，且后面的组选择失败
    w.m_customSelected = false;
    w.appendMatchedData(itemMap);

    ut_get_selectFirstItem = false;
    stu.set_lamda(&MatchWidget::selectFirstItem, [&](){
        return  ut_get_selectFirstItem;
    });

    w.selectNextItem();
    EXPECT_TRUE(ut_call_adjust);
    stu.reset(&MatchWidget::selectFirstItem);

    // 3. 当前项所在的组，后面还有项
    w.m_customSelected = false;
    ut_call_adjust = false;
    searchGroupName = GRANDSEARCH_GROUP_FOLDER;
    itemMap.clear();
    itemMap.insert(searchGroupName, items);
    w.appendMatchedData(itemMap);

    w.selectNextItem();
    EXPECT_TRUE(ut_call_adjust);
}

TEST(MatchWidgetTest, selectPreviousItem)
{
    MatchWidget w;

    stub_ext::StubExt stu;

    bool ut_hide = true;
    stu.set_lamda(ADDR(QWidget, isHidden), [&](){
        return ut_hide;
    });

    bool ut_call_adjust = false;
    stu.set_lamda(ADDR(MatchWidget, adjustScrollBar), [&](){
        ut_call_adjust = true;
    });

    w.selectPreviousItem();
    EXPECT_FALSE(ut_call_adjust);

    // 窗口显示的情况下，添加数据才会触发刷新，否则会忽略
    ut_hide = false;

    // 1. 测试当前项是第一组的第一个
    ut_call_adjust = false;

    QString searchGroupName(GRANDSEARCH_GROUP_FILE);
    MatchedItem item;
    MatchedItems items{item};
    MatchedItemMap itemMap{{searchGroupName, items}};
    w.appendMatchedData(itemMap);

    w.selectPreviousItem();
    EXPECT_TRUE(ut_call_adjust);

    // 2.1 测试当前项是当前组的第一个，且前面还有其它的组存在有效项
    ut_call_adjust = false;

    searchGroupName = GRANDSEARCH_GROUP_FOLDER;
    itemMap.clear();
    itemMap.insert(searchGroupName, items);
    w.appendMatchedData(itemMap);
    w.selectFirstItem(1);

    bool ut_get_selectLastItem = true;
    stu.set_lamda(&MatchWidget::selectLastItem, [&](){
        return  ut_get_selectLastItem;
    });

    w.selectPreviousItem();
    EXPECT_TRUE(ut_call_adjust);
    stu.reset(&MatchWidget::selectLastItem);

    // 2.2 测试当前项是当前组的第一个，且前面的组选择失败
    w.m_customSelected = false;
    w.appendMatchedData(itemMap);
    w.selectFirstItem(1);

    ut_get_selectLastItem = false;
    stu.set_lamda(&MatchWidget::selectLastItem, [&](){
        return  ut_get_selectLastItem;
    });

    w.selectPreviousItem();
    EXPECT_TRUE(ut_call_adjust);
    stu.reset(&MatchWidget::selectLastItem);

    // 3. 当前项所在的组，前面还有项
    w.m_customSelected = false;
    ut_call_adjust = false;
    searchGroupName = GRANDSEARCH_GROUP_FILE;
    itemMap.clear();
    itemMap.insert(searchGroupName, items);
    w.appendMatchedData(itemMap);
    w.selectLastItem(0);

    w.selectPreviousItem();
    EXPECT_TRUE(ut_call_adjust);
}

TEST(MatchWidgetTest, handleItem)
{
    MatchWidget w;

    stub_ext::StubExt stu;

    bool ut_hide = false;
    stu.set_lamda(ADDR(QWidget, isHidden), [&](){
        return ut_hide;
    });

    bool ut_call_utilsOpen = false;
    stu.set_lamda(ADDR(Utils, openMatchedItem), [&](){
        ut_call_utilsOpen = true;
        return ut_call_utilsOpen;
    });

    // 窗口显示的情况下，添加数据才会触发刷新，否则会忽略
    ut_hide = false;
    ut_call_utilsOpen = false;

    QString searchGroupName(GRANDSEARCH_GROUP_FILE);
    MatchedItem item;
    MatchedItems items{item};
    MatchedItemMap itemMap{{searchGroupName, items}};
    w.appendMatchedData(itemMap);

    w.handleItem();
    EXPECT_TRUE(ut_call_utilsOpen);
}

TEST(MatchWidgetTest, onPreviewStateChanged)
{
    bool preview = true;
    MatchWidget widget;
    GroupWidgets list;
    GroupWidget tempWidget;
    list.append(&tempWidget);
    widget.m_vGroupWidgets = list;

    bool test = false;
    stub_ext::StubExt stu;
    stu.set_lamda(&GrandSearchListView::updatePreviewItemState, [&](){
        test = true;
    });

    widget.onPreviewStateChanged(preview);
    EXPECT_TRUE(test);
}

TEST(MatchWidgetTest, onSelectItemByMouse)
{
    MatchWidget w;

    stub_ext::StubExt stu;

    // 窗口显示的情况下，添加数据才会触发刷新，否则会忽略
    bool ut_hide = false;
    stu.set_lamda(ADDR(QWidget, isHidden), [&](){
        return ut_hide;
    });

    QString searchGroupName(GRANDSEARCH_GROUP_FILE);
    MatchedItem item;
    MatchedItems items{item};
    MatchedItemMap itemMap{{searchGroupName, items}};
    w.appendMatchedData(itemMap);

    searchGroupName = GRANDSEARCH_GROUP_FOLDER;
    itemMap.clear();
    itemMap.insert(searchGroupName, items);
    w.appendMatchedData(itemMap);

    stu.set_lamda((bool(MatchWidget::*)(int))ADDR(MatchWidget, hasSelectItem), [&](){
        return true;
    });

    QSignalSpy spy(&w, &MatchWidget::sigCurrentItemChanged);

    GrandSearchListView *listView = w.m_groupWidgetMap.value(GRANDSEARCH_GROUP_FILE)->getListView();
    ASSERT_TRUE(listView);

    listView->sigCurrentItemChanged(item);
    EXPECT_EQ(spy.count(), 1);
}

TEST(MatchWidgetTest, selectFirstItem)
{
    MatchWidget w;

    stub_ext::StubExt stu;

    bool ut_hide = true;
    stu.set_lamda(ADDR(QWidget, isHidden), [&](){
        return ut_hide;
    });

    bool ut_call_currentIndexChanged = false;
    stu.set_lamda(ADDR(MatchWidget, currentIndexChanged), [&](){
        ut_call_currentIndexChanged = true;
    });

    bool result = w.selectFirstItem(0);
    EXPECT_FALSE(result);
    EXPECT_FALSE(ut_call_currentIndexChanged);

    // 窗口显示的情况下，添加数据才会触发刷新，否则会忽略
    ut_hide = false;

    ut_call_currentIndexChanged = false;

    QString searchGroupName(GRANDSEARCH_GROUP_FOLDER);
    MatchedItem item;
    MatchedItems items{item};
    MatchedItemMap itemMap{{searchGroupName, items}};
    w.appendMatchedData(itemMap);

    result = w.selectFirstItem(0);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ut_call_currentIndexChanged);
}

TEST(MatchWidgetTest, selectLastItem)
{
    MatchWidget w;

    stub_ext::StubExt stu;

    bool ut_hide = true;
    stu.set_lamda(ADDR(QWidget, isHidden), [&](){
        return ut_hide;
    });

    bool ut_call_currentIndexChanged = false;
    stu.set_lamda(ADDR(MatchWidget, currentIndexChanged), [&](){
        ut_call_currentIndexChanged = true;
    });

    bool result = w.selectLastItem(0);
    EXPECT_FALSE(result);
    EXPECT_FALSE(ut_call_currentIndexChanged);

    // 窗口显示的情况下，添加数据才会触发刷新，否则会忽略
    ut_hide = false;

    ut_call_currentIndexChanged = false;

    QString searchGroupName(GRANDSEARCH_GROUP_FOLDER);
    MatchedItem item;
    MatchedItems items{item};
    MatchedItemMap itemMap{{searchGroupName, items}};
    w.appendMatchedData(itemMap);

    result = w.selectLastItem(0);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ut_call_currentIndexChanged);
}

TEST(MatchWidgetTest, hasSelectItem)
{
    MatchWidget w;

    stub_ext::StubExt stu;

    bool ut_hide = true;
    stu.set_lamda(ADDR(QWidget, isHidden), [&](){
        return ut_hide;
    });

    bool result = w.hasSelectItem();
    EXPECT_FALSE(result);

    // 窗口显示的情况下，添加数据才会触发刷新，否则会忽略
    ut_hide = false;

    QString searchGroupName(GRANDSEARCH_GROUP_FOLDER);
    MatchedItem item;
    MatchedItems items{item};
    MatchedItemMap itemMap{{searchGroupName, items}};
    w.appendMatchedData(itemMap);

    result = w.hasSelectItem();
    EXPECT_TRUE(result);
}

TEST(MatchWidgetTest, hasSelectItem_group)
{
    MatchWidget w;

    stub_ext::StubExt stu;

    bool ut_hide = true;
    stu.set_lamda(ADDR(QWidget, isHidden), [&](){
        return ut_hide;
    });

    w.m_vGroupWidgets.append(nullptr);
    bool result = w.hasSelectItem(0);
    EXPECT_FALSE(result);
    w.m_vGroupWidgets.clear();

    // 窗口显示的情况下，添加数据才会触发刷新，否则会忽略
    ut_hide = false;

    QString searchGroupName(GRANDSEARCH_GROUP_FOLDER);
    MatchedItem item;
    MatchedItems items{item};
    MatchedItemMap itemMap{{searchGroupName, items}};
    w.appendMatchedData(itemMap);

    result = w.hasSelectItem(0);
    EXPECT_TRUE(result);

    stu.set_lamda(&GroupWidget::getListView, [&](){
        return nullptr;
    });

    result = w.hasSelectItem(0);
    EXPECT_FALSE(result);
    stu.reset(&GroupWidget::getListView);

    stu.set_lamda(&QAbstractItemView::currentIndex, [&](){
        return QModelIndex();
    });

    result = w.hasSelectItem(0);
    EXPECT_FALSE(result);
}

TEST(MatchWidgetTest, clearSelectItem)
{
    MatchWidget w;

    stub_ext::StubExt stu;

    bool ut_hide = false;
    stu.set_lamda(ADDR(QWidget, isHidden), [&](){
        return ut_hide;
    });

    // 窗口显示的情况下，添加数据才会触发刷新，否则会忽略
    ut_hide = false;

    QString searchGroupName(GRANDSEARCH_GROUP_FOLDER);
    MatchedItem item;
    MatchedItems items{item};
    MatchedItemMap itemMap{{searchGroupName, items}};
    w.appendMatchedData(itemMap);

    QSignalSpy spy(&w, &MatchWidget::sigCurrentItemChanged);

    EXPECT_EQ(spy.count(), 0);
    w.clearSelectItem();
    EXPECT_EQ(spy.count(), 1);
}

TEST(MatchWidgetTest, adjustScrollBar)
{
    MatchWidget w;

    stub_ext::StubExt stu;

    bool ut_hide = false;
    stu.set_lamda(ADDR(QWidget, isHidden), [&](){
        return ut_hide;
    });

    // 窗口显示的情况下，添加数据才会触发刷新，否则会忽略
    ut_hide = false;

    QString searchGroupName(GRANDSEARCH_GROUP_FOLDER);
    MatchedItem item;
    MatchedItems items{item};
    MatchedItemMap itemMap{{searchGroupName, items}};
    w.appendMatchedData(itemMap);

    w.m_vGroupWidgets.prepend(nullptr);
    stu.set_lamda(&GroupWidget::getListView, [&](){
        return nullptr;
    });

    int oriValue = w.m_scrollArea->verticalScrollBar()->value();
    w.adjustScrollBar();
    stu.reset(&GroupWidget::getListView);
    int updateValue = w.m_scrollArea->verticalScrollBar()->value();
    EXPECT_EQ(oriValue, updateValue);

    // 弹出插入的测试数据空指针
    w.m_vGroupWidgets.pop_front();

    itemMap.insert(GRANDSEARCH_GROUP_FILE, items);
    w.appendMatchedData(itemMap);
    w.clearSelectItem();
    w.selectLastItem(1);

#if 0 // crash
    stu.set_lamda(&QScrollArea::height, [&](){
        return 0;
    });
    stu.set_lamda(&QAbstractSlider::value, [&](){
        return 0;
    });

    w.adjustScrollBar();
    stu.reset(&QScrollArea::height);
    stu.reset(&QAbstractSlider::value);
    updateValue = w.m_scrollArea->verticalScrollBar()->value();
    EXPECT_NE(oriValue, updateValue);
#endif
}

TEST(MatchWidgetTest, currentIndexChanged)
{
    MatchWidget w;

    stub_ext::StubExt stu;

    bool ut_hide = false;
    stu.set_lamda(ADDR(QWidget, isHidden), [&](){
        return ut_hide;
    });

    // 窗口显示的情况下，添加数据才会触发刷新，否则会忽略
    ut_hide = false;

    QString searchGroupName(GRANDSEARCH_GROUP_FOLDER);
    MatchedItem item;
    MatchedItems items{item};
    MatchedItemMap itemMap{{searchGroupName, items}};
    w.appendMatchedData(itemMap);

    auto group = w.m_vGroupWidgets.at(0);
    auto list = group->getListView();
    auto index = list->currentIndex();

    QSignalSpy spy(&w, &MatchWidget::sigCurrentItemChanged);
    EXPECT_EQ(spy.count(), 0);
    w.currentIndexChanged(GRANDSEARCH_GROUP_FOLDER, index);
    EXPECT_EQ(spy.count(), 1);
}

TEST(MatchWidgetTest, reLayout)
{
    MatchWidget w;

    stub_ext::StubExt stu;

    bool ut_hide = false;
    stu.set_lamda(ADDR(QWidget, isHidden), [&](){
        return ut_hide;
    });

    // 窗口显示的情况下，添加数据才会触发刷新，否则会忽略
    ut_hide = false;

    QString searchGroupName(GRANDSEARCH_GROUP_FOLDER);
    MatchedItem item;
    MatchedItems items{item};
    MatchedItemMap itemMap{{searchGroupName, items}};
    w.appendMatchedData(itemMap);

    w.reLayout();
    EXPECT_NE(w.m_vScrollLayout->count(), 0);
}

TEST(MatchWidgetTest, createGroupWidget)
{
    MatchWidget w;

    GroupWidget *result = w.createGroupWidget("");
    EXPECT_FALSE(result);

    result = w.createGroupWidget(GRANDSEARCH_GROUP_FOLDER);
    EXPECT_TRUE(result);

    result = w.createGroupWidget(GRANDSEARCH_GROUP_FILE);
    EXPECT_TRUE(result);

    result = w.createGroupWidget(GRANDSEARCH_GROUP_FILE);
    EXPECT_TRUE(result);
}

TEST(MatchWidgetTest, sortVislibleGroupList)
{
    MatchWidget w;

    stub_ext::StubExt stu;

    bool ut_hide = false;
    stu.set_lamda(ADDR(QWidget, isHidden), [&](){
        return ut_hide;
    });

    // 窗口显示的情况下，添加数据才会触发刷新，否则会忽略
    ut_hide = false;

    QString searchGroupName(GRANDSEARCH_GROUP_FOLDER);
    MatchedItem item;
    MatchedItems items{item};
    MatchedItemMap itemMap{{searchGroupName, items}};
    w.appendMatchedData(itemMap);

    itemMap.clear();
    itemMap.insert("Test", items);
    w.appendMatchedData(itemMap);

    w.sortVislibleGroupList();
    EXPECT_FALSE(w.m_vGroupWidgets.isEmpty());
}

TEST(MatchWidgetTest, paintEvent)
{
    MatchWidget w;

    bool isCall = false;

    stub_ext::StubExt stu;
    typedef void (*fptr)(DWidget*,QPaintEvent*);
    fptr ut_paintEvent = (fptr)(&DWidget::paintEvent);
    stu.set_lamda(ut_paintEvent, [&](){
        isCall = true;
    });

    QPaintEvent event(QRect(0, 0, 100, 100));
    w.paintEvent(&event);

    EXPECT_TRUE(isCall);
}

TEST(MatchWidgetTest, resizeEvent)
{
    MatchWidget w;

    stub_ext::StubExt stu;

    bool ut_hide = false;
    stu.set_lamda(ADDR(QWidget, isHidden), [&](){
        return ut_hide;
    });

    // 窗口显示的情况下，添加数据才会触发刷新，否则会忽略
    ut_hide = false;

    QString searchGroupName(GRANDSEARCH_GROUP_FOLDER);
    MatchedItem item;
    MatchedItems items{item};
    MatchedItemMap itemMap{{searchGroupName, items}};
    w.appendMatchedData(itemMap);

    bool isCall = false;
    typedef void (*fptr)(DWidget*,QResizeEvent*);
    fptr ut_resizeEvent = (fptr)(&DWidget::resizeEvent);
    stu.set_lamda(ut_resizeEvent, [&](){
        isCall = true;
    });

    QResizeEvent event(QSize(100, 100), QSize(200, 200));
    w.resizeEvent(&event);
    EXPECT_TRUE(isCall);
}
