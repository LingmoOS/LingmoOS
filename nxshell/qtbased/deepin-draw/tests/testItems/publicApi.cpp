// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "publicApi.h"

//#define protected public
//#define private public

#include <QtTest>
#include <QTestEventList>
#include <qaction.h>

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "mainwindow.h"
#include "../../src/application.h"
#include "toptoolbar.h"
#include <DComboBox>
#include <DLineEdit>
#include <DWidget>
#include <DSlider>

#include "pageview.h"
#include "calphacontrolwidget.h"
#include <dzoommenucombobox.h>
#include "cspinbox.h"
#include "ciconbutton.h"
#include "pagescene.h"
#include "pageview.h"
#include "pageitem.h"
#include "textitem.h"

#include <QBrush>
#undef protected
#undef private

DWIDGET_USE_NAMESPACE

extern PageItem *_pageSceneSelectItem(PageScene *scene);
MainWindow *getMainWindow()
{
    if (drawApp->topMainWindow() == nullptr) {
        drawApp->showMainWindow(QStringList());
        drawApp->topMainWindow()->showMaximized();
    }
    return drawApp->topMainWindow();
}

PageView *getCurView()
{
    if (getMainWindow() != nullptr) {
        if (nullptr == drawApp->drawBoard()->currentPage()) {
            drawApp->drawBoard()->addPage();
        }

        return drawApp->drawBoard()->currentPage()->view();
    }
    return nullptr;
}

class DMouseMoveEvent: public QTestMouseEvent
{
public:
    //继承父亲的构造函数
    using QTestMouseEvent::QTestMouseEvent;

    void simulate(QWidget *w) override
    {
        if (_action == QTest::MouseMove) {
            if (_delay == -1 || _delay < 100)
                _delay = 100;
            if (_delay > 0) {
                QTest::qWait(_delay);
                QTest::lastMouseTimestamp += _delay;
            }

            QMouseEvent me(QEvent::MouseMove, _pos, _button, _button, _modifiers);
            me.setTimestamp(ulong(++QTest::lastMouseTimestamp));
            QSpontaneKeyEvent::setSpontaneous(&me);
            if (!dApp->notify(w, &me)) {
                static const char *const mouseActionNames[] =
                { "MousePress", "MouseRelease", "MouseClick", "MouseDClick", "MouseMove" };
                QString warning = QString::fromLatin1("Mouse event \"%1\" not accepted by receiving widget");
                QTest::qWarn(warning.arg(QString::fromLatin1(mouseActionNames[static_cast<int>(_action)])).toLatin1().data());
            }
        } else {
            QTestMouseEvent::simulate(w);
        }
    }

    QTestEvent *clone() const override
    {
        return new DMouseMoveEvent(*this);
    }
};

void DTestEventList::addMousePress(Qt::MouseButton button, Qt::KeyboardModifiers stateKey,
                                   QPoint pos, int delay)
{
    _pressedMouseButton = button;
    QTestEventList::addMousePress(button, stateKey, pos, delay);
}
void DTestEventList::addMouseRelease(Qt::MouseButton button, Qt::KeyboardModifiers stateKey,
                                     QPoint pos, int delay)
{
    _pressedMouseButton = Qt::NoButton;
    QTestEventList::addMouseRelease(button, stateKey, pos, delay);
}
void DTestEventList::addMouseMove(QPoint pos, int delay, Qt::KeyboardModifiers stateKey)
{
    append(new DMouseMoveEvent(QTest::MouseMove, _pressedMouseButton, stateKey, pos, delay));
}
void DTestEventList::simulate(QWidget *w)
{
    //保证离屏时的焦点问题
    {
        if (w->window() != qApp->activePopupWidget()) {
            if (qApp->activePopupWidget() != nullptr) {
                qApp->activePopupWidget()->hide();
            }
            if (w->window() != nullptr) {
                w->window()->activateWindow();
            }
        }
        w->setFocus();
    }

    for (int i = 0; i < count(); ++i)
        at(i)->simulate(w);
}

