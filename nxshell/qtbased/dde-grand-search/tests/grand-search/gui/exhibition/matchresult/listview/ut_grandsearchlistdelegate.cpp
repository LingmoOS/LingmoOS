// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gui/exhibition/matchresult/listview/grandsearchlistview.h"
#include "gui/exhibition/matchresult/listview/grandsearchlistdelegate.h"
#include "gui/exhibition/matchresult/listview/grandsearchlistmodel.h"
#include "global/matcheditem.h"
#include "global/builtinsearch.h"
#include "utils/utils.h"

#include "stubext.h"

#include <DStyledItemDelegate>
#include <DGuiApplicationHelper>

#include <gtest/gtest.h>

#include <QPaintEvent>
#include <QEvent>
#include <QWidget>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QPainter>
#include <QHelpEvent>
#include <QPen>

using namespace testing;
using namespace GrandSearch;
DWIDGET_USE_NAMESPACE;

TEST(GrandSearchListDelegateTest, constructor)
{
    auto view = new GrandSearchListView;
    GrandSearchListDelegate *delegate = view->m_delegate;

    ASSERT_TRUE(view);
    ASSERT_TRUE(delegate);

    delete view;
}

TEST(GrandSearchListDelegateTest, initStyleOption)
{
    GrandSearchListView view;
    GrandSearchListDelegate *delegate = view.m_delegate;

    typedef void (*fptr)(DStyledItemDelegate*,QStyleOptionViewItem *, const QModelIndex &);
    fptr ut_initStyleOption = (fptr)(&DStyledItemDelegate::initStyleOption);

    stub_ext::StubExt stu;

    bool ut_call_initStyleOption = false;
    stu.set_lamda(ut_initStyleOption, [&](){
        ut_call_initStyleOption = true;
    });

    QStyleOptionViewItem option;
    QModelIndex index;

    delegate->initStyleOption(&option, index);
    EXPECT_TRUE(ut_call_initStyleOption);
}

TEST(GrandSearchListDelegateTest, editorEvent)
{
    GrandSearchListView view;
    GrandSearchListDelegate *delegate = view.m_delegate;

    QEvent event(QEvent::FocusIn);
    GrandSearchListModel model(2, 2);
    QStyleOptionViewItem option;
    QModelIndex index;

    bool result = delegate->editorEvent(&event, &model, option, index);
    EXPECT_TRUE(result);
}

TEST(GrandSearchListDelegateTest, paint)
{
    GrandSearchListView view;
    GrandSearchListDelegate *delegate = view.m_delegate;

    stub_ext::StubExt stu;

    bool ut_call_drawSelectState = false;
    stu.set_lamda(&GrandSearchListDelegate::drawSelectState, [&](){
        ut_call_drawSelectState = true;
    });

    bool ut_call_drawSearchResultText = false;
    stu.set_lamda(&GrandSearchListDelegate::drawSearchResultText, [&](){
        ut_call_drawSearchResultText = true;
    });

    bool ut_call_paint = false;
    typedef void (*fptr)(QStyledItemDelegate*,QPainter *, const QStyleOptionViewItem &, const QModelIndex &);
    fptr ut_paint = (fptr)(&QStyledItemDelegate::paint);
    stu.set_lamda(ut_paint, [&](){
        ut_call_paint = true;
    });

    QPainter painter;
    QStyleOptionViewItem option;
    QModelIndex index;

    delegate->paint(&painter, option, index);

    EXPECT_TRUE(ut_call_drawSelectState);
    EXPECT_TRUE(ut_call_drawSearchResultText);
    EXPECT_TRUE(ut_call_paint);
}

TEST(GrandSearchListDelegateTest, sizeHint)
{
    GrandSearchListView view;
    GrandSearchListDelegate *delegate = view.m_delegate;

    QStyleOptionViewItem option;
    QModelIndex index;

    QSize actual = delegate->sizeHint(option, index);
    QSize expect(740, 36);

    EXPECT_EQ(actual.width(), expect.width());
    EXPECT_EQ(actual.height(), expect.height());
}

TEST(GrandSearchListDelegateTest, helpEvent)
{
    GrandSearchListView view;
    QString searchGroupName(GRANDSEARCH_GROUP_FOLDER);
    MatchedItem item;
    MatchedItems items{item, item};

    view.setMatchedItems(items);

    GrandSearchListDelegate *delegate = view.m_delegate;

    QHelpEvent helpEventFocusIn(QEvent::FocusIn, QPoint(10, 10), QPoint(100, 100));

    QStyleOptionViewItem option;
    QModelIndex index;

    delegate->helpEvent(&helpEventFocusIn, &view, option, index);

    QHelpEvent helpEventToolTip(QEvent::ToolTip, QPoint(10, 10), QPoint(100, 100));

    index = view.model()->index(0, 0);

    bool result = delegate->helpEvent(&helpEventToolTip, &view, option, index);
    EXPECT_TRUE(result);

    view.m_model->setData(index, QString("TestToolTip"), Qt::ToolTipRole);
    result = delegate->helpEvent(&helpEventToolTip, &view, option, index);
    EXPECT_TRUE(result);

    view.m_model->setData(index, QString(), Qt::ToolTipRole);
    result = delegate->helpEvent(&helpEventToolTip, &view, option, index);
    EXPECT_TRUE(result);
}

