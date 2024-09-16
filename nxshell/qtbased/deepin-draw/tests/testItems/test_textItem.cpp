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
#include "textitem.h"
#include "ccuttool.h"
#include <DFloatingButton>
#include <DComboBox>
#include <dzoommenucombobox.h>
#include "cspinbox.h"

#include <QDebug>
#include <QAbstractItemView>
#include <DLineEdit>

#include "publicApi.h"

#if TEST_TEXT_ITEM

TEST(TextItem, TestTextItemCreateView)
{
    createNewViewByShortcutKey();
}

TEST(TextItem, TestDrawTextItem)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    Page *c = getMainWindow()->drawBoard()->currentPage();
    ASSERT_NE(c, nullptr);

    drawApp->setCurrentTool(text);

    int addedCount = view->pageScene()->allPageItems().count();
    createItemByMouse(view);
    ASSERT_EQ(getToolButtonStatus(eraser), false);
    ASSERT_EQ(view->pageScene()->allPageItems().count(), addedCount + 1);
    ASSERT_EQ(view->pageScene()->allPageItems().first()->type(), TextType);
}

TEST(TextItem, TestCopyTextItem)
{
    drawApp->topMainWindow()->setFocus();
    keyShortCutCopyItem();
}


TEST(TextItem, TestTextItemProperty)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    TextItem *text = dynamic_cast<TextItem *>(view->pageScene()->allPageItems().first());
    ASSERT_NE(text, nullptr);

    // Font color
    QColor color(Qt::red);
    ColorSettingButton *stroke = drawApp->topToolbar()->findChild<ColorSettingButton *>("Text color button");
    stroke->setColor(color);
    QTest::qWait(100);
    ASSERT_EQ(text->textColor(), color);

    // Font Family Type
    QComboBox *typeCombox = drawApp->topToolbar()->findChild<QComboBox *>("Text font family comboBox");
    ASSERT_NE(typeCombox, nullptr);
    QString family = "Bitstream Charter";//Andale Mono
    //typeCombox->activated(family);
    {
        DTestEventList eForFamilyEvent;
        eForFamilyEvent.addMouseClick(Qt::LeftButton, Qt::NoModifier,
                                      QPoint(typeCombox->width() - 10, typeCombox->rect().center().y()), 200);
        eForFamilyEvent.simulate(typeCombox);
    }
    {
        QTest::qWait(300);
        QString resultFamily = typeCombox->currentText();
//        DTestEventList eForFamilyEvent;
//        eForFamilyEvent.addMouseClick(Qt::LeftButton, Qt::NoModifier,
//                                      typeCombox->view()->viewport()->rect().center(), 300);
//        eForFamilyEvent.simulate(typeCombox->view()->viewport());
        typeCombox->hidePopup();
        qDebug() << "text->fontFamily() = " << text->fontFamily() << "resultFamily = " << resultFamily;
        ASSERT_EQ(text->fontFamily(), resultFamily);
    }

    QTest::qWait(100);
    //ASSERT_EQ(text->fontFamily(), family);

    // Font Style Type
    typeCombox = drawApp->topToolbar()->findChild<QComboBox *>("Text font style comboBox");
    ASSERT_NE(typeCombox, nullptr);
    QString style = "Regular";
    typeCombox->setCurrentText(style);
    QTest::qWait(100);
    ASSERT_EQ(text->fontStyle(), style);

    // Font Size Type
    typeCombox = drawApp->topToolbar()->findChild<QComboBox *>("Text font size comboBox");
    ASSERT_NE(typeCombox, nullptr);