void  createNewViewByShortcutKey()
{
    Q_UNUSED(QTest::qWaitFor([ = ]() {
        return getCurView() != nullptr;
    }))

    auto oldView = getCurView();
    if (oldView == nullptr) {
        qDebug() << __FILE__ << __LINE__ << "get PageView is nullptr.";
    }
    ASSERT_NE(oldView, nullptr);

    DTestEventList e;
    e.addKeyClick(Qt::Key_N, Qt::ControlModifier, 200);
    //qWarning() << "getCurView0 --------------------------" << qApp->focusWidget() << qApp->focusWindow();
    e.simulate(getCurView());
    //qWarning() << "getCurView1 --------------------------" << qApp->focusWidget() << qApp->focusWindow();
    Q_UNUSED(QTest::qWaitFor([ = ]() {
        return getCurView() != oldView;
    }));

    if (getCurView() == nullptr) {
        qDebug() << __FILE__ << __LINE__ << "get PageView is nullptr.";
    }
    ASSERT_NE(getCurView(), nullptr);
    ASSERT_NE(getCurView(), oldView);
}

void setPenWidth(PageItem *item, int width)
{
    int defaultWidth = static_cast<VectorItem *>(item)->pen().width();

    QComboBox *sideComBox = drawApp->topToolbar()->findChild<QComboBox *>("Line width combox");

    int add = 0;
    if (sideComBox == nullptr) {
        add = -1;
        sideComBox = drawApp->topToolbar()->findChild<QComboBox *>("Pen width combox");
    }

    // pen width 0 1 2 4 8 10 px
//    if (width == 0 || width == 1 || width == 2) {
//        sideComBox->setCurrentIndex(width + add);
//    } else if (width == 4) {
//        sideComBox->setCurrentIndex(3 + add);
//    } else if (width == 8 || width == 10) {
//        sideComBox->setCurrentIndex(width / 2 + add);
//    }
    QTest::qWait(100);

    DTestEventList e;
    e.addKeyPress(Qt::Key_Z, Qt::ControlModifier, 100);
    e.simulate(item->pageScene()->page()->view()->viewport());
    ASSERT_EQ(static_cast<VectorItem *>(item)->pen().width(), defaultWidth);
    e.clear();
    e.addKeyPress(Qt::Key_Y, Qt::ControlModifier, 100);
    e.simulate(item->pageScene()->page()->view()->viewport());
    ASSERT_EQ(static_cast<VectorItem *>(item)->pen().width(), width);
}

void setStrokeColor(PageItem *item, QColor color)
{
    QColor defaultColor = static_cast<VectorItem *>(item)-> pen().color();
    qMyWaitFor([ = ]() {
        return drawApp->topToolbar()->findChild<ColorSettingButton *>("stroken color button") != nullptr;
    });
    ColorSettingButton *stroke = drawApp->topToolbar()->findChild<ColorSettingButton *>("stroken color button");
    stroke->setColor(color);
    QTest::qWait(100);

    DTestEventList e;
    e.addKeyPress(Qt::Key_Z, Qt::ControlModifier, 200);
    e.simulate(item->pageScene()->page()->view()->viewport());
    ASSERT_EQ(static_cast<VectorItem *>(item)->pen().color(), defaultColor);
    e.clear();
    e.addKeyPress(Qt::Key_Y, Qt::ControlModifier, 200);
    e.simulate(item->pageScene()->page()->view()->viewport());
    ASSERT_EQ(static_cast<VectorItem *>(item)->pen().color(), color);
}

