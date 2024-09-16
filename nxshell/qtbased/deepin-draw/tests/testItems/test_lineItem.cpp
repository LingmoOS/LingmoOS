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
#include "lineitem.h"
#include "ccuttool.h"
#include <DFloatingButton>
#include <DComboBox>
#include <dzoommenucombobox.h>
#include "cspinbox.h"
#include "csizehandlerect.h"

#include <QDebug>
#include <DLineEdit>

#include "publicApi.h"

#if TEST_LINE_ITEM

TEST(LineItem, TestLineItemCreateView)
{
    createNewViewByShortcutKey();
}

TEST(LineItem, TestDrawLineItem)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    Page *c = getMainWindow()->drawBoard()->currentPage();
    ASSERT_NE(c, nullptr);

    drawApp->setCurrentTool(line);

    int oldCount = view->pageScene()->allPageItems().count();

    createItemByMouse(view);

    drawApp->setCurrentTool(line);
    createItemByMouse(view, false, QPoint(500, 300), QPoint(600, 400), true, Qt::ShiftModifier);

    drawApp->setCurrentTool(line);
    createItemByMouse(view, false, QPoint(500, 300), QPoint(600, 400), true, Qt::AltModifier);

    drawApp->setCurrentTool(line);
    createItemByMouse(view, false, QPoint(500, 300), QPoint(600, 400), true, Qt::ShiftModifier | Qt::AltModifier);

    ASSERT_EQ(getToolButtonStatus(eraser), false);

    auto items   = view->pageScene()->allPageItems();

    int nowCount = items.count();

    ASSERT_EQ(nowCount - oldCount, 4);

    foreach (auto item, items) {
        ASSERT_EQ(item->type(), LineType);
    }
}

TEST(LineItem, TestCopyLineItem)
{
    int count    = currentSceneBzCount();

    keyShortCutCopyItem(1);

    int newCount = currentSceneBzCount();

    ASSERT_EQ(newCount - count, 1);
}

TEST(LineItem, TestLineItemProperty)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    LineItem *line = dynamic_cast<LineItem *>(view->pageScene()->allPageItems().first());
    ASSERT_NE(line, nullptr);

    // pen width
    setPenWidth(line, 4);
    ASSERT_EQ(line->pen().width(), 4);

    // stroke color
    QColor strokeColor(Qt::red);
    setStrokeColor(line, strokeColor);
    ASSERT_EQ(line->pen().color(), strokeColor);

    // Start Type
    QComboBox *typeCombox = drawApp->topToolbar()->findChild<QComboBox *>("Line start style combox");
    ASSERT_NE(typeCombox, nullptr);
    for (int i = 0; i < typeCombox->count(); i++) {
        int defaultType = line->type();
        typeCombox->setCurrentIndex(i);
        QTest::qWait(100);
        ASSERT_EQ(line->type(), i);
        DTestEventList e;
        e.addKeyPress(Qt::Key_Z, Qt::ControlModifier, 100);
        e.simulate(view->viewport());
        ASSERT_EQ(line->type(), defaultType);
        e.clear();
        e.addKeyPress(Qt::Key_Y, Qt::ControlModifier, 100);
        e.simulate(view->viewport());
        ASSERT_EQ(line->type(), i);
    }

    // End Type
    typeCombox = drawApp->topToolbar()->findChild<QComboBox *>("Line end style combox");
    ASSERT_NE(typeCombox, nullptr);
    for (int i = 0; i < typeCombox->count(); i++) {
        int defaultType = line->type();
        typeCombox->setCurrentIndex(i);
        QTest::qWait(100);
        ASSERT_EQ(line->type(), i);

        DTestEventList e;
        e.addKeyPress(Qt::Key_Z, Qt::ControlModifier, 100);
        e.simulate(view->viewport());
        ASSERT_EQ(line->type(), defaultType);
        e.clear();
        e.addKeyPress(Qt::Key_Y, Qt::ControlModifier, 100);
        e.simulate(view->viewport());
        ASSERT_EQ(line->type(), i);
    }
}

TEST(LineItem, TestResizeLineItem)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);

    PageItem *pItem = dynamic_cast<PageItem *>(view->pageScene()->allPageItems().first());
    ASSERT_NE(pItem, nullptr);

    view->pageScene()->clearSelections();
    view->pageScene()->selectPageItem(pItem);

    Handles handles = pItem->handleNodes();

    // note: 等比拉伸(alt,shift)按住拉伸会失效
    for (int i = 0; i < handles.size(); ++i) {
        HandleNode *pNode = handles[i];
        QPoint posInView = view->mapFromScene(pNode->mapToScene(pNode->boundingRect().center()));
//        QRectF result = pItem->rect();
        DTestEventList e;
        e.addMouseMove(posInView, 100);
        e.addMousePress(Qt::LeftButton, Qt::ShiftModifier, posInView, 100);
        e.addMouseMove(posInView + QPoint(50, 50), 100);
        e.addMouseRelease(Qt::LeftButton, Qt::ShiftModifier, posInView + QPoint(50, 50), 100);
        e.simulate(view->viewport());
//        ASSERT_NE(pItem->rect(), result);
    }
}

TEST(LineItem, TestItemAlignment)
{
    itemAlignment();
}

TEST(LineItem, TestRightClick)
{
    itemRightClick();
}

TEST(LineItem, TestSelectAllLineItem)
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
    //emit view->m_itemsVEqulSpaceAlign->triggered(true);
    // 垂直等间距对齐
    //emit view->m_itemsHEqulSpaceAlign->triggered(true);

    //滚轮事件
    QWheelEvent wheelevent(QPointF(1000, 1000), 100, Qt::MouseButton::NoButton, Qt::KeyboardModifier::ControlModifier);
    view->wheelEvent(&wheelevent);
    QWheelEvent wheelevent2(QPointF(1000, 1000), 100, Qt::MouseButton::NoButton, Qt::KeyboardModifier::NoModifier);
    view->wheelEvent(&wheelevent2);
    QWheelEvent wheelevent3(QPointF(1000, 1000), 100, Qt::MouseButton::NoButton, Qt::KeyboardModifier::ShiftModifier);
    view->wheelEvent(&wheelevent3);
}

TEST(LineItem, TestLayerChange)
{
    layerChange();
}

TEST(LineItem, TestGroupUngroup)
{
    groupUngroup();
}

TEST(LineItem, TestSaveLineItemToFile)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    Page *c = getMainWindow()->drawBoard()->currentPage();
    ASSERT_NE(c, nullptr);

    ASSERT_EQ(view->pageScene()->allPageItems().count(), 5);

    // save ddf file
    QString LineItemPath = QApplication::applicationDirPath() + "/test_line.ddf";
    c->setFile(LineItemPath);
    c->save();

    QFileInfo info(LineItemPath);
    ASSERT_TRUE(info.exists());
}

TEST(LineItem, TestOpenLineItemFromFile)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);

    // 打开保存绘制的 ddf
    QString LineItemPath = QApplication::applicationDirPath() + "/test_line.ddf";

    QMimeData mimedata;
    QList<QUrl> li;
    li.append(QUrl(LineItemPath));
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
