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
#include "staritem.h"
#include "ccuttool.h"
#include <DFloatingButton>
#include <DComboBox>
#include <dzoommenucombobox.h>

#define protected public
#define private public
#include "cspinbox.h"
#undef protected
#undef private

#include <QDebug>
#include <DLineEdit>
#include <QKeyEvent>

#include "publicApi.h"

#if TEST_START_ITEM

TEST(StartItem, TestStartItemCreateView)
{
    createNewViewByShortcutKey();
}

TEST(StartItem, TestDrawStartItem)
{
//    PageView *view = getCurView();
//    ASSERT_NE(view, nullptr);
//    CCentralwidget *c = getMainWindow()->getCCentralwidget();
//    ASSERT_NE(c, nullptr);

//    drawApp->setCurrentTool(polygonalStar);

//    int addedCount = view->pageScene()->allPageItems().count();
//    createItemByMouse(view);
//    ASSERT_EQ(view->pageScene()->allPageItems().count(), addedCount + 1);

//    ASSERT_EQ(view->pageScene()->allPageItems().first()->type(), PolygonalStarType);
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    Page *c = getMainWindow()->drawBoard()->currentPage();
    ASSERT_NE(c, nullptr);

    drawApp->setCurrentTool(polygonalStar);

    int oldCount = view->pageScene()->allPageItems().count();

    createItemByMouse(view);

    drawApp->setCurrentTool(polygonalStar);
    createItemByMouse(view, false, QPoint(500, 300), QPoint(600, 400), true, Qt::ShiftModifier);

    drawApp->setCurrentTool(polygonalStar);
    createItemByMouse(view, false, QPoint(500, 300), QPoint(600, 400), true, Qt::AltModifier);

    drawApp->setCurrentTool(polygonalStar);
    createItemByMouse(view, false, QPoint(500, 300), QPoint(600, 400), true, Qt::ShiftModifier | Qt::AltModifier);

    ASSERT_EQ(getToolButtonStatus(eraser), false);

    auto items   = view->pageScene()->allPageItems();

    int nowCount = items.count();

    ASSERT_EQ(nowCount - oldCount, 4);

    foreach (auto item, items) {
        ASSERT_EQ(item->type(), PolygonalStarType);
    }
}

TEST(StartItem, TestCopyStartItem)
{
    int count    = currentSceneBzCount();

    keyShortCutCopyItem(1);

    int newCount = currentSceneBzCount();

    ASSERT_EQ(newCount - count, 1);
}

TEST(StartItem, TestStartItemProperty)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    StarItem *start = dynamic_cast<StarItem *>(view->pageScene()->allPageItems().first());
    ASSERT_NE(start, nullptr);

    // pen width
    setPenWidth(start, 4);
    ASSERT_EQ(start->pen().width(), 4);

    // stroke color
    QColor strokeColor(Qt::red);
    setStrokeColor(start, strokeColor);
    ASSERT_EQ(start->pen().color(), strokeColor);

    // brush color
    QColor brushColor(Qt::green);
    setBrushColor(start, brushColor);

    // Start anchor
    int defaultAnchor = start->anchorNum();
    CSpinBox *sp = drawApp->topToolbar()->findChild<CSpinBox *>("Star Anchor spinbox");
    ASSERT_NE(sp, nullptr);
    int value = sp->value() * 2;
    sp->setValue(value);
    QTest::qWait(100);
    ASSERT_EQ(start->anchorNum(), sp->value());
    DTestEventList e;
    e.addKeyPress(Qt::Key_Z, Qt::ControlModifier, 100);
    e.simulate(view->viewport());
    ASSERT_EQ(start->anchorNum(), defaultAnchor);
    e.clear();
    e.addKeyPress(Qt::Key_Y, Qt::ControlModifier, 100);
    e.simulate(view->viewport());
    ASSERT_EQ(start->anchorNum(), value);

    // Start Radius
    int defaultRadius = start->innerRadius();
    sp = drawApp->topToolbar()->findChild<CSpinBox *>("Star inner radius spinbox");
    ASSERT_NE(sp, nullptr);
//    DTestEventList ee;
//    ee.addKeyPress(Qt::Key_Down, Qt::NoModifier, 100);
//    //ee.addKeyRelease(Qt::Key_Down, Qt::NoModifier, 100);
//    ee.addDelay(300);
//    ee.simulate(sp);

    QKeyEvent keyEvent(QEvent::None, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
    QKeyEvent keyEvent2(QEvent::None, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);
    sp->keyPressEvent(&keyEvent);
    sp->keyReleaseEvent(&keyEvent);
    sp->keyPressEvent(&keyEvent2);
    sp->keyReleaseEvent(&keyEvent2);

    value = sp->value() + 10;
    sp->setValue(value);

    QTest::qWait(100);
    ASSERT_EQ(start->innerRadius(), sp->value());
    ASSERT_EQ(start->innerRadius(), sp->value());
    e.clear();
    e.addKeyPress(Qt::Key_Z, Qt::ControlModifier, 200);
    e.simulate(view->viewport());
    ASSERT_EQ(start->innerRadius(), defaultRadius);
    e.clear();
    e.addKeyPress(Qt::Key_Y, Qt::ControlModifier, 200);
    e.simulate(view->viewport());
    ASSERT_EQ(start->innerRadius(), value);
}

TEST(StartItem, TestRightClick)
{
    itemRightClick();
}

TEST(StartItem, TestResizeStartItem)
{
    resizeItem();
}

TEST(StartItem, TestSelectAllStartItem)
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

TEST(StartItem, TestSaveStartItemToFile)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    Page *c = getMainWindow()->drawBoard()->currentPage();
    ASSERT_NE(c, nullptr);

    // save ddf file
    QString StartItemPath = QApplication::applicationDirPath() + "/test_start.ddf";
    c->setFile(StartItemPath);
    c->save();
    c->close(true);

    QFileInfo info(StartItemPath);
    ASSERT_TRUE(info.exists());
}

TEST(StartItem, TestOpenStartItemFromFile)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);

    // 打开保存绘制的 ddf
    QString StartItemPath = QApplication::applicationDirPath() + "/test_start.ddf";

    QMimeData mimedata;
    QList<QUrl> li;
    li.append(QUrl(StartItemPath));
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
