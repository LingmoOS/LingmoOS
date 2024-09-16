// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gui/exhibition/matchresult/groupwidget.h"
#include "gui/exhibition/matchresult/listview/grandsearchlistview.h"
#include "global/matcheditem.h"
#include "utils/utils.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <DLabel>
#include <DHorizontalLine>
#include <DPushButton>

#include <QPaintEvent>
#include <QEvent>
#include <QWidget>
#include <QVBoxLayout>

using namespace testing;
using namespace GrandSearch;
DWIDGET_USE_NAMESPACE

TEST(GroupWidgettTest, constructor)
{
    GroupWidget *w = new GroupWidget;

    ASSERT_TRUE(w);
    ASSERT_TRUE(w->d_p);
    ASSERT_TRUE(w->m_listView);
    ASSERT_TRUE(w->m_viewMoreButton);

    delete w;
}

TEST(GroupWidgettTest, appendMatchedItems)
{
    GroupWidget w;

    stub_ext::StubExt stu;

    bool ut_call_updateShowItems = false;
    stu.set_lamda(ADDR(GroupWidget, updateShowItems), [&](){
        ut_call_updateShowItems = true;
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
    EXPECT_FALSE(ut_call_updateShowItems);
    EXPECT_FALSE(ut_call_addRows);

    // 2.测试折叠添加,且此次为动态排序数据
    ut_call_updateShowItems = false;
    ut_call_addRows = false;    

    w.m_bListExpanded = false;
    w.m_listView->clear();

    QVariantHash itemWeight({{GRANDSEARCH_PROPERTY_ITEM_WEIGHT, 100}});
    item.extra = QVariant::fromValue(itemWeight);

    items << item << item << item;
    w.appendMatchedItems(items, searchGroupName);

    EXPECT_TRUE(ut_call_updateShowItems);
    EXPECT_FALSE(ut_call_addRows);

    // 3.测试折叠添加,且不是动态排序数据
    ut_call_updateShowItems = false;
    ut_call_addRows = false;
    items.clear();

    w.m_bListExpanded = false;
    w.m_listView->clear();

    MatchedItem itemOther;
    items << itemOther;

    w.appendMatchedItems(items, searchGroupName);

    EXPECT_TRUE(ut_call_updateShowItems);
    EXPECT_FALSE(ut_call_addRows);
    EXPECT_FALSE(w.m_cacheItems.isEmpty());

    // 4.测试全量添加
    ut_call_updateShowItems = false;
    ut_call_addRows = false;

    w.m_bListExpanded = true;

    w.appendMatchedItems(items, searchGroupName);

    EXPECT_FALSE(ut_call_updateShowItems);
    EXPECT_TRUE(ut_call_addRows);
}

TEST(GroupWidgettTest, clear)
{
    GroupWidget w;

    w.clear();
    EXPECT_TRUE(w.m_firstFiveItems.isEmpty());
    EXPECT_TRUE(w.m_restShowItems.isEmpty());
    EXPECT_TRUE(w.m_cacheItems.isEmpty());
}

TEST(GroupWidgettTest, setSearchGroupName)
{
    GroupWidget w;

    QString searchGroupName;
    w.setSearchGroupName(searchGroupName);
    EXPECT_EQ(w.m_searchGroupName, searchGroupName);

    searchGroupName = QString("Test");
    w.setSearchGroupName(searchGroupName);
    EXPECT_EQ(w.m_searchGroupName, searchGroupName);
}

TEST(GroupWidgettTest, searchGroupName)
{
    GroupWidget w;

    QString searchGroupName("Test1");
    w.m_searchGroupName = searchGroupName;
    QString value = w.searchGroupName();
    EXPECT_EQ(value, searchGroupName);

    searchGroupName = "Test2";
    w.m_searchGroupName = searchGroupName;
    value = w.searchGroupName();
    EXPECT_EQ(value, searchGroupName);
}

TEST(GroupWidgettTest, setGroupName)
{
    GroupWidget w;

    QString groupName;
    w.setGroupName(groupName);
    QString textValue = w.m_groupLabel->text();
    EXPECT_EQ(textValue, groupName);

    groupName = QString("Test");
    w.setGroupName(groupName);
    textValue = w.m_groupLabel->text();
    EXPECT_EQ(textValue, groupName);
}

TEST(GroupWidgettTest, groupName)
{
    GroupWidget w;

    QString groupName("Test1");
    w.m_groupLabel->setText(groupName);
    QString value = w.groupName();
    EXPECT_EQ(value, groupName);

    groupName = "Test2";
    w.m_groupLabel->setText(groupName);
    value = w.groupName();
    EXPECT_EQ(value, groupName);
}

TEST(GroupWidgettTest, showHorLine)
{
    GroupWidget w;

    w.showHorLine(false);
    bool result = w.m_line->isVisible();
    EXPECT_FALSE(result);
}

TEST(GroupWidgettTest, isHorLineVisilbe)
{
    GroupWidget w;

    bool result = w.isHorLineVisilbe();
    bool actual = w.m_line->isVisible();
    EXPECT_EQ(result, actual);
}

TEST(GroupWidgettTest, getListView)
{
    GroupWidget w;

    EXPECT_EQ(w.getListView(), w.m_listView);
}

TEST(GroupWidgettTest, getViewMoreButton)
{
    GroupWidget w;

    EXPECT_EQ(w.getViewMoreButton(), w.m_viewMoreButton);
}

TEST(GroupWidgettTest, itemCount)
{
    GroupWidget w;

    int actual = w.itemCount();
    int expect = w.m_listView->rowCount();
    EXPECT_EQ(actual, expect);
}
TEST(GroupWidgettTest, getCurSelectHeight)
{
    GroupWidget w;

    int actual = w.getCurSelectHeight();
    int expect = 0;
    EXPECT_EQ(actual, expect);

    QString searchGroupName(GRANDSEARCH_GROUP_FOLDER);
    MatchedItem item;
    MatchedItems items{item, item, item, item};
    w.appendMatchedItems(items, searchGroupName);

    int itemCount = w.m_listView->rowCount();
    ASSERT_GT(itemCount, 0);

    auto index = w.m_listView->model()->index(0, 0);
    ASSERT_TRUE(index.isValid());

    w.m_listView->setCurrentIndex(index);

    actual = w.getCurSelectHeight();
    EXPECT_GT(actual, expect);
}
TEST(GroupWidgettTest, reLayout)
{
    GroupWidget w;

    stub_ext::StubExt stu;

    bool ut_hide = false;
    stu.set_lamda(ADDR(QWidget, isHidden), [&](){
        return ut_hide;
    });

    w.reLayout();
    EXPECT_EQ(w.m_vContentLayout->spacing(), 10);

    ut_hide = true;
    w.reLayout();
    EXPECT_EQ(w.m_vContentLayout->spacing(), 0);
}

TEST(GroupWidgettTest, convertDisplayName)
{
    GroupWidget w;

    QString actual = w.convertDisplayName(GRANDSEARCH_GROUP_APP);
    QString expect = GroupName_App;
    EXPECT_EQ(actual, expect);

    QString testOther("TestOtherName");
    actual = w.convertDisplayName(testOther);
    EXPECT_EQ(actual, testOther);
}

TEST(GroupWidgettTest, updateShowItems)
{
    GroupWidget w;

    w.m_listView->clear();
    w.m_firstFiveItems.clear();

    stub_ext::StubExt stu;

    typedef void (*fptr)(QWidget*, bool);
    fptr QWidget_setVisible = (fptr)(&QWidget::setVisible);

    bool isCallSetVisible = false;
    stu.set_lamda(QWidget_setVisible, [&](){
        isCallSetVisible = true;
    });

    MatchedItems items;
    w.updateShowItems(items);

    EXPECT_TRUE(isCallSetVisible);
}

TEST(GroupWidgettTest, onMoreBtnClicked)
{
    GroupWidget w;

    w.onMoreBtnClicked();
    EXPECT_TRUE(w.m_cacheWeightItems.isEmpty());
    EXPECT_TRUE(w.m_cacheItems.isEmpty());
    EXPECT_TRUE(w.m_bListExpanded);
}

TEST(GroupWidgettTest, paintEvent)
{
    GroupWidget w;

    stub_ext::StubExt stu;

    bool ut_call = false;
    typedef void (*fptr)(QWidget*,QPaintEvent*);
    fptr ut_paintEvent = (fptr)(&QWidget::paintEvent);
    stu.set_lamda(ut_paintEvent, [&](){
        ut_call = true;
    });

    QPaintEvent event(QRect(0, 0, 100, 100));
    w.paintEvent(&event);
    EXPECT_TRUE(ut_call);
}
