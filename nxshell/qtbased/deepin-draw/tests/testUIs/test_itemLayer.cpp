// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../testItems/publicApi.h"
#include "application.h"
#include "pageview.h"
#include "pagescene.h"
#include "pageitem.h"
#include "pageitem_p.h"

#if TEST_Z_ITEM
TEST(ItemLayer, TestItemLayerCreateView)
{
    createNewViewByShortcutKey();
}

TEST(ItemLayer, TestItemLayerCreateItem)
{
    // [0] create item
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    Page *c = getMainWindow()->drawBoard()->currentPage();
    ASSERT_NE(c, nullptr);

    drawApp->setCurrentTool(rectangle);

    createItemByMouse(view);

    ASSERT_EQ(view->pageScene()->allPageItems().first()->type(), RectType);

    drawApp->setCurrentTool(rectangle);

    createItemByMouse(view, false, QPoint(550, 300), QPoint(650, 450), false);

    drawApp->setCurrentTool(rectangle);

    createItemByMouse(view, false, QPoint(600, 300), QPoint(700, 500), false);

    QList<PageItem *> items = view->pageScene()->allPageItems();

    ASSERT_EQ(items.count(), 3);
}

TEST(ItemLayer, TestItemLayerUP)
{
    PageView *view = getCurView();

    ASSERT_NE(view, nullptr);

    QList<PageItem *> items = view->pageScene()->allPageItems(EAesSort);

    ASSERT_EQ(items.count(), 3);

    ASSERT_EQ(items[0]->zValue(), 0);
    ASSERT_EQ(items[1]->zValue(), 1);
    ASSERT_EQ(items[2]->zValue(), 2);

    for (int i = 0; i < items.count(); i++) {
        PageItem *pItem = dynamic_cast<PageItem *>(items.at(i));

        view->pageScene()->clearSelections();
        view->pageScene()->selectPageItem(pItem);

        DTestEventList e;
        e.addKeyPress(Qt::Key_BracketRight, Qt::ControlModifier, 50);
        e.addKeyRelease(Qt::Key_BracketRight, Qt::ControlModifier, 50);
        e.simulate(view->viewport());

        //up one item also will make one down, so if one item not at top(z will not changed) then the bottom two item will be 1.
        ASSERT_EQ(pItem->zValue(), i != 2 ? 1 : 2);
    }
}

TEST(ItemLayer, TestItemLayerDown)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);

    QList<PageItem *> items = view->pageScene()->allPageItems(EDesSort);
    ASSERT_EQ(items.count(), 3);

    ASSERT_EQ(items[0]->zValue(), 2);
    ASSERT_EQ(items[1]->zValue(), 1);
    ASSERT_EQ(items[2]->zValue(), 0);

    for (int i = 0; i < items.count(); i++) {
        PageItem *pItem = items.at(i);
        view->pageScene()->clearSelections();
        view->pageScene()->selectPageItem(pItem);

        DTestEventList e;
        e.addKeyPress(Qt::Key_BracketLeft, Qt::ControlModifier, 50);
        e.addKeyRelease(Qt::Key_BracketLeft, Qt::ControlModifier, 50);
        e.simulate(view->viewport());

        //down one item also will make one up, so if one item not at bottom(z will not changed) then the top two item will be 1.
        ASSERT_EQ(pItem->zValue(), i != 2 ? 1 : 0);
    }
}

