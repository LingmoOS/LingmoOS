// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#define protected public
#define private public
#include "pageview.h"
#include <qaction.h>
#undef protected
#undef private
#include "toptoolbar.h"
#include "pagescene.h"
#include "application.h"

#include "ccuttool.h"
#define protected public
#define private public
#include <DFloatingButton>
#include <DComboBox>
#include <dzoommenucombobox.h>
#include "cspinbox.h"
#include "cgraphicsitemevent.h"
#include "pentool.h"

#include <QDebug>
#include <DLineEdit>

#include "publicApi.h"

#if TEST_PEN_ITEM

TEST(PenItem, TestPenItemCreateView)
{
    createNewViewByShortcutKey();
}

TEST(PenItem, TestDrawPenItem)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    Page *c = getMainWindow()->drawBoard()->currentPage();
    ASSERT_NE(c, nullptr);

    int oldCount = view->pageScene()->allPageItems().count();

    drawApp->setCurrentTool(pen);

    createItemByMouse(view);

    ASSERT_EQ(getToolButtonStatus(eraser), true);

    auto items   = view->pageScene()->allPageItems();

    int nowCount = items.count();

    ASSERT_EQ(nowCount - oldCount, 1);

    foreach (auto item, items) {
        ASSERT_EQ(item->type(), RasterItemType);
    }
}

TEST(PenItem, TestPenItemProperty)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    RasterItem *pen = dynamic_cast<RasterItem *>(view->pageScene()->allPageItems().first());
    ASSERT_NE(pen, nullptr);

    view->pageScene()->selectPageItem(pen);
    setPenWidth(pen, 1);
    //ASSERT_EQ(pen->pen().width(), 1);

    // Start Type
//    DComboBox *typeCombox = drawApp->topToolbar()->findChild<DComboBox *>("Line start style combox");
//    ASSERT_NE(typeCombox, nullptr);
//    for (int i = 0; i < typeCombox->count(); i++) {
//        ELineType defaultType = pen->getPenStartType();
//        typeCombox->setCurrentIndex(i);
//        QTest::qWait(300);
//        ASSERT_EQ(pen->getPenStartType(), i);

//        DTestEventList e;
//        e.addKeyPress(Qt::Key_Z, Qt::ControlModifier, 200);
//        e.simulate(view->viewport());
//        ASSERT_EQ(pen->getPenStartType(), defaultType);
//        e.clear();
//        e.addKeyPress(Qt::Key_Y, Qt::ControlModifier, 200);
//        e.simulate(view->viewport());
//        ASSERT_EQ(pen->getPenStartType(), i);
//    }
}

//蜡笔测试
TEST(PenItem, TestCrayon)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    Page *c = getMainWindow()->drawBoard()->currentPage();
    ASSERT_NE(c, nullptr);

    drawApp->setCurrentTool(pen);

    createItemByMouse(view);

    //dynamic_cast<PenTool *>(drawApp->drawBoard()->tool(pen))->m_pPenStyleComboBox->setCurrentIndex(2);

    QTest::qWait(200);

    //坐标测算
    DTestEventList e;
    e.clear();
    e.addMousePress(Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(1030, 600), 200);
    e.addMouseMove(QPoint(1230, 700), 200);
    e.addMouseRelease(Qt::MouseButton::LeftButton);
    e.simulate(view->viewport());

    QTest::qWait(800);

    //橡皮擦
    ASSERT_EQ(getToolButtonStatus(eraser), true);

    e.clear();
    e.addKeyClick(Qt::Key_E, Qt::NoModifier, 200);
    e.simulate(view->viewport());

    //橡皮擦属性栏
    //auto eraserSpinBox = dynamic_cast<CSpinBoxSettingWgt *>(getMainWindow()->m_topToolbar->attributionsWgt()->widgetOfAttr(EEraserWidth))->spinBox();
    //eraserSpinBox->setValue(22);
    QTest::qWait(200);

    //disable menu
    QContextMenuEvent menuEvent(QContextMenuEvent::Mouse, QPoint(1200, 600));
    dApp->sendEvent(view->viewport(), &menuEvent);
    QTest::qWait(500);
    ASSERT_EQ(qobject_cast<QMenu *>(dApp->activePopupWidget()), nullptr);

    //hover
    e.clear();
    e.addMouseMove(QPoint(1040, 610));
    e.addDelay(500);
    e.addMouseMove(QPoint(10, 10));
    e.addDelay(500);

    //clear
    e.addMousePress(Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(1030, 600), 200);
    e.addMouseMove(QPoint(1230, 700), 200);
    e.addMouseRelease(Qt::MouseButton::LeftButton);
    e.addDelay(200);
    e.addKeyClick(Qt::Key_Escape, Qt::NoModifier, 200);
    e.simulate(view->viewport());

    /*DTestEventList e1;
    e1.addKeyPress(Qt::Key_Z, Qt::ControlModifier, 100);
    e1.simulate(view->viewport());
    e1.clear();
    e1.addKeyPress(Qt::Key_Y, Qt::ControlModifier, 100);
    e1.simulate(view->viewport());*/

    QTest::qWait(800);
}