void setBrushColor(PageItem *item, QColor color)
{
    QColor defaultColor = static_cast<VectorItem *>(item)->brush().color();

    qMyWaitFor([ = ]() {
        return drawApp->topToolbar()->findChild<ColorSettingButton *>("fill color button") != nullptr;
    });

    ColorSettingButton *brush = drawApp->topToolbar()->findChild<ColorSettingButton *>("fill color button");

    brush->setColor(color);

    //qWarning() << "setBrushColor1" << qApp->focusWidget() << qApp->focusObject();;
    DTestEventList e;
    e.addKeyPress(Qt::Key_A, Qt::ControlModifier, 200);
    e.addKeyPress(Qt::Key_Z, Qt::ControlModifier, 200);
    e.simulate(item->pageScene()->page()->view()->viewport());
    ASSERT_EQ(static_cast<VectorItem *>(item)->brush().color(), defaultColor);
    e.clear();
    //qWarning() << "setBrushColor2" << qApp->focusWidget() << qApp->focusObject();;
    e.addKeyPress(Qt::Key_Y, Qt::ControlModifier, 200);
    e.simulate(item->pageScene()->page()->view()->viewport());
    ASSERT_EQ(static_cast<VectorItem *>(item)->brush().color(), color);

    item = dynamic_cast<PageItem *>(item->pageScene()->allPageItems().first());
    //   [0]  show colorPanel
    QMouseEvent mousePressEvent(QEvent::MouseButtonPress, QPointF(5, 5),
                                Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    dApp->sendEvent(brush, &mousePressEvent);
    QTest::qWait(200);
    CColorPickWidget *pickColor = brush->colorPick();
    ASSERT_NE(pickColor, nullptr);

//    pickColor->setTheme(0);
//    pickColor->setExpandWidgetVisble(false);
//    pickColor->setExpandWidgetVisble(true);

//    //  [1]  Color  LineEdit
//    DLineEdit *colorLineEdit = pickColor->findChild<DLineEdit *>("ColorLineEdit");
//    QTest::qWait(200);
//    ASSERT_NE(colorLineEdit, nullptr);
//    colorLineEdit->setText("8fc31f");
//    QTest::qWait(200);
//    ASSERT_EQ(static_cast<VectorItem *>(item)->brush().color(), pickColor->color());
//    ASSERT_EQ(static_cast<VectorItem *>(item)->brush().color(), QColor("#8fc31f")); //这里初始化QColor要把#加上

//    //  [2]  Color  Alpha
//    CAlphaControlWidget *alphaControlWidget = pickColor->findChild<CAlphaControlWidget *>("CAlphaControlWidget");
//    DSlider *slider = alphaControlWidget->findChild<DSlider *>("Color Alpha slider");
//    ASSERT_NE(slider, nullptr);
//    slider->setValue(155);
//    ASSERT_EQ(static_cast<VectorItem *>(item)->paintBrush().color().alpha(), slider->value());

//    //  [3]  show expand Color  Panel
//    CIconButton *iconbutton = pickColor->findChild<CIconButton *>("CIconButton");
//    ASSERT_NE(iconbutton, nullptr);
//    QEvent event(QEvent::Enter);
//    dApp->sendEvent(iconbutton, &event);
//    QTest::qWait(200);
//    dApp->sendEvent(iconbutton, &mousePressEvent);
//    QTest::qWait(200);
//    QMouseEvent mouseReleaseEvent(QEvent::MouseButtonRelease, QPointF(5, 5), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
//    dApp->sendEvent(iconbutton, &mouseReleaseEvent);
//    QTest::qWait(200);
//    event = QEvent(QEvent::Leave);
//    dApp->sendEvent(iconbutton, &event);
//    QTest::qWait(200);

//    //  [4]  picker color
//    PickColorWidget *picker = pickColor->findChild<PickColorWidget *>("PickColorWidget");
//    color.setAlpha(100);
//    picker->setColor(color);

//    pickColor->hide();

    ASSERT_EQ(static_cast<VectorItem *>(item)->paintBrush().color(), color);
}

void resizeItem()
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);

    PageItem *pItem = dynamic_cast<PageItem *>(view->pageScene()->allPageItems().first());
    ASSERT_NE(pItem, nullptr);

    view->pageScene()->clearSelections();
    view->pageScene()->selectPageItem(pItem);


    Handles handles = view->pageScene()->selectedPageItems().first()->handleNodes();

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
        e.addDelay(200);
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
    }

    // 全选拉伸
    view->pageScene()->selectAll();
    handles = _pageSceneSelectItem(view->pageScene())->handleNodes();
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