TEST(ItemLayer, TestItemSendToTop)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);

    //get scene items.
    auto items = view->pageScene()->allPageItems(EAesSort);

    ASSERT_EQ(items.count(), 3);

    ASSERT_EQ(items[0]->zValue(), 0);
    ASSERT_EQ(items[1]->zValue(), 1);
    ASSERT_EQ(items[2]->zValue(), 2);

    int maxZvalue = view->pageScene()->getMaxZValue();

    ASSERT_EQ(maxZvalue, 2);

    for (int i = 0; i < items.count(); i++) {
        PageItem *pItem = items.at(i);

        view->pageScene()->clearSelections();

        view->pageScene()->selectPageItem(pItem);

        //set to top then z should be the max
        DTestEventList e;
        e.addKeyPress(Qt::Key_BracketRight, Qt::ControlModifier | Qt::ShiftModifier, 100);
        e.addKeyRelease(Qt::Key_BracketRight, Qt::ControlModifier | Qt::ShiftModifier, 100);
        e.simulate(view->viewport());
        ASSERT_EQ(pItem->zValue(), maxZvalue);
    }
    //重新获取items
    view->pageScene()->clearSelections();
    items = view->pageScene()->allPageItems(EAesSort);

    view->pageScene()->selectPageItem(items[0]);
    view->pageScene()->selectPageItem(items[1]);

    DTestEventList e0;
    e0.addKeyPress(Qt::Key_BracketRight, Qt::ControlModifier | Qt::ShiftModifier, 100);
    e0.addKeyRelease(Qt::Key_BracketRight, Qt::ControlModifier | Qt::ShiftModifier, 100);
    e0.simulate(view->viewport());

    ASSERT_EQ(items[0]->zValue(), 1);
    ASSERT_EQ(items[1]->zValue(), 2);
    ASSERT_EQ(items[2]->zValue(), 0);

    //重新获取items
    view->pageScene()->clearSelections();
    items = view->pageScene()->allPageItems(EAesSort);

    view->pageScene()->selectPageItem(items[0]);
    view->pageScene()->selectPageItem(items[2]);

    DTestEventList e1;
    e1.addKeyPress(Qt::Key_BracketRight, Qt::ControlModifier | Qt::ShiftModifier, 100);
    e1.addKeyRelease(Qt::Key_BracketRight, Qt::ControlModifier | Qt::ShiftModifier, 100);
    e1.simulate(view->viewport());

    ASSERT_EQ(items[0]->zValue(), 1);
    ASSERT_EQ(items[1]->zValue(), 0);
    ASSERT_EQ(items[2]->zValue(), 2);
}

TEST(ItemLayer, TestItemLayerSendToButtom)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);

    //get scene items.
    auto items = view->pageScene()->allPageItems(EAesSort);

    ASSERT_EQ(items.count(), 3);

    ASSERT_EQ(items[0]->zValue(), 0);
    ASSERT_EQ(items[1]->zValue(), 1);
    ASSERT_EQ(items[2]->zValue(), 2);

    int minZvalue = 0;

    for (int i = 0; i < items.count(); i++) {
        PageItem *pItem = items.at(i);

        view->pageScene()->clearSelections();

        view->pageScene()->selectPageItem(pItem);

        //set to top then z should be the min:0
        DTestEventList e;
        e.addKeyPress(Qt::Key_BracketLeft, Qt::ControlModifier | Qt::ShiftModifier, 100);
        e.addKeyRelease(Qt::Key_BracketLeft, Qt::ControlModifier | Qt::ShiftModifier, 100);
        e.simulate(view->viewport());

        ASSERT_EQ(pItem->zValue(), minZvalue);
    }

    //重新获取items
    view->pageScene()->clearSelections();
    items = view->pageScene()->allPageItems(EAesSort);

    view->pageScene()->selectPageItem(items[1]);
    view->pageScene()->selectPageItem(items[2]);

    DTestEventList e0;
    e0.addKeyPress(Qt::Key_BracketLeft, Qt::ControlModifier | Qt::ShiftModifier, 100);
    e0.addKeyRelease(Qt::Key_BracketLeft, Qt::ControlModifier | Qt::ShiftModifier, 100);
    e0.simulate(view->viewport());

    ASSERT_EQ(items[0]->zValue(), 2);
    ASSERT_EQ(items[1]->zValue(), 0);
    ASSERT_EQ(items[2]->zValue(), 1);

    //重新获取items
    view->pageScene()->clearSelections();
    items = view->pageScene()->allPageItems(EAesSort);

    view->pageScene()->selectPageItem(items[0]);
    view->pageScene()->selectPageItem(items[2]);

    DTestEventList e1;
    e1.addKeyPress(Qt::Key_BracketLeft, Qt::ControlModifier | Qt::ShiftModifier, 100);
    e1.addKeyRelease(Qt::Key_BracketLeft, Qt::ControlModifier | Qt::ShiftModifier, 100);
    e1.simulate(view->viewport());

    ASSERT_EQ(items[0]->zValue(), 0);
    ASSERT_EQ(items[1]->zValue(), 2);
    ASSERT_EQ(items[2]->zValue(), 1);
}

