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
#include "pageview.h"
#include "drawboard.h"
#include "pagescene.h"

#include <DFloatingButton>
#include <DComboBox>
#include <dzoommenucombobox.h>
#include "cspinbox.h"


#include <QDebug>
#include <DLineEdit>

#include "publicApi.h"

#ifdef TEST_ELLIPSE_ITEM

TEST(EllipseItem, TestEllipseItemCreateView)
{
    createNewViewByShortcutKey();
}

TEST(EllipseItem, TestDrawEllipseItem)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    Page *c = getMainWindow()->drawBoard()->currentPage();
    ASSERT_NE(c, nullptr);

    drawApp->setCurrentTool(ellipse);

    int oldCount = view->pageScene()->allPageItems().count();

    createItemByMouse(view);

    drawApp->setCurrentTool(ellipse);
    createItemByMouse(view, false, QPoint(500, 300), QPoint(600, 400), true, Qt::ShiftModifier);

    drawApp->setCurrentTool(ellipse);
    createItemByMouse(view, false, QPoint(500, 300), QPoint(600, 400), true, Qt::AltModifier);

    drawApp->setCurrentTool(ellipse);
    createItemByMouse(view, false, QPoint(500, 300), QPoint(600, 400), true, Qt::ShiftModifier | Qt::AltModifier);

    ASSERT_EQ(getToolButtonStatus(eraser), false);

    auto items   = view->pageScene()->allPageItems();

    int nowCount = items.count();

    ASSERT_EQ(nowCount - oldCount, 4);

    foreach (auto item, items) {
        ASSERT_EQ(item->type(), EllipseType);
    }
}

TEST(EllipseItem, TestCopyEllipseItem)
{
    int count    = currentSceneBzCount();

    keyShortCutCopyItem(1);

    int newCount = currentSceneBzCount();

    ASSERT_EQ(newCount - count, 1);
}

TEST(EllipseItem, TestEllipseItemProperty)
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

TEST(EllipseItem, TestItemAlignment)
{
    itemAlignment();
}

TEST(EllipseItem, TestRightClick)
{
    itemRightClick();
}

TEST(EllipseItem, TestResizeEllipseItem)
{
    resizeItem();
}

TEST(EllipseItem, TestSelectAllEllipseItem)
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
//    emit view->m_itemsVEqulSpaceAlign->triggered(true);
//    // 垂直等间距对齐
//    emit view->m_itemsHEqulSpaceAlign->triggered(true);

//    //滚轮事件
//    QWheelEvent wheelevent(QPointF(1000, 1000), 100, Qt::MouseButton::NoButton, Qt::KeyboardModifier::ControlModifier);
//    view->wheelEvent(&wheelevent);
//    QWheelEvent wheelevent2(QPointF(1000, 1000), 100, Qt::MouseButton::NoButton, Qt::KeyboardModifier::NoModifier);
//    view->wheelEvent(&wheelevent2);
//    QWheelEvent wheelevent3(QPointF(1000, 1000), 100, Qt::MouseButton::NoButton, Qt::KeyboardModifier::ShiftModifier);
//    view->wheelEvent(&wheelevent3);
}

TEST(EllipseItem, TestLayerChange)
{
    layerChange();
}

TEST(EllipseItem, TestGroupUngroup)
{
    groupUngroup();
}


TEST(EllipseItem, TestSaveEllipseItemToFile)
{
    PageView *view = getCurView();
    //int addedCount = view->drawScene()->getBzItems(view->drawScene()->items()).count();
    ASSERT_NE(view, nullptr);
    Page *c = getMainWindow()->drawBoard()->currentPage();
    ASSERT_NE(c, nullptr);

    // save ddf file
    QString EllipseItemPath = QApplication::applicationDirPath() + "/test_ellipse.ddf";
    c->setFile(EllipseItemPath);
    c->save();

    c->close(true);

    QFileInfo info(EllipseItemPath);
    ASSERT_TRUE(info.exists());
}

TEST(EllipseItem, TestOpenEllipseItemFromFile)
{
    PageView *view = getCurView();

    qWarning() << "name0 ========== ====== " << view->page()->name();
    ASSERT_NE(view, nullptr);

    // 打开保存绘制的 ddf
    QString EllipseItemPath = QApplication::applicationDirPath() + "/test_ellipse.ddf";

    QMimeData mimedata;
    QList<QUrl> li;
    li.append(QUrl(EllipseItemPath));
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