TEST(GrandSearchListDelegateTest, drawSelectState)
{
    GrandSearchListView view;
    GrandSearchListDelegate *delegate = view.m_delegate;

    QString searchGroupName(GRANDSEARCH_GROUP_FOLDER);
    MatchedItem item;
    MatchedItems items{item, item};

    view.setMatchedItems(items);

    stub_ext::StubExt stu;

    bool ut_call_save = false;
    stu.set_lamda(&QPainter::save, [&](){
        ut_call_save = true;
    });

    bool ut_call_restore = false;
    stu.set_lamda(&QPainter::restore, [&](){
        ut_call_restore = true;
    });

    QStyleOptionViewItem option;
    QModelIndex index = view.model()->index(0, 0);
    option.widget = &view;

    delegate->initStyleOption(&option, index);
    option.state = option.state | QStyle::State_Selected;

    QPainter painter;
    delegate->drawSelectState(&painter, option, index);
    EXPECT_FALSE(ut_call_save);
    EXPECT_FALSE(ut_call_restore);

    option.state = option.state & (~QStyle::State_Selected);
    option.state = option.state | QStyle::State_MouseOver;

    delegate->drawSelectState(&painter, option, index);
    EXPECT_TRUE(ut_call_save);
    EXPECT_TRUE(ut_call_restore);
}

TEST(GrandSearchListDelegateTest, drawSearchResultText)
{
    GrandSearchListView view;

    QString searchGroupName(GRANDSEARCH_GROUP_FOLDER);
    MatchedItem item;
    item.name = "QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ"
                "QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ";
    item.searcher = "com.deepin.dde-grand-search.web-statictext";
    MatchedItems items{item, item};

    view.setMatchedItems(items);

    QModelIndex index = view.model()->index(0, 0);
    view.setCurrentIndex(index);

    QPainter painter;
    QStyleOptionViewItem option;

    view.m_themeType = 2;
    option.widget = &view;
    option.rect = QRect(QPoint(10, 10), QSize(10, 10));
    option.state = option.state | QStyle::State_HasFocus;

    stub_ext::StubExt stu;

    bool ut_call_save = false;
    stu.set_lamda(&QPainter::save, [&](){
        ut_call_save = true;
    });

    bool ut_call_restore = false;
    stu.set_lamda(&QPainter::restore, [&](){
        ut_call_restore = true;
    });

    bool drawTail = false;
    stu.set_lamda(&GrandSearchListDelegate::drawTailText, [&](){
        drawTail = true;
    });

    view.m_isPreviewItem = false;
    view.m_delegate->drawSearchResultText(&painter, option, index);
    EXPECT_TRUE(ut_call_save);
    EXPECT_TRUE(ut_call_restore);
    EXPECT_TRUE(drawTail);

    ut_call_save = false;
    ut_call_restore = false;
    drawTail = false;

    view.m_isPreviewItem = true;
    view.m_themeType = 1;
    view.m_delegate->drawSearchResultText(&painter, option, index);
    EXPECT_TRUE(ut_call_save);
    EXPECT_TRUE(ut_call_restore);
    EXPECT_FALSE(drawTail);
}

TEST(GrandSearchListDelegateTest, drawTailText)
{
    GrandSearchListView view;
    QStyleOptionViewItem option;
    int textMaxWidth = 500;
    int actualStartx = 100;
    MatchedItem item;
    QStringList list;
    list.append("aaa");
    list.append("bbb");
    QVariantHash value({{GRANDSEARCH_PROPERTY_ITEM_TAILER, list}});
    item.extra = QVariant::fromValue(value);

    MatchedItems items{item, item};
    view.setMatchedItems(items);

    QModelIndex index = view.model()->index(0, 0);
    view.setCurrentIndex(index);

    view.onSetThemeType(DGuiApplicationHelper::DarkType);
    option.widget = &view;

    stub_ext::StubExt stu;
    stu.set_lamda(&GrandSearchListDelegate::calcTailShowData, [](){
        QMap<int, QString> map;
        map.insert(1, "aaa");
        map.insert(2, "bbb");
        return map;
    });

    bool draw = false;
    stu.set_lamda(&GrandSearchListDelegate::drawTailDetailedInfo, [&](){
        draw = true;
    });

    QPainter painter;
    view.m_delegate->drawTailText(&painter, option, index, textMaxWidth, actualStartx);
    EXPECT_TRUE(draw);

    view.onSetThemeType(DGuiApplicationHelper::LightType);
    draw = false;
    view.m_delegate->drawTailText(&painter, option, index, textMaxWidth, actualStartx);
    EXPECT_TRUE(draw);
}