//书法笔测试
TEST(PenItem, TestCalligraphyPen)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    Page *c = getMainWindow()->drawBoard()->currentPage();
    ASSERT_NE(c, nullptr);

    drawApp->setCurrentTool(pen);

    createItemByMouse(view);

    //dynamic_cast<CPenTool *>(drawApp->drawBoard()->tool(pen))->m_pPenStyleComboBox->setCurrentIndex(1);

    QTest::qWait(200);

    //坐标测算
    DTestEventList e;
    e.clear();
    e.addMousePress(Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(1080, 680), 200);
    e.addMouseMove(QPoint(1280, 780), 200);
    e.addMouseRelease(Qt::MouseButton::LeftButton);
    e.simulate(view->viewport());

    QTest::qWait(800);

    ASSERT_EQ(getToolButtonStatus(eraser), true);
}

//CPenTool
TEST(PenItem, TestCPenOtherFunction)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    Page *c = getMainWindow()->drawBoard()->currentPage();
    ASSERT_NE(c, nullptr);

    drawApp->setCurrentTool(pen);

    createItemByMouse(view);

    auto tool = dynamic_cast<PenTool *>(drawApp->drawBoard()->tool(pen));

    QTest::qWait(200);

    //eventfilter
    tool->eventFilter(nullptr, nullptr);

//    auto eventFilterObject = const_cast<QAbstractItemView *>(tool->m_pPenStyleComboBox->view());
//    QEvent e1(QEvent::Type::Show);
//    tool->eventFilter(reinterpret_cast<QObject *>(eventFilterObject), &e1);

//    QEvent e2(QEvent::Type::Hide);
//    tool->eventFilter(reinterpret_cast<QObject *>(eventFilterObject), &e2);

//    QEvent e3(QEvent::Type::None);
//    tool->eventFilter(reinterpret_cast<QObject *>(eventFilterObject), &e3);
}

TEST(PenItem, TestSavePenItemToFile)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    Page *c = getMainWindow()->drawBoard()->currentPage();
    ASSERT_NE(c, nullptr);

    // save ddf file
    QString PenItemPath = QApplication::applicationDirPath() + "/test_pen.ddf";
    c->setFile(PenItemPath);
    c->save();

    QFileInfo info(PenItemPath);
    ASSERT_TRUE(info.exists());
}

