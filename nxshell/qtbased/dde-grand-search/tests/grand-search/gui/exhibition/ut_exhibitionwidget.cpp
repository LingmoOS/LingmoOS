// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/matcheditem.h"
#include "utils/utils.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QPaintEvent>
#include <QEvent>
#include <QWidget>

#include "gui/exhibition/exhibitionwidget.h"
#include "gui/exhibition/exhibitionwidget_p.h"
#include "gui/exhibition/matchresult/matchwidget.h"
#include "gui/exhibition/preview/previewwidget.h"

using namespace testing;
using namespace GrandSearch;

TEST(ExhibitionWidgetTest, constructor)
{
    ExhibitionWidget *w = new ExhibitionWidget;

    ASSERT_TRUE(w);
    ASSERT_TRUE(w->d_p);
    ASSERT_TRUE(w->m_matchWidget);
    ASSERT_TRUE(w->m_previewWidget);

    delete w;
}

TEST(ExhibitionWidgetTest, clearData)
{
    ExhibitionWidget w;

    stub_ext::StubExt stu;

    bool ut_call = false;
    stu.set_lamda(ADDR(MatchWidget, clearMatchedData), [&](){
        ut_call = true;
    });

    w.clearData();
    EXPECT_TRUE(ut_call);
}

TEST(ExhibitionWidgetTest, onSelectNextItem)
{
    ExhibitionWidget w;

    stub_ext::StubExt stu;

    bool ut_hide = true;
    stu.set_lamda(ADDR(QWidget, isHidden), [&](){
        return ut_hide;
    });

    bool ut_call = false;
    stu.set_lamda(ADDR(MatchWidget, selectNextItem), [&](){
       ut_call = true;
    });

    w.onSelectNextItem();
    EXPECT_FALSE(ut_call);

    ut_hide = false;
    ut_call = false;
    w.onSelectNextItem();
    EXPECT_TRUE(ut_call);
}

TEST(ExhibitionWidgetTest, onSelectPreviousItem)
{
    ExhibitionWidget w;

    stub_ext::StubExt stu;

    bool ut_hide = true;
    stu.set_lamda(ADDR(QWidget, isHidden), [&](){
        return ut_hide;
    });

    bool ut_call = false;
    stu.set_lamda(ADDR(MatchWidget, selectPreviousItem), [&](){
       ut_call = true;
    });

    w.onSelectPreviousItem();
    EXPECT_FALSE(ut_call);

    ut_hide = false;
    ut_call = false;
    w.onSelectPreviousItem();
    EXPECT_TRUE(ut_call);
}

TEST(ExhibitionWidgetTest, onHandleItem)
{
    ExhibitionWidget w;

    stub_ext::StubExt stu;

    bool ut_hide = true;
    stu.set_lamda(ADDR(QWidget, isHidden), [&](){
        return ut_hide;
    });

    bool ut_call = false;
    stu.set_lamda(ADDR(MatchWidget, handleItem), [&](){
       ut_call = true;
    });

    w.onHandleItem();
    EXPECT_FALSE(ut_call);

    ut_hide = false;
    ut_call = false;
    w.onHandleItem();
    EXPECT_TRUE(ut_call);
}

TEST(ExhibitionWidgetTest, appendMatchedData)
{
    ExhibitionWidget w;

    stub_ext::StubExt stu;

    bool ut_call = false;
    stu.set_lamda(ADDR(MatchWidget, appendMatchedData), [&](){
       ut_call = true;
    });

    MatchedItemMap items;
    w.appendMatchedData(items);
    EXPECT_TRUE(ut_call);
}

TEST(ExhibitionWidgetTest, onSearchCompleted)
{
    ExhibitionWidget w;

    stub_ext::StubExt stu;

    bool ut_call = false;
    stu.set_lamda(ADDR(MatchWidget, onSearchCompleted), [&](){
       ut_call = true;
    });

    w.onSearchCompleted();
    EXPECT_TRUE(ut_call);
}

TEST(ExhibitionWidgetTest, previewItem)
{
    ExhibitionWidget w;

    stub_ext::StubExt stu;

    bool ut_can_preview = false;
    stu.set_lamda(ADDR(Utils, canPreview), [&](){
        return ut_can_preview;
    });

    bool ut_preview_item = false;
    stu.set_lamda(ADDR(PreviewWidget, previewItem), [&](){
        ut_preview_item = true;
        return true;
    });

    QString searchGroupName("test");
    MatchedItem item;
    item.name = "test";

    w.previewItem(searchGroupName, item);
    EXPECT_FALSE(ut_preview_item);

    ut_can_preview = true;
    ut_preview_item = false;
    w.previewItem(searchGroupName, item);
    EXPECT_TRUE(ut_preview_item);
}

TEST(ExhibitionWidgetTest, paintEvent)
{
    ExhibitionWidget w;

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
