// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "searchwidget.h"
#undef private

#include <QTest>

#include "gtest/gtest.h"

class Tst_SearchWidget : public testing::Test
{};

TEST_F(Tst_SearchWidget, showToggle_test)
{
    SearchWidget widget;
    widget.showToggle();

    QCOMPARE(widget.categoryBtn()->isVisible(), true);
    QCOMPARE(widget.toggleModeBtn()->isVisible(), true);
}

TEST_F(Tst_SearchWidget, hideToggle_test)
{
    SearchWidget widget;

    widget.hideToggle();

    QCOMPARE(!widget.categoryBtn()->isVisible(), true);
    QCOMPARE(!widget.toggleModeBtn()->isVisible(), true);
}

TEST_F(Tst_SearchWidget, otherApi_test)
{
    SearchWidget widget;
    widget.updateSize(1.5, 1.2);

    widget.edit();

    widget.categoryBtn();
    widget.clearSearchContent();
    widget.setLeftSpacing(10);
    widget.setRightSpacing(10);

    widget.m_calcUtil->setDisplayMode(0);
    widget.updateCurrentCategoryBtnIcon();

    widget.m_calcUtil->setDisplayMode(1);
    widget.updateCurrentCategoryBtnIcon();
}