TEST(PenItem, TestOpenPenItemFromFile)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);

    // 打开保存绘制的 ddf
    QString PenItemPath = QApplication::applicationDirPath() + "/test_pen.ddf";

    QMimeData mimedata;
    QList<QUrl> li;
    li.append(QUrl(PenItemPath));
    mimedata.setUrls(li);

    const QPoint pos = view->viewport()->rect().center();
    QDragEnterEvent eEnter(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    dApp->sendEvent(view->viewport(), &eEnter);

    QDropEvent e(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    dApp->sendEvent(view->viewport(), &e);
    (void)QTest::qWaitFor([ = ]() {
        return (view != getCurView() && getCurView()->pageScene()->allPageItems().count());
    });

    view = getCurView();
    ASSERT_NE(view, nullptr);

    int addedCount = view->pageScene()->allPageItems().count();
    ASSERT_EQ(addedCount, 1);
    view->page()->close(true);
}

TEST(PenItem, TestOtherFunction)
{
//    //带起始点的构造函数
//    CGraphicsPenItem penItem(QPointF(50.0, 50.0));

//    //画笔属性读取
//    auto attrList = penItem.attributions();

//    //画笔属性设置
//    QVariant var;

//    var = QColor(1, 1, 1);
//    penItem.setAttributionVar(DrawAttribution::EPenColor, var, EChangedBegin);

//    var = 1;
//    penItem.setAttributionVar(DrawAttribution::EPenWidth, var, EChangedUpdate);

//    var = 1;
//    penItem.setAttributionVar(DrawAttribution::EStreakEndStyle, var, EChangedUpdate);

//    var = 1;
//    penItem.setAttributionVar(DrawAttribution::EStreakBeginStyle, var, EChangedUpdate);

//    var = 1;
//    penItem.setAttributionVar(999, var, 3);

//    //缩放
//    CGraphItemScalEvent scalEvent;
//    penItem.doScaling(&scalEvent);
//    penItem.testScaling(&scalEvent);

//    //rect获取
//    penItem.rect();

//    //graphics units获取
//    EDataReason reson = EDuplicate;
//    auto unit = penItem.getGraphicsUnit(reson);
//    unit.release();

//    //draw complete
//    penItem.updatePenPath(QPointF(60.0, 60.0), true);
//    QTest::qWait(200);
//    penItem.drawComplete(true);
//    QTest::qWait(200);
//    penItem.drawComplete(false);
//    QTest::qWait(200);

//    //get path
//    penItem.getPath();
//    penItem.getPenStartpath();
//    penItem.getPenEndpath();
//    penItem.setDrawFlag(true);
}

TEST(PenItem, TestOldPenItem)
{
//    createNewViewByShortcutKey();

//    PageView *view = getCurView();
//    ASSERT_NE(view, nullptr);
//    Page *c = getMainWindow()->drawBoard()->currentPage();
//    ASSERT_NE(c, nullptr);

//    {
//        CGraphicsPenItem *oldItem = new CGraphicsPenItem();
//        EXPECT_NE(oldItem, nullptr);
//        oldItem->setAttributionVar(EImageLeftRot, QVariant(), 0);
//        delete oldItem;
//    }
//    {
//        CGraphicsPenItem *oldItem = new CGraphicsPenItem(QPointF(0, 0));
//        EXPECT_NE(oldItem, nullptr);
//        c->scene()->addCItem(oldItem);

//        oldItem->updatePenPath(QPointF(10, 10), false);
//        oldItem->updatePenPath(QPointF(20, 15), false);
//        oldItem->updatePenPath(QPointF(25, 20), false);
//        oldItem->updatePenPath(QPointF(27, 33), false);
//        oldItem->updatePenPath(QPointF(29, 38), false);
//        oldItem->drawComplete(true);
//        EXPECT_EQ(oldItem->getPath().isEmpty(), false);

//        oldItem->setPenStartType(normalRing);
//        oldItem->setPenStartType(soildRing);
//        oldItem->setPenStartType(normalArrow);
//        oldItem->setPenStartType(soildArrow);
//        EXPECT_EQ(oldItem->getPenStartType(), soildArrow);

//        oldItem->setPenEndType(normalRing);
//        oldItem->setPenEndType(soildRing);
//        oldItem->setPenEndType(normalArrow);
//        oldItem->setPenEndType(soildArrow);
//        EXPECT_EQ(oldItem->getPenEndType(), soildArrow);

//        EXPECT_EQ(oldItem->isPosPenetrable(QPointF(0, 0)), false);

//        auto unit = oldItem->getGraphicsUnit(EDuplicate);
//        EXPECT_NE(unit.data.pPen, nullptr);
//        oldItem->loadGraphicsUnit(unit);
//        unit.release();

//        c->scene()->removeCItem(oldItem);

//        delete oldItem;
//    }
//    c->close(true);
}

#endif