TEST(ItemLayer, TestSaveItemLayerToFile)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    Page *c = getMainWindow()->drawBoard()->currentPage();
    ASSERT_NE(c, nullptr);

    // save ddf file
    QString path = QApplication::applicationDirPath() + "/test_itemLayer.ddf";
    c->setFile(path);
    c->save();
    c->close(true);

    QFileInfo info(path);
    ASSERT_TRUE(info.exists());
}
TEST(ItemLayer, TestOpenItemLayerFromFile)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);

    // 打开保存绘制的 ddf
    QString path = QApplication::applicationDirPath() + "/test_itemLayer.ddf";

    QMimeData mimedata;
    QList<QUrl> li;
    li.append(QUrl(path));
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

    view->page()->close(true);
}

TEST(ItemLayer, TestZItem)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    Page *c = getMainWindow()->drawBoard()->currentPage();
    ASSERT_NE(c, nullptr);

    drawApp->setCurrentTool(rectangle);

    createItemByMouse(view);

    auto items = view->pageScene()->allPageItems();

    zEndingItem(items, -1);
    zEndingItem(items, -2);
    zEndingItem(items, 0);
}

TEST(ItemLayer, TestGetCenter)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    Page *c = getMainWindow()->drawBoard()->currentPage();
    ASSERT_NE(c, nullptr);

    drawApp->setCurrentTool(rectangle);

    createItemByMouse(view);

    auto item = view->pageScene()->allPageItems().first();
    auto rect = item->itemRect();
    QPoint point;

//    point = item->getCenter(EDirection::LeftTop).toPoint();
//    ASSERT_EQ(point, rect.bottomRight().toPoint());

//    point = item->getCenter(CSizeHandleRect::EDirection::Top).toPoint();
//    ASSERT_EQ(point, QPointF(rect.center().x(), rect.bottom()).toPoint());

//    point = item->getCenter(CSizeHandleRect::EDirection::RightTop).toPoint();
//    ASSERT_EQ(point, rect.bottomLeft().toPoint());

//    point = item->getCenter(CSizeHandleRect::EDirection::Right).toPoint();
//    ASSERT_EQ(point, QPointF(rect.left(), rect.center().y()).toPoint());

//    point = item->getCenter(CSizeHandleRect::EDirection::RightBottom).toPoint();
//    ASSERT_EQ(point, rect.topLeft().toPoint());

//    point = item->getCenter(CSizeHandleRect::EDirection::Bottom).toPoint();
//    ASSERT_EQ(point, QPointF(rect.center().x(), rect.top()).toPoint());

//    point = item->getCenter(CSizeHandleRect::EDirection::LeftBottom).toPoint();
//    ASSERT_EQ(point, rect.topRight().toPoint());

//    point = item->getCenter(CSizeHandleRect::EDirection::Left).toPoint();
//    ASSERT_EQ(point, QPointF(rect.right(), rect.center().y()).toPoint());

//    point = item->getCenter(CSizeHandleRect::EDirection::Rotation).toPoint();
//    ASSERT_EQ(point, rect.center().toPoint());

//    point = item->getCenter(CSizeHandleRect::EDirection::None).toPoint();
//    ASSERT_EQ(point, rect.center().toPoint());
}

#endif
