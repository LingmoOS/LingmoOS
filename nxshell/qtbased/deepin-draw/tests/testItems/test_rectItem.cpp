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

#include "rectitem.h"
#include "ccuttool.h"
#include <DFloatingButton>
#include <DComboBox>
#include <dzoommenucombobox.h>
#include "cspinbox.h"

#include <QDebug>
#include <DLineEdit>

#include "publicApi.h"

#if TEST_RECT_ITEM

TEST(RectItem, TestRectItemCreateView)
{
    createNewViewByShortcutKey();
}

TEST(RectItem, TestDrawRectItem)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    Page *c = getMainWindow()->drawBoard()->currentPage();
    ASSERT_NE(c, nullptr);

    drawApp->setCurrentTool(rectangle);

    int oldCount = view->pageScene()->allPageItems().count();

    createItemByMouse(view);

    drawApp->setCurrentTool(rectangle);
    createItemByMouse(view, false, QPoint(500, 300), QPoint(600, 400), true, Qt::ShiftModifier);

    drawApp->setCurrentTool(rectangle);
    createItemByMouse(view, false, QPoint(500, 300), QPoint(600, 400), true, Qt::AltModifier);

    drawApp->setCurrentTool(rectangle);
    createItemByMouse(view, false, QPoint(500, 300), QPoint(600, 400), true, Qt::ShiftModifier | Qt::AltModifier);

    ASSERT_EQ(getToolButtonStatus(eraser), false);

    auto items   = view->pageScene()->allPageItems();

    int nowCount = items.count();

    ASSERT_EQ(nowCount - oldCount, 4);

    foreach (auto item, items) {
        ASSERT_EQ(item->type(), RectType);
    }
}

TEST(RectItem, TestCopyRectItem)
{
    int count    = currentSceneBzCount();

    keyShortCutCopyItem(1);

    int newCount = currentSceneBzCount();

    ASSERT_EQ(newCount - count, 1);
}

TEST(RectItem, TestRectItemProperty)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);

    RectItem *rect = dynamic_cast<RectItem *>(view->pageScene()->allPageItems().first());
    ASSERT_NE(rect, nullptr);

    // pen width
    setPenWidth(rect, 4);
    ASSERT_EQ(rect->pen().width(), 4);

    // stroke color
    QColor strokeColor(Qt::red);
    setStrokeColor(rect, strokeColor);
    ASSERT_EQ(rect->pen().color(), strokeColor);

    // brush color
    setBrushColor(rect, QColor(Qt::green));

    // Rect Radius
    int defaultRadius = rect->getXRadius();
    QSpinBox *sp = drawApp->topToolbar()->findChild<QSpinBox *>("Rect Radio spinbox");
    ASSERT_NE(sp, nullptr);
    int value = sp->value() * 10;
    sp->setValue(value);
    QTest::qWait(100);
    ASSERT_EQ(rect->getXRadius(), sp->value());

    DTestEventList e;
    e.addKeyPress(Qt::Key_Z, Qt::ControlModifier, 200);
    e.simulate(view->viewport());
    ASSERT_EQ(rect->getXRadius(), defaultRadius);
    e.clear();
    e.addKeyPress(Qt::Key_Y, Qt::ControlModifier, 200);
    e.simulate(view->viewport());
    ASSERT_EQ(rect->getXRadius(), value);
}

TEST(RectItem, TestRightClick)
{
    itemRightClick();
}

TEST(RectItem, TestResizeRectItem)
{
    resizeItem();
}

TEST(RectItem, TestSelectAllRectItem)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);

    // 全选图元
    DTestEventList e;
    e.addMouseMove(QPoint(20, 20), 100);
    e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(10, 10), 200);
    e.addMouseMove(QPoint(1800, 900), 100);
    e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(1000, 1000), 200);
    e.addKeyPress(Qt::Key_A, Qt::ControlModifier, 100);
    e.addKeyRelease(Qt::Key_A, Qt::ControlModifier, 100);
    e.simulate(view->viewport());

    ASSERT_EQ(getToolButtonStatus(eraser), false);

    // 水平等间距对齐
    //view->m_itemsVEqulSpaceAlign->triggered(true);
    // 垂直等间距对齐
    //view->m_itemsHEqulSpaceAlign->triggered(true);

    //滚轮事件
    QWheelEvent wheelevent(QPointF(1000, 1000), 200, Qt::MouseButton::NoButton, Qt::KeyboardModifier::ControlModifier);
    view->wheelEvent(&wheelevent);
    QWheelEvent wheelevent2(QPointF(1000, 1000), 200, Qt::MouseButton::NoButton, Qt::KeyboardModifier::NoModifier);
    view->wheelEvent(&wheelevent2);
    QWheelEvent wheelevent3(QPointF(1000, 1000), 200, Qt::MouseButton::NoButton, Qt::KeyboardModifier::ShiftModifier);
    view->wheelEvent(&wheelevent3);
}

TEST(RectItem, TestLayerChange)
{
    layerChange();
}

TEST(RectItem, TestGroupUngroup)
{
    groupUngroup();
}

TEST(RectItem, TestSaveRectItemToFile)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    Page *c = getMainWindow()->drawBoard()->currentPage();
    ASSERT_NE(c, nullptr);

    // save ddf file
    QString RectItemPath = QApplication::applicationDirPath() + "/test_rect.ddf";
    c->setFile(RectItemPath);
    c->save();
    c->close(true);

    QFileInfo info(RectItemPath);
    ASSERT_TRUE(info.exists());
}

TEST(RectItem, TestOpenRectItemFromFile)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);

    // 打开保存绘制的 ddf
    QString RectItemPath = QApplication::applicationDirPath() + "/test_rect.ddf";
    QMimeData mimedata;
    QList<QUrl> li;
    li.append(QUrl(RectItemPath));
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
