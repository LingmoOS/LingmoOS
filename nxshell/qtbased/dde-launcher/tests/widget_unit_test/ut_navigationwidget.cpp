// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "navigationwidget.h"
#include "appslistmodel.h"
#undef private

#include <QTest>
#include <QSignalSpy>

#include "gtest/gtest.h"

class Tst_NavigationWidget : public testing::Test
{};

TEST_F(Tst_NavigationWidget, categoryButton_test)
{
    NavigationWidget widget;

    EXPECT_NE(widget.button(AppsListModel::Internet), nullptr);
    EXPECT_NE(widget.button(AppsListModel::Chat), nullptr);
    EXPECT_NE(widget.button(AppsListModel::Music), nullptr);
    EXPECT_NE(widget.button(AppsListModel::Video), nullptr);
    EXPECT_NE(widget.button(AppsListModel::Graphics), nullptr);
    EXPECT_NE(widget.button(AppsListModel::Game), nullptr);
    EXPECT_NE(widget.button(AppsListModel::Office), nullptr);
    EXPECT_NE(widget.button(AppsListModel::Reading), nullptr);
    EXPECT_NE(widget.button(AppsListModel::Development), nullptr);
    EXPECT_NE(widget.button(AppsListModel::System), nullptr);
    EXPECT_NE(widget.button(AppsListModel::Others), nullptr);
    EXPECT_EQ(widget.button(AppsListModel::AppCategory(-1)), nullptr);

    // sender为nullptr
    widget.buttonClicked();

    // sender为网络分类按钮
    QSignalSpy spy(widget.button(AppsListModel::Internet), SIGNAL(clicked()));
    widget.button(AppsListModel::Internet)->click();
}

TEST_F(Tst_NavigationWidget, event_test)
{
    NavigationWidget widget;

    QEnterEvent enterEvent(QPoint(0, 0), QPoint(10, 0), QPoint(10, 10));
    QApplication::sendEvent(&widget, &enterEvent);

    QTest::qWait(10);

    QSignalSpy spy(&widget, SIGNAL(mouseEntered()));
    QCOMPARE(spy.count(), 1);

    QEvent leaveEvent(QEvent::Leave);
    QApplication::sendEvent(&widget, &leaveEvent);
    QTest::qWait(50);

    QShowEvent showEvent;
    QApplication::sendEvent(&widget, &showEvent);
}
