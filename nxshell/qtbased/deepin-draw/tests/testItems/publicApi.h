// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PUBLICAPI_H
#define PUBLICAPI_H

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#define protected public
#define private public
//#include "cgraphicsview.h"
#include <QtTest>
#include <QTestEventList>
#include <qaction.h>

#include "mainwindow.h"
#include "pageview.h"
#include "pageitem.h"
#include "pagescene.h"
#include "globaldefine.h"

#include <DComboBox>
#include <DLineEdit>
#include <DWidget>
#include <DSlider>

#include "calphacontrolwidget.h"
//#include <dzoommenucombobox.h>
#include "cspinbox.h"
//#include "ccolorpickwidget.h"
#include "ciconbutton.h"
//#include "cgraphicsitem.h"
//#include "pickcolorwidget.h"

#include <QBrush>
#undef protected
#undef private

//#define ON true
//#define OFF false
//#define TEST_DELETE_ITEM OFF
//#define TEST_BLUR_ITEM OFF
//#define TEST_START_ITEM OFF
//#define TEST_PICTURE_ITEM OFF
//#define TEST_RECT_ITEM ON
//#define TEST_TRIANGLE_ITEM OFF
//#define TEST_POLYGON_ITEM OFF
//#define TEST_LINE_ITEM OFF
//#define TEST_PEN_ITEM OFF
//#define TEST_TEXT_ITEM OFF
//#define TEST_CUT_ITEM OFF
//#define TEST_SCANLE_ITEM OFF
//#define TEST_ELLIPSE_ITEM OFF
//#define TEST_FUNCTION_ITEM OFF

DWIDGET_USE_NAMESPACE

MainWindow *getMainWindow();

PageView *getCurView();

class DMouseMoveEvent;

class DTestEventList: public QTestEventList
{
public:
    void addMousePress(Qt::MouseButton button, Qt::KeyboardModifiers stateKey = Qt::KeyboardModifiers(),
                       QPoint pos = QPoint(), int delay = -1);

    void addMouseRelease(Qt::MouseButton button, Qt::KeyboardModifiers stateKey = Qt::KeyboardModifiers(),
                         QPoint pos = QPoint(), int delay = -1);

    void addMouseMove(QPoint pos = QPoint(), int delay = -1, Qt::KeyboardModifiers stateKey = Qt::KeyboardModifiers());

    void simulate(QWidget *w);
private:
    Qt::MouseButton _pressedMouseButton = Qt::NoButton;
};


void createNewViewByShortcutKey();

void setPenWidth(PageItem *item, int width);

void setStrokeColor(PageItem *item, QColor color);

void setBrushColor(PageItem *item, QColor color);

void resizeItem();

void createItemByMouse(PageView *view,
                       bool altCopyItem = false, QPoint topLeft = QPoint(500, 300),
                       QPoint bottomRight = QPoint(600, 400), bool doUndoRedo = true,
                       Qt::KeyboardModifiers stateKey = Qt::NoModifier);

void createItemWithkeyShift(PageView *view, bool altCopyItem = true, QPoint startPos = QPoint(200, 100)
                                                                                       , QPoint endPos = QPoint(400, 300));

void keyShortCutCopyItem(int addTimes = 1);

QList<PageItem *> currentSceneBzItems(ESortItemTp sortTp = EDesSort);
int                    currentSceneBzCount();

void layerChange();


void groupUngroup();

void selectAllItem();


void itemAlignment();


void itemTextRightClick();


void itemRightClick();

template <class Functor>
static void qMyWaitFor(Functor predicate, int timeout = 5000)
{
    // We should not spin the event loop in case the predicate is already true,
    // otherwise we might send new events that invalidate the predicate.
    if (predicate()) {
        return /*true*/;
    }

    // qWait() is expected to spin the event loop, even when called with a small
    // timeout like 1ms, so we we can't use a simple while-loop here based on
    // the deadline timer not having timed out. Use do-while instead.

    int remaining = timeout;
    QDeadlineTimer deadline(remaining, Qt::PreciseTimer);

    do {
        //QCoreApplication::processEvents(QEventLoop::AllEvents, remaining);
        //QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QCoreApplication::processEvents(QEventLoop::AllEvents, qMin(10, remaining));
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

        remaining = deadline.remainingTime();

        if (predicate()) {
            return /*true*/;
        }

        remaining = deadline.remainingTime();
    } while (remaining > 0);

    return /*predicate()*/; // Last chance
}
template <class Functor>
static bool qMyWaitFor_b(Functor predicate, int timeout = 5000)
{
    // We should not spin the event loop in case the predicate is already true,
    // otherwise we might send new events that invalidate the predicate.
    if (predicate()) {
        return true;
    }

    // qWait() is expected to spin the event loop, even when called with a small
    // timeout like 1ms, so we we can't use a simple while-loop here based on
    // the deadline timer not having timed out. Use do-while instead.

    int remaining = timeout;
    QDeadlineTimer deadline(remaining, Qt::PreciseTimer);

    do {
        //QCoreApplication::processEvents(QEventLoop::AllEvents, remaining);
        //QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QCoreApplication::processEvents(QEventLoop::AllEvents, qMin(10, remaining));
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

        remaining = deadline.remainingTime();

        if (predicate()) {
            return true;
        }

        remaining = deadline.remainingTime();
    } while (remaining > 0);

    return predicate(); // Last chance
}

void setQuitDialogResult(int ret);
void setQuitListResults(const QList<int> &results);
bool getToolButtonStatus(EDrawToolMode toolId);

#endif // PUBLICAPI_H
