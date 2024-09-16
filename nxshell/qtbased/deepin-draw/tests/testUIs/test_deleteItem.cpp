// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#define protected public
#define private public
#include "qfiledialog.h"
#undef protected
#undef private

#include "application.h"

#include <DFloatingButton>
#include <DComboBox>
#include <dzoommenucombobox.h>
#include "cspinbox.h"
#include "pageview.h"
#include "pagescene.h"

#include <QDebug>
#include <QTimer>
#include <DFileDialog>
#include <QDialogButtonBox>

#include "../testItems/publicApi.h"

#if TEST_DELETE_ITEM

TEST(DeleteItem, TestDeleteItemCreateView)
{
    createNewViewByShortcutKey();
}

TEST(DeleteItem, TestDeleteItem)
{
    // Draw Triangle
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    Page *c = getMainWindow()->drawBoard()->currentPage();
    ASSERT_NE(c, nullptr);

    drawApp->setCurrentTool(triangle);

    int addedCount = view->pageScene()->allPageItems().count();
    createItemByMouse(view);
    ASSERT_EQ(view->pageScene()->allPageItems().count(), addedCount + 1);
    ASSERT_EQ(view->pageScene()->allPageItems().first()->type(), TriangleType);


    DTestEventList e;
    e.addKeyClick(Qt::Key_A, Qt::ControlModifier, 100);
    e.addKeyClick(Qt::Key_Delete, Qt::NoModifier, 100);
    e.simulate(getCurView());

    addedCount = view->pageScene()->allPageItems().count();
    ASSERT_EQ(addedCount, 0);

    view->page()->close(true);
}

#endif