void createItemByMouse(PageView *view, bool altCopyItem,
                       QPoint topLeft, QPoint bottomRight,
                       bool doUndoRedo, Qt::KeyboardModifiers stateKey)
{
    DTestEventList e;
    e.clear();
    e.addMouseMove(topLeft, 100);

    e.addMousePress(Qt::LeftButton, stateKey, topLeft, 100);
    e.addMouseMove(bottomRight / 2, 100, stateKey);
    e.addMouseMove(bottomRight, 100, stateKey);

    e.addMouseRelease(Qt::LeftButton, stateKey, bottomRight, 100);
    e.addMouseClick(Qt::LeftButton, stateKey, QPoint(10, 10), 100);
    e.simulate(view->viewport());

    //int addedCount = view->pageScene()->allPageItems().count();

    // alt move copy item will not sucess,because move event has no modifier
    if (altCopyItem) {
        e.clear();
        QPoint p = bottomRight - QPoint(30, 30);
        e.addMouseMove(p, 100);
        e.addKeyPress(Qt::Key_Alt, Qt::AltModifier, 100);
        e.addMousePress(Qt::LeftButton, Qt::AltModifier, p, 100);
        e.addMouseMove(bottomRight + QPoint(50, 50), 100);
        e.addMouseRelease(Qt::LeftButton, Qt::AltModifier, bottomRight + QPoint(50, 50), 100);
        e.addKeyRelease(Qt::Key_Alt, Qt::AltModifier, 100);
        e.simulate(view->viewport());
    }

    if (doUndoRedo) {
        //int addedCount = view->pageScene()->allPageItems().count();
        DTestEventList e;
        e.addKeyPress(Qt::Key_Z, Qt::ControlModifier, 200);
        e.simulate(view->viewport());

        e.clear();
        //addedCount = view->pageScene()->allPageItems().count();
        e.addKeyPress(Qt::Key_Y, Qt::ControlModifier, 200);
        e.simulate(view->viewport());
    }
}

void createItemWithkeyShift(PageView *view, bool altCopyItem, QPoint startPos, QPoint endPos)
{
    QPoint topRight = startPos + QPoint(100, -50);
    QPoint topLeft = startPos + QPoint(-100, -50);
    QPoint bottomLeft = startPos + QPoint(-100, 50);
    DTestEventList e;
    e.clear();
    e.addMouseMove(topLeft, 100);
    e.addKeyPress(Qt::Key_Shift, Qt::NoModifier, 100);
    e.addMousePress(Qt::LeftButton, Qt::NoModifier, topLeft, 100);
    e.addMouseMove(topRight, 300);
    e.addMouseMove(topLeft, 300);
    e.addMouseMove(bottomLeft, 300);
    e.addMouseMove(endPos, 300);
    e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, endPos, 100);
    e.addKeyRelease(Qt::Key_Shift, Qt::NoModifier, 100);
    e.simulate(view->viewport());

    if (altCopyItem) {
        e.clear();
        e.addMouseMove(endPos / 2, 100);
        e.addMousePress(Qt::LeftButton, Qt::AltModifier, endPos / 2, 100);
        e.addMouseMove(endPos + QPoint(50, 50), 100);
        e.addMouseRelease(Qt::LeftButton, Qt::AltModifier, endPos + QPoint(50, 50), 100);
        e.simulate(view->viewport());
    }
}
void keyShortCutCopyItem(int addTimes)
{
    //auto currentTool = drawApp->currentTool();
    drawApp->setCurrentTool(selection);
    PageView *view = getCurView();

    if (view == nullptr)
        return;

    auto items = currentSceneBzItems();
    int  count = items.size();

    if (count == 0)
        return;

    auto pItem = items.first();

    view->pageScene()->clearSelections();

    view->pageScene()->selectPageItem(pItem);

    for (int i = 0; i < addTimes; ++i) {
        DTestEventList e;
        e.addKeyPress(Qt::Key_C, Qt::ControlModifier, 100);
        e.addKeyPress(Qt::Key_V, Qt::ControlModifier, 100);
        e.simulate(view->viewport());
    }

    //int newCount = view->pageScene()->allPageItems().size();

    //ASSERT_EQ(newCount, count + addTimes);
}

void layerChange()
{
    DTestEventList e;
    e.clear();
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);

    PageItem *pItem = dynamic_cast<PageItem *>(view->pageScene()->allPageItems().first());
    ASSERT_NE(pItem, nullptr);
    view->pageScene()->clearSelections();
    view->pageScene()->selectPageItem(pItem);
    e.addKeyPress(Qt::Key_BracketLeft, Qt::ControlModifier, 100);
    e.addKeyRelease(Qt::Key_BracketLeft, Qt::ControlModifier, 100);

    e.addKeyPress(Qt::Key_BracketRight, Qt::ControlModifier, 100);
    e.addKeyRelease(Qt::Key_BracketRight, Qt::ControlModifier, 100);

    e.addKeyPress(Qt::Key_BracketLeft, Qt::ControlModifier | Qt::ShiftModifier, 100);
    e.addKeyRelease(Qt::Key_BracketLeft, Qt::ControlModifier | Qt::ShiftModifier, 100);

    e.addKeyPress(Qt::Key_BracketRight, Qt::ControlModifier | Qt::ShiftModifier, 100);
    e.addKeyRelease(Qt::Key_BracketRight, Qt::ControlModifier | Qt::ShiftModifier, 100);

    e.simulate(view->viewport());
}