TEST(GrandSearchListDelegateTest, drawTailDetailedInfo)
{
    QPainter painter;
    QStyleOptionViewItem option;
    QString text;
    QColor color;
    QFont font;
    int startX;

    text = "aaa";
    color = QColor("#FFFFFF");
    font = DFontSizeManager::instance()->get(DFontSizeManager::T8);
    QFontMetrics fontMetrics(font);
    startX = 100;
    option.rect.setHeight(50);

    stub_ext::StubExt stu;
    bool ut_call_save = false;
    stu.set_lamda(&QPainter::save, [&](){
        ut_call_save = true;
    });

    bool ut_call_restore = false;
    stu.set_lamda(&QPainter::restore, [&](){
        ut_call_restore = true;
    });

    GrandSearchListView view;
    view.m_delegate->drawTailDetailedInfo(&painter, option, text, color, font, fontMetrics, startX);
    EXPECT_TRUE(ut_call_save);
    EXPECT_TRUE(ut_call_restore);
}

TEST(GrandSearchListDelegateTest, calcTailShowInfo)
{
    int totalTailWidth = 500;
    int tailCount = 2;
    int averageWidth = 100;
    GrandSearchListView view;
    view.m_delegate->calcTailShowInfo(totalTailWidth, tailCount, averageWidth);
    EXPECT_EQ(245, averageWidth);

    totalTailWidth = 5;
    averageWidth = 10;
    view.m_delegate->calcTailShowInfo(totalTailWidth, tailCount, averageWidth);
    EXPECT_EQ(totalTailWidth, averageWidth);
}

TEST(GrandSearchListDelegateTest, calcTailShowData)
{
    QStringList strings;
    int tailCount;
    int averageWidth;
    QFont font;
    QFontMetrics fontMetrics(font);

    averageWidth = 200;

    stub_ext::StubExt stu;
    QMap<int, QString> map1;
    map1.insert(1, "aaa");
    stu.set_lamda(&GrandSearchListDelegate::calcTailShowDataByMaxWidth, [&](){
        return map1;
    });

    QMap<int, QString> map2;
    map2.insert(2, "bbb");
    stu.set_lamda(&GrandSearchListDelegate::calcTailShowDataByOptimalWidth, [&](){
        return map2;
    });

    GrandSearchListView view;
    QMap<int, QString> result = view.m_delegate->calcTailShowData(strings, tailCount, averageWidth, fontMetrics);
    EXPECT_EQ(result, map1);

    averageWidth = 100;
    result = view.m_delegate->calcTailShowData(strings, tailCount, averageWidth, fontMetrics);
    EXPECT_EQ(result, map2);
}

TEST(GrandSearchListDelegateTest, calcTailShowDataByMaxWidth)
{
    QStringList strings;
    int tailCount = 2;
    int averageWidth;
    QFont font;
    QFontMetrics fontMetrics(font);

    strings.append("aaa");
    strings.append("aaaaaaaaaaaaaa");
    averageWidth = 100;

    stub_ext::StubExt stu;
    stu.set_lamda(&QFontMetrics::elidedText, [](){
        QString s = "aa";
        return s;
    });

    QMap<int, QString> map;
    map.insert(0, "aa");
    map.insert(1, "aa");

    GrandSearchListView view;
    QMap<int, QString> result = view.m_delegate->calcTailShowDataByMaxWidth(strings, tailCount, averageWidth, fontMetrics);
    EXPECT_EQ(result, map);
}

TEST(GrandSearchListDelegateTest, calcTailShowDataByOptimalWidth)
{
    QStringList strings;
    int tailCount;
    int averageWidth;
    QFont font;
    QFontMetrics fontMetrics(font);

    strings.append("aa");
    strings.append("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
                   "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
                   "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
                   "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb");
    tailCount = 2;
    averageWidth = 160;

    QMap<int, QString> map;
    map.insert(0, "aa");
    map.insert(1, "a");

    stub_ext::StubExt stu;
    stu.set_lamda(&QFontMetrics::elidedText, [](){
        QString s = "a";
        return s;
    });

    GrandSearchListView view;
    QMap<int, QString> result = view.m_delegate->calcTailShowDataByOptimalWidth(strings, tailCount, averageWidth, fontMetrics);
    EXPECT_EQ(result, map);

    stu.set_lamda(&QFontMetrics::elidedText, [](){
       QString s = "ccccccccccccccccccccccccc";
       return s;
    });
    strings.clear();
    strings.append("ccccccccccccccccccccccccc");
    strings.append("a");
    tailCount = 2;
    result = view.m_delegate->calcTailShowDataByOptimalWidth(strings, tailCount, averageWidth, fontMetrics);
    map.clear();
    map.insert(0, "ccccccccccccccccccccccccc");
    map.insert(1, "a");
    EXPECT_EQ(map, result);
}
