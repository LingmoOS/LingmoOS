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
#include "polygonitem.h"
#include "ccuttool.h"

#include <DFloatingButton>
#include <DComboBox>
#include <dzoommenucombobox.h>
#include "cspinbox.h"

#include <QDebug>
#include <DLineEdit>

#include "publicApi.h"

#if TEST_POLYGON_ITEM

TEST(PolygonItem, TestPolygonItemCreateView)
{
    createNewViewByShortcutKey();
}

TEST(PolygonItem, TestDrawPolygonItem)
{
//    PageView *view = getCurView();
//    ASSERT_NE(view, nullptr);
//    CCentralwidget *c = getMainWindow()->getCCentralwidget();
//    ASSERT_NE(c, nullptr);

//    drawApp->setCurrentTool(polygon);

//    int addedCount = view->pageScene()->allPageItems().count();
//    createItemByMouse(view);
//    ASSERT_EQ(view->pageScene()->allPageItems().count(), addedCount + 1);
//    ASSERT_EQ(view->pageScene()->allPageItems().first()->type(), PolygonType);
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    Page *c = getMainWindow()->drawBoard()->currentPage();
    ASSERT_NE(c, nullptr);

    drawApp->setCurrentTool(polygon);

    int oldCount = view->pageScene()->allPageItems().count();

    createItemByMouse(view);

    drawApp->setCurrentTool(polygon);
    createItemByMouse(view, false, QPoint(500, 300), QPoint(600, 400), true, Qt::ShiftModifier);

    drawApp->setCurrentTool(polygon);
    createItemByMouse(view, false, QPoint(500, 300), QPoint(600, 400), true, Qt::AltModifier);

    drawApp->setCurrentTool(polygon);
    createItemByMouse(view, false, QPoint(500, 300), QPoint(600, 400), true, Qt::ShiftModifier | Qt::AltModifier);

    ASSERT_EQ(getToolButtonStatus(eraser), false);

    auto items   = view->pageScene()->allPageItems();

    int nowCount = items.count();

    ASSERT_EQ(nowCount - oldCount, 4);

    foreach (auto item, items) {
        ASSERT_EQ(item->type(), PolygonType);
    }
}

TEST(PolygonItem, TestCopyPolygonItem)
{
    int count    = currentSceneBzCount();

    keyShortCutCopyItem(1);

    int newCount = currentSceneBzCount();

    ASSERT_EQ(newCount - count, 1);
}

TEST(PolygonItem, TestPolygonItemProperty)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    PolygonItem *polygon  = dynamic_cast<PolygonItem *>(view->pageScene()->allPageItems().first());
    ASSERT_NE(polygon, nullptr);

    // pen width
    setPenWidth(polygon, 4);
    ASSERT_EQ(polygon->pen().width(), 4);

    // stroke color
    QColor strokeColor(Qt::red);
    setStrokeColor(polygon, strokeColor);
    ASSERT_EQ(polygon->pen().color(), strokeColor);

    // brush color
    QColor brushColor(Qt::green);
    setBrushColor(polygon, brushColor);

    // Polygon Side
    int defaultPoints = polygon->nPointsCount();
    CSpinBox *sp = drawApp->topToolbar()->findChild<CSpinBox *>("Polgon edges spinbox");
    ASSERT_NE(sp, nullptr);
    int value = sp->value() + 3;
    sp->setValue(value);
    QTest::qWait(100);
    ASSERT_EQ(polygon->nPointsCount(), sp->value());

    DTestEventList e;
    e.addKeyPress(Qt::Key_Z, Qt::ControlModifier, 100);
    e.simulate(view->viewport());

    ASSERT_EQ(polygon->nPointsCount(), defaultPoints);
    e.clear();
    e.addKeyPress(Qt::Key_Y, Qt::ControlModifier, 200);
    e.simulate(view->viewport());
    ASSERT_EQ(polygon->nPointsCount(), value);
}

TEST(PolygonItem, TestRightClick)
{
    itemRightClick();
}

TEST(PolygonItem, TestResizePolygonItem)
{
    resizeItem();
}

TEST(PolygonItem, TestSelectAllPolygonItem)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);

    // 全选图元
    DTestEventList e;
    e.addMouseMove(QPoint(20, 20), 100);
    e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(10, 10), 200);
    e.addMouseMove(QPoint(1800, 900), 100);
    e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(1000, 1000), 200);
    e.addKeyPress(Qt::Key_A, Qt::ControlModifier, 200);
    e.addKeyRelease(Qt::Key_A, Qt::ControlModifier, 200);
    e.simulate(view->viewport());

    ASSERT_EQ(getToolButtonStatus(eraser), false);

    // 水平等间距对齐
    //emit view->m_itemsVEqulSpaceAlign->triggered(true);
    // 垂直等间距对齐
    //emit view->m_itemsHEqulSpaceAlign->triggered(true);

    //滚轮事件
    QWheelEvent wheelevent(QPointF(1000, 1000), 200, Qt::MouseButton::NoButton, Qt::KeyboardModifier::ControlModifier);
    view->wheelEvent(&wheelevent);
    QWheelEvent wheelevent2(QPointF(1000, 1000), 200, Qt::MouseButton::NoButton, Qt::KeyboardModifier::NoModifier);
    view->wheelEvent(&wheelevent2);
    QWheelEvent wheelevent3(QPointF(1000, 1000), 200, Qt::MouseButton::NoButton, Qt::KeyboardModifier::ShiftModifier);
    view->wheelEvent(&wheelevent3);
}

TEST(PolygonItem, TestLayerChange)
{
    layerChange();
}

TEST(PolygonItem, TestGroupUngroup)
{
    groupUngroup();
}

TEST(PolygonItem, TestSavePolygonItemToFile)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    Page *c = getMainWindow()->drawBoard()->currentPage();
    ASSERT_NE(c, nullptr);

    // save ddf file
    QString PolygonItemPath = QApplication::applicationDirPath() + "/test_polygon.ddf";
    c->setFile(PolygonItemPath);
    c->save();
    c->close(true);

    QFileInfo info(PolygonItemPath);
    ASSERT_TRUE(info.exists());
}

TEST(PolygonItem, TestOpenPolygonItemFromFile)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);

    // 打开保存绘制的 ddf
    QString PolygonItemPath = QApplication::applicationDirPath() + "/test_polygon.ddf";

    QMimeData mimedata;
    QList<QUrl> li;
    li.append(QUrl(PolygonItemPath));
    mimedata.setUrls(li);

    const QPoint pos = view->viewport()->rect().center();
    QDragEnterEvent eEnter(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    dApp->sendEvent(view->viewport(), &eEnter);

    QDropEvent e(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    dApp->sendEvent(view->viewport(), &e);
    qMyWaitFor([ = ]() {
        return (view != getCurView() && getCurView()->pageScene()->allPageItems().count());
    });

    view = getCurView();
    ASSERT_NE(view, nullptr);
    int addedCount = view->pageScene()->allPageItems().count();
    ASSERT_EQ(addedCount, 5);
    view->page()->close(true);
}

#endif