void groupUngroup()
{
    DTestEventList e;
    e.clear();
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);

    PageItem *pItem = dynamic_cast<PageItem *>(view->pageScene()->allPageItems().first());
    ASSERT_NE(pItem, nullptr);
    e.clear();
    view->pageScene()->selectAll();
    e.addKeyPress(Qt::Key_G, Qt::ControlModifier, 100);
    e.addKeyRelease(Qt::Key_G, Qt::ControlModifier, 100);
    e.simulate(view->viewport());

    e.clear();
    pItem = dynamic_cast<PageItem *>(view->pageScene()->allPageItems().first());
    ASSERT_NE(pItem, nullptr);
    view->pageScene()->clearSelections();
    view->pageScene()->selectPageItem(pItem);
    e.addKeyPress(Qt::Key_G, Qt::ControlModifier | Qt::ShiftModifier, 100);
    e.addKeyRelease(Qt::Key_G, Qt::ControlModifier | Qt::ShiftModifier, 100);
    e.simulate(view->viewport());
}

void selectAllItem()
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);

    DTestEventList e;
    e.addMouseMove(QPoint(20, 20), 100);
    e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(10, 10), 100);
    e.addMouseMove(QPoint(1800, 900), 100);
    e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(1000, 1000), 100);
    e.addKeyPress(Qt::Key_A, Qt::ControlModifier, 100);
    e.addKeyRelease(Qt::Key_A, Qt::ControlModifier, 100);
    e.simulate(view->viewport());

}

void itemAlignment()
{
    PageView *view = getCurView();

    ASSERT_NE(view, nullptr);
    DTestEventList e;
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

}

void itemTextRightClick()
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    qDebug() << "cuo1";
    PageItem *pItem = dynamic_cast<PageItem *>(view->pageScene()->allPageItems().first());
    ASSERT_NE(pItem, nullptr);
    qDebug() << "cuo2";

    view->pageScene()->clearSelections();
    view->pageScene()->selectPageItem(pItem);

    DTestEventList e;

    QPoint posDClick = view->mapFromScene(pItem->mapToScene(pItem->boundingRect().center()));
    e.addMouseMove(posDClick, 100);
    e.addDelay(100);
    e.simulate(view->viewport());

    static_cast<TextItem *>(pItem)->setEditing(true);

    QContextMenuEvent event(QContextMenuEvent::Mouse, QPoint(100, 100));
    dApp->sendEvent(view->viewport(), &event);
    e.clear();
    e.addDelay(100);
    e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(28, 270), 100);
    e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(28, 270), 100);
    e.addDelay(100);
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
    e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(28, 130), 100);
    e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(28, 130), 100);
    e.addDelay(100);
    e.simulate(QApplication::activePopupWidget());

    dApp->sendEvent(view->viewport(), &event);
    e.clear();
    e.addDelay(100);
    e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(28, 200), 100);
    e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(28, 200), 100);
    e.addDelay(100);
    e.simulate(QApplication::activePopupWidget());

    dApp->sendEvent(view->viewport(), &event);
    e.clear();
    e.addDelay(100);
    e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(28, 170), 100);
    e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(28, 170), 100);
    e.addDelay(100);
    e.simulate(QApplication::activePopupWidget());

}

void itemRightClick()
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    PageItem *pItem = dynamic_cast<PageItem *>(view->pageScene()->allPageItems().first());
    ASSERT_NE(pItem, nullptr);

    view->pageScene()->clearSelections();
    view->pageScene()->selectPageItem(pItem);

//    DTestEventList e;