//    QString size = "61px";
//    typeCombox->lineEdit()->setText(size);
//    typeCombox->lineEdit()->returnPressed();

    int sz = typeCombox->itemText(typeCombox->currentIndex() + 1).remove("px").toInt();
    typeCombox->setCurrentIndex(typeCombox->currentIndex() + 1);

    QTest::qWait(100);
    ASSERT_EQ(text->fontSize(), /*61*/sz);


    view->pageScene()->clearSelections();
    view->pageScene()->selectPageItem(text);

    DTestEventList e;

    QPoint posDClick = view->mapFromScene(text->mapToScene(text->rect().center()));
    e.addMouseMove(posDClick, 100);
    e.addDelay(100);
    e.simulate(view->viewport());

    static_cast<TextItem *>(text)->setEditing(true);

    QTest::qWait(100);

    QContextMenuEvent event(QContextMenuEvent::Mouse, QPoint(100, 100));
    dApp->sendEvent(view->viewport(), &event);

    e.clear();
    e.addDelay(100);
    e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(28, 270), 100);
    e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(28, 270), 100);
    e.addDelay(100);
    qDebug() << "QApplication::activePopupWidget() = " << QApplication::activePopupWidget();
    e.simulate(QApplication::activePopupWidget());

    dApp->sendEvent(view->viewport(), &event);
    e.clear();
    e.addDelay(100);
    e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(28, 250), 100);
    e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(28, 250), 100);
    e.addDelay(100);
    e.simulate(QApplication::activePopupWidget());

    dApp->sendEvent(view->viewport(), &event);
    e.clear();
    e.addDelay(100);
    e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(28, 290), 100);
    e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(28, 290), 100);
    e.addDelay(100);
    e.simulate(QApplication::activePopupWidget());

    dApp->sendEvent(view->viewport(), &event);
    e.clear();
    e.addDelay(100);
    e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(28, 100), 100);
    e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(28, 100), 100);
    e.addDelay(100);
    e.simulate(QApplication::activePopupWidget());

    dApp->sendEvent(view->viewport(), &event);
    e.clear();
    e.addDelay(100);
    e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(28, 45), 100);
    e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(28, 45), 100);
    e.addDelay(100);
    e.simulate(QApplication::activePopupWidget());

    dApp->sendEvent(view->viewport(), &event);
    e.clear();
    e.addDelay(100);
    e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(28, 10), 100);
    e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(28, 10), 100);
    e.addDelay(100);
    e.simulate(QApplication::activePopupWidget());

    dApp->sendEvent(view->viewport(), &event);
    e.clear();
    e.addDelay(100);
    e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(28, 80), 100);
    e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(28, 80), 100);
    e.addDelay(100);
    e.simulate(QApplication::activePopupWidget());

    dApp->sendEvent(view->viewport(), &event);
    e.clear();
    e.addDelay(100);
    e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(28, 100), 100);
    e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(28, 100), 100);
    e.addDelay(100);
    e.simulate(QApplication::activePopupWidget());

    dApp->sendEvent(view->viewport(), &event);
    e.clear();
    e.addDelay(100);
    e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(28, 130), 100);
    e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(28, 130), 100);
    e.addDelay(100);
    e.simulate(QApplication::activePopupWidget());
    dApp->sendEvent(view->viewport(), &event);
    e.clear();
    e.addDelay(100);
    e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(28, 170), 100);
    e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(28, 170), 100);
    e.addDelay(100);
    e.simulate(QApplication::activePopupWidget());

    dApp->sendEvent(view->viewport(), &event);
    e.clear();
    e.addDelay(100);
    e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(28, 200), 100);
    e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(28, 200), 100);
    e.addDelay(100);
    e.simulate(QApplication::activePopupWidget());

    e.addKeyPress(Qt::Key_L, Qt::ControlModifier | Qt::ShiftModifier, 100);
    e.addKeyRelease(Qt::Key_L, Qt::ControlModifier | Qt::ShiftModifier, 100);
    e.addDelay(100);
    e.addKeyPress(Qt::Key_R, Qt::ControlModifier | Qt::ShiftModifier, 100);
    e.addKeyRelease(Qt::Key_R, Qt::ControlModifier | Qt::ShiftModifier, 100);
    e.addDelay(100);
    e.addKeyPress(Qt::Key_T, Qt::ControlModifier | Qt::ShiftModifier, 100);
    e.addKeyRelease(Qt::Key_T, Qt::ControlModifier | Qt::ShiftModifier, 100);
    e.addDelay(100);
    e.addKeyPress(Qt::Key_B, Qt::ControlModifier | Qt::ShiftModifier, 100);
    e.addKeyRelease(Qt::Key_B, Qt::ControlModifier | Qt::ShiftModifier, 100);
    e.addDelay(100);
    e.addKeyPress(Qt::Key_V, Qt::ControlModifier | Qt::ShiftModifier, 100);
    e.addKeyRelease(Qt::Key_V, Qt::ControlModifier | Qt::ShiftModifier, 100);
    e.addDelay(100);
    e.addKeyPress(Qt::Key_H, Qt::ControlModifier | Qt::ShiftModifier, 100);
    e.addKeyRelease(Qt::Key_H, Qt::ControlModifier | Qt::ShiftModifier, 100);
    e.addDelay(100);
    e.addKeyPress(Qt::Key_Z, Qt::ControlModifier, 100);
    e.addKeyRelease(Qt::Key_Z, Qt::ControlModifier, 100);
    e.addDelay(100);
    e.addKeyPress(Qt::Key_Y, Qt::ControlModifier, 100);
    e.addKeyRelease(Qt::Key_Y, Qt::ControlModifier, 100);
    e.addDelay(100);
    e.simulate(view->viewport());


    view->pageScene()->clearSelections();
    view->pageScene()->selectPageItem(text);

    Handles handles = view->pageScene()->allPageItems().first()->handleNodes();

    int delay = 50;

    // 普通拉伸
    for (int i = 0; i < handles.size(); ++i) {
        HandleNode *pNode = handles[i];
        QPoint posInView = view->mapFromScene(pNode->mapToScene(pNode->boundingRect().center()));
        DTestEventList e;
        e.addMouseMove(posInView, delay);
        e.addMousePress(Qt::LeftButton, Qt::ShiftModifier, posInView, delay);
        e.addMouseMove(posInView + QPoint(20, 20), delay);
        e.addMouseRelease(Qt::LeftButton, Qt::ShiftModifier, posInView + QPoint(20, 20), delay);
        e.simulate(view->viewport());

        //  Undo Redo
        e.clear();
        e.addKeyPress(Qt::Key_Z, Qt::ControlModifier, delay);
        e.addKeyRelease(Qt::Key_Z, Qt::ControlModifier, delay);
        e.addDelay(100);
        e.clear();
        e.addKeyPress(Qt::Key_Y, Qt::ControlModifier, delay);
        e.addKeyRelease(Qt::Key_Y, Qt::ControlModifier, delay);
        e.simulate(view->viewport());
    }

    // SHIFT   ALT拉伸:  QTestEvent mouseMove 中移动鼠标的实现是直接设置全局鼠标位置 5.15中解决了此问题
    for (int i = 0; i < handles.size(); ++i) {
        HandleNode *pNode = handles[i];
        QPoint posInView = view->mapFromScene(pNode->mapToScene(pNode->boundingRect().center()));

        QMouseEvent mouseEvent(QEvent::MouseButtonPress, posInView, Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier);
        QApplication::sendEvent(view->viewport(), &mouseEvent);
        QTest::qWait(delay);

        QMouseEvent mouseEvent1(QEvent::MouseMove, posInView + QPoint(20, 20), Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier);
        QApplication::sendEvent(view->viewport(), &mouseEvent1);
        QTest::qWait(delay);

        QMouseEvent mouseEvent2(QEvent::MouseButtonRelease, posInView + QPoint(20, 20), Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier);
        QApplication::sendEvent(view->viewport(), &mouseEvent2);
        QTest::qWait(delay);
    }
    for (int i = 0; i < handles.size(); ++i) {
        HandleNode *pNode = handles[i];
        QPoint posInView = view->mapFromScene(pNode->mapToScene(pNode->boundingRect().center()));
        QMouseEvent mouseEvent(QEvent::MouseButtonPress, posInView, Qt::LeftButton, Qt::LeftButton, Qt::AltModifier);
        QApplication::sendEvent(view->viewport(), &mouseEvent);
        QTest::qWait(delay);

        QMouseEvent mouseEvent1(QEvent::MouseMove, posInView - QPoint(20, 20), Qt::LeftButton, Qt::LeftButton, Qt::AltModifier);
        QApplication::sendEvent(view->viewport(), &mouseEvent1);
        QTest::qWait(delay);

        QMouseEvent mouseEvent2(QEvent::MouseButtonRelease, posInView - QPoint(20, 20), Qt::LeftButton, Qt::LeftButton, Qt::AltModifier);
        QApplication::sendEvent(view->viewport(), &mouseEvent2);
        QTest::qWait(delay);
    }
    for (int i = 0; i < handles.size(); ++i) {
        HandleNode *pNode = handles[i];
        QPoint posInView = view->mapFromScene(pNode->mapToScene(pNode->boundingRect().center()));
        QMouseEvent mouseEvent(QEvent::MouseButtonPress, posInView, Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier | Qt::AltModifier);
        QApplication::sendEvent(view->viewport(), &mouseEvent);
        QTest::qWait(delay);

        QMouseEvent mouseEvent1(QEvent::MouseMove, posInView + QPoint(20, 20), Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier | Qt::AltModifier);
        QApplication::sendEvent(view->viewport(), &mouseEvent1);
        QTest::qWait(delay);

        QMouseEvent mouseEvent2(QEvent::MouseButtonRelease, posInView + QPoint(20, 20), Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier | Qt::AltModifier);
        QApplication::sendEvent(view->viewport(), &mouseEvent2);
        QTest::qWait(delay);
    }

    // 全选拉伸
    view->pageScene()->selectAll();
    handles = view->pageScene()->allPageItems().first() ->handleNodes();
    for (int i = 0; i < handles.size(); ++i) {
        HandleNode *pNode = handles[i];
        QPoint posInView = view->mapFromScene(pNode->mapToScene(pNode->boundingRect().center()));
        DTestEventList e;
        e.addMouseMove(posInView, 100);
        e.addMousePress(Qt::LeftButton, Qt::ShiftModifier, posInView, 100);
        e.addMouseMove(posInView - QPoint(50, 50), 100);
        e.addMouseRelease(Qt::LeftButton, Qt::ShiftModifier, posInView - QPoint(50, 50), 100);
        e.simulate(view->viewport());
    }
}


TEST(TextItem, TestSelectAllTextItem)
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

TEST(TextItem, TestLayerChange)
{
    layerChange();
}

TEST(TextItem, TestGroupUngroup)
{
    groupUngroup();
}

TEST(TextItem, TestSaveTextItemToFile)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    Page *c = getMainWindow()->drawBoard()->currentPage();
    ASSERT_NE(c, nullptr);

    // save ddf file
    QString TextItemPath = QApplication::applicationDirPath() + "/test_text.ddf";
    c->setFile(TextItemPath);
    c->save();
    c->close(true);
    QTest::qWait(100);

    QFileInfo info(TextItemPath);
    ASSERT_TRUE(info.exists());
}

TEST(TextItem, TestOpenTextItemFromFile)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);

    // 打开保存绘制的 ddf
    QString TextItemPath = QApplication::applicationDirPath() + "/test_text.ddf";

    QMimeData mimedata;
    QList<QUrl> li;
    li.append(QUrl(TextItemPath));
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
    ASSERT_EQ(addedCount, 2);
    view->page()->close(true);
}

#endif
