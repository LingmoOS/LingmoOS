// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#define protected public
#define private public
#include <qaction.h>
#undef protected
#undef private
#include "toptoolbar.h"
#include "application.h"

#include "ccuttool.h"
#include "triangleitem.h"

#include <DFloatingButton>
#include <DComboBox>
#include <dzoommenucombobox.h>
#include "cspinbox.h"

#include <QDebug>
#include <DLineEdit>

#include "publicApi.h"

#if TEST_TRIANGLE_ITEM

TEST(TriangleItem, TestTriangleItemCreateView)
{
    createNewViewByShortcutKey();
}

TEST(TriangleItem, TestDrawTriangleItem)
{
//    PageView *view = getCurView();
//    ASSERT_NE(view, nullptr);
//    CCentralwidget *c = getMainWindow()->getCCentralwidget();
//    ASSERT_NE(c, nullptr);

//    drawApp->setCurrentTool(triangle);

//    int addedCount = view->pageScene()->allPageItems().count();
//    createItemByMouse(view);
//    ASSERT_EQ(view->pageScene()->allPageItems().count(), addedCount + 1);
//    ASSERT_EQ(view->pageScene()->allPageItems().first()->type(), TriangleType);

    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    Page *c = getMainWindow()->drawBoard()->currentPage();
    ASSERT_NE(c, nullptr);

    drawApp->setCurrentTool(triangle);

    int oldCount = view->pageScene()->allPageItems().count();

    createItemByMouse(view);

    ASSERT_EQ(getToolButtonStatus(eraser), false);

    drawApp->setCurrentTool(triangle);
    createItemByMouse(view, false, QPoint(500, 300), QPoint(600, 400), true, Qt::ShiftModifier);

    drawApp->setCurrentTool(triangle);
    createItemByMouse(view, false, QPoint(500, 300), QPoint(600, 400), true, Qt::AltModifier);

    drawApp->setCurrentTool(triangle);
    createItemByMouse(view, false, QPoint(500, 300), QPoint(600, 400), true, Qt::ShiftModifier | Qt::AltModifier);

    auto items   = view->pageScene()->allPageItems();

    int nowCount = items.count();

    ASSERT_EQ(nowCount - oldCount, 4);

    foreach (auto item, items) {
        ASSERT_EQ(item->type(), TriangleType);
    }
}

TEST(TriangleItem, TestCopyTriangleItem)
{
    keyShortCutCopyItem();
}

TEST(TriangleItem, TestTriangleItemProperty)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    VectorItem *item = dynamic_cast<VectorItem *>(view->pageScene()->allPageItems().first());

    // pen width
    setPenWidth(item, 4);
    ASSERT_EQ(item->pen().width(), 4);

    // stroke color
    QColor strokeColor(Qt::red);
    setStrokeColor(item, strokeColor);
    ASSERT_EQ(item->pen().color(), strokeColor);

    // brush color
    QColor brushColor(Qt::green);
    setBrushColor(item, brushColor);
}

TEST(TriangleItem, TestRightClick)
{
    itemRightClick();
}

TEST(TriangleItem, TestResizeTriangleItem)
{
    resizeItem();
}

TEST(TriangleItem, TestSelectAllTriangleItem)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);

    // 全选图元
    DTestEventList e;
    e.addMouseMove(QPoint(20, 20), 100);
    e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(10, 10), 100);
    e.addMouseMove(QPoint(1800, 900), 100);
    e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(1000, 1000), 100);
    e.addKeyPress(Qt::Key_A, Qt::ControlModifier, 100);
    e.addKeyRelease(Qt::Key_A, Qt::ControlModifier, 100);
    e.simulate(view->viewport());

    ASSERT_EQ(getToolButtonStatus(eraser), false);

    // 水平等间距对齐
    //view->m_itemsVEqulSpaceAlign->triggered(true);
    // 垂直等间距对齐
    //view->m_itemsHEqulSpaceAlign->triggered(true);

    //滚轮事件
    QWheelEvent wheelevent(QPointF(1000, 1000), 100, Qt::MouseButton::NoButton, Qt::KeyboardModifier::ControlModifier);
    view->wheelEvent(&wheelevent);
    QWheelEvent wheelevent2(QPointF(1000, 1000), 100, Qt::MouseButton::NoButton, Qt::KeyboardModifier::NoModifier);
    view->wheelEvent(&wheelevent2);
    QWheelEvent wheelevent3(QPointF(1000, 1000), 100, Qt::MouseButton::NoButton, Qt::KeyboardModifier::ShiftModifier);
    view->wheelEvent(&wheelevent3);
}

TEST(TriangleItem, TestLayerChange)
{
    layerChange();
}

TEST(TriangleItem, TestGroupUngroup)
{
    groupUngroup();
}

TEST(TriangleItem, TestSaveTriangleItemToFile)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    Page *c = getMainWindow()->drawBoard()->currentPage();
    ASSERT_NE(c, nullptr);

    // save ddf file
    QString TriangleItemPath = QApplication::applicationDirPath() + "/test_triangle.ddf";
    c->setFile(TriangleItemPath);
    c->save();
    c->close(true);
    QFileInfo info(TriangleItemPath);
    ASSERT_TRUE(info.exists());
}

TEST(TriangleItem, TestOpenTriangleItemFromFile)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);

    // 打开保存绘制的 ddf
    QString TriangleItemPath = QApplication::applicationDirPath() + "/test_triangle.ddf";

    QMimeData mimedata;
    QList<QUrl> li;
    li.append(QUrl(TriangleItemPath));
    mimedata.setUrls(li);

    const QPoint pos = view->viewport()->rect().center();
    QDragEnterEvent eEnter(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    dApp->sendEvent(view->viewport(), &eEnter);

    QDropEvent e(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    dApp->sendEvent(view->viewport(), &e);
    qMyWaitFor([ = ]() {
        return (view != getCurView());
    });

    view = getCurView();
    ASSERT_NE(view, nullptr);
    int addedCount = view->pageScene()->allPageItems().count();
    ASSERT_EQ(addedCount, 5);
    view->page()->close(true);
}

#endif