//    QPoint pos = view->mapFromScene(pItem->mapToScene(pItem->boundingRect().center()));
//    e.addMouseMove(pos, 100);
//    e.addDelay(100);
//    e.simulate(view->viewport());



    //1.弹出菜单
    QContextMenuEvent event(QContextMenuEvent::Mouse, QPoint(100, 100));
    dApp->sendEvent(view->viewport(), &event);
    QTest::qWait(200);
    QMenu *pMenu = qobject_cast<QMenu *>(dApp->activePopupWidget());
    ASSERT_NE(pMenu, nullptr);
    //声明查找action的函数
    auto fFindAction = [ = ](const QString & name) {
        QAction *reslut = nullptr;
        for (auto pAction : pMenu->actions()) {
            if (pAction->text() == name)
                return pAction;
        }
        return reslut;
    };

    int bzItemsCount = view->pageScene()->allPageItems().count();
    int rootItemsCount = view->pageScene()->allRootPageItems().count();

    if (bzItemsCount == 0) {
        qWarning() << "QContextMenuEvent action not test because current view bzitems is empty.";
        return;
    }

    //2.全选
    auto pActionSelectAll = fFindAction(QObject::tr("Select All"));
    ASSERT_NE(pActionSelectAll, nullptr);
    emit pActionSelectAll->triggered();
    QTest::qWait(200);
    EXPECT_EQ(rootItemsCount, view->pageScene()->selectedItemCount());

    //3.剪切
    auto pActionCut = fFindAction(QObject::tr("Cut"));
    ASSERT_NE(pActionCut, nullptr);
    emit pActionCut->triggered();
    QTest::qWait(200);
    EXPECT_EQ(view->pageScene()->allPageItems().count(), 0);

    //4.撤销
    auto pActionUndo = fFindAction(QObject::tr("Undo"));
    ASSERT_NE(pActionUndo, nullptr);
    emit pActionUndo->triggered();
    QTest::qWait(200);
    EXPECT_EQ(view->pageScene()->allPageItems().count(), bzItemsCount);
    view->pageScene()->selectAll();

    //5.复制粘贴
    auto pActionCopy = fFindAction(QObject::tr("Copy"));
    ASSERT_NE(pActionCopy, nullptr);
    emit pActionCopy->triggered();
    QTest::qWait(200);
    EXPECT_EQ(view->pageScene()->allPageItems().count(), bzItemsCount);
    auto pActionPaste = fFindAction(QObject::tr("Paste"));
    ASSERT_NE(pActionPaste, nullptr);
    emit pActionPaste->triggered();
    QTest::qWait(200);
    EXPECT_EQ(view->pageScene()->allPageItems().count(), 2 * bzItemsCount);

    //6.撤销
    emit pActionUndo->triggered();
    QTest::qWait(200);
    EXPECT_EQ(view->pageScene()->allPageItems().count(), bzItemsCount);
    view->pageScene()->selectAll();

    //7.删除
    auto pActionDelete = fFindAction(QObject::tr("Delete"));
    ASSERT_NE(pActionDelete, nullptr);
    emit pActionDelete->triggered();
    QTest::qWait(200);
    EXPECT_EQ(view->pageScene()->allPageItems().count(), 0);

    //6.撤销
    emit pActionUndo->triggered();
    QTest::qWait(200);
    EXPECT_EQ(view->pageScene()->allPageItems().count(), bzItemsCount);
    view->pageScene()->selectAll();

    //7.组合
    auto pActionGroup = fFindAction(QObject::tr("Group"));
    ASSERT_NE(pActionGroup, nullptr);
    emit pActionGroup->triggered();
    QTest::qWait(200);
    EXPECT_EQ(view->pageScene()->allPageItems().count(), bzItemsCount);
    EXPECT_EQ(view->pageScene()->allRootPageItems().count(), 1);

    //8.撤销
    emit pActionUndo->triggered();
    QTest::qWait(200);
    EXPECT_EQ(view->pageScene()->allRootPageItems().count(), rootItemsCount);
    view->pageScene()->selectAll();

    //9.还原
    auto pActionRedo = fFindAction("Redo");
    emit pActionRedo->triggered();
    QTest::qWait(200);
    EXPECT_EQ(view->pageScene()->allRootPageItems().count(), 1);
    view->pageScene()->selectAll();


    //7.取消组合
    auto pActionUnGroup = fFindAction(QObject::tr("Ungroup"));
    ASSERT_NE(pActionUnGroup, nullptr);
    emit pActionUnGroup->triggered();
    QTest::qWait(200);
    EXPECT_EQ(view->pageScene()->allPageItems().count(), bzItemsCount);
    EXPECT_EQ(view->pageScene()->allRootPageItems().count(), rootItemsCount);

    pMenu->hide();


//    QContextMenuEvent event(QContextMenuEvent::Mouse, QPoint(100, 100));
//    dApp->sendEvent(view->viewport(), &event);
//    //QTest::qWait(200);
//    e.clear();
//    e.addDelay(200);
//    e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(28, 120), 100);
//    e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(28, 120), 100);
//    e.addDelay(100);
//    e.simulate(QApplication::activePopupWidget());

//    dApp->sendEvent(view->viewport(), &event);
//    e.clear();
//    e.addDelay(2000);
//    e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(28, 230), 100);
//    e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(28, 230), 100);
//    e.addDelay(100);
//    e.simulate(QApplication::activePopupWidget());

//    dApp->sendEvent(view->viewport(), &event);
//    e.clear();
//    e.addDelay(2000);
//    e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(28, 260), 100);
//    e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(28, 260), 100);
//    e.addDelay(100);
//    e.simulate(QApplication::activePopupWidget());

//    dApp->sendEvent(view->viewport(), &event);
//    e.clear();
//    e.addDelay(2000);
//    e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(28, 120), 100);
//    e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(28, 120), 100);
//    e.addDelay(100);
//    e.simulate(QApplication::activePopupWidget());

//    dApp->sendEvent(view->viewport(), &event);
//    e.clear();
//    e.addDelay(2000);
//    e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(28, 140), 100);
//    e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(28, 140), 100);
//    e.addDelay(100);
//    e.simulate(QApplication::activePopupWidget());

//    dApp->sendEvent(view->viewport(), &event);
//    e.clear();
//    e.addDelay(2000);
//    e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(28, 170), 100);
//    e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(28, 170), 100);
//    e.addDelay(100);
//    e.simulate(QApplication::activePopupWidget());

//    dApp->sendEvent(view->viewport(), &event);
//    e.clear();
//    e.addDelay(2000);
//    e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(28, 200), 100);
//    e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(28, 200), 100);
//    e.addDelay(100);
//    e.simulate(QApplication::activePopupWidget());

//    dApp->sendEvent(view->viewport(), &event);
//    e.clear();
//    e.addDelay(2000);
//    e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(28, 170), 100);
//    e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(28, 170), 100);
//    e.addDelay(100);
//    e.simulate(QApplication::activePopupWidget());

//    dApp->sendEvent(view->viewport(), &event);
//    e.clear();
//    e.addDelay(2000);
//    e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(28, 45), 1000);
//    e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(28, 45), 1000);
//    e.addDelay(1000);
//    e.simulate(QApplication::activePopupWidget());

//    dApp->sendEvent(view->viewport(), &event);
//    e.clear();
//    e.addDelay(2000);
//    e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(28, 10), 1000);
//    e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(28, 10), 1000);
//    e.addDelay(1000);
//    e.simulate(QApplication::activePopupWidget());

//    dApp->sendEvent(view->viewport(), &event);
//    e.clear();
//    e.addDelay(2000);
//    e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(28, 80), 100);
//    e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(28, 80), 100);
//    e.addDelay(1000);
//    e.simulate(QApplication::activePopupWidget());
}

QList<PageItem *> currentSceneBzItems(ESortItemTp sortTp)
{
    PageView *view = getCurView();
    if (view == nullptr)
        return QList<PageItem *>();

    return view->pageScene()->allPageItems(sortTp);
}

int currentSceneBzCount()
{
    return currentSceneBzItems().count();
}

extern int  quitResult;
extern QList<int> quitResults;
void setQuitDialogResult(int ret)
{
    quitResult = ret;
}
void setQuitListResults(const QList<int> &results)
{
    quitResults = results;
}
bool getToolButtonStatus(EDrawToolMode toolId)
{
    return getMainWindow()->drawBoard()->toolManager()->tool(toolId)->toolButton()->isEnabled();
}
