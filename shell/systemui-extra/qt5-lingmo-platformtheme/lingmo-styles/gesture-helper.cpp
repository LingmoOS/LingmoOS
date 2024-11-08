/*
 * Qt5-LINGMO's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "gesture-helper.h"
#include "gestures/lingmo-two-finger-slide-gesture.h"
#include "gestures/lingmo-two-finger-zoom-gesture.h"

#include <QWidget>
#include <QGestureEvent>
#include <QTapAndHoldGesture>

#include <QContextMenuEvent>

#include <QApplication>
#include <QMenu>

#include <QTouchEvent>

#include <QScroller>
#include <QMouseEvent>

#include <QAbstractItemView>
#include <QItemSelectionModel>

#include <QScroller>

#include <QWheelEvent>

#include <QtMath>

#include <QDebug>

GestureHelper::GestureHelper(QObject *parent) : QObject(parent)
{
    // we translate event by ourselves
    qApp->setAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents, false);
    //qApp->setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents, false);

    m_menu_popped_timer.setInterval(500);
    m_menu_popped_timer.setSingleShot(true);

    m_pinch_operation_delayer.setInterval(200);
    m_pinch_operation_delayer.setSingleShot(true);

    m_slider = new LINGMO::TwoFingerSlideGestureRecognizer;
    m_zoomer = new LINGMO::TwoFingerZoomGestureRecognizer;

    m_slide_type = QGestureRecognizer::registerRecognizer(m_slider);
    m_zoom_type = QGestureRecognizer::registerRecognizer(m_zoomer);

}

GestureHelper::~GestureHelper()
{
    QGestureRecognizer::unregisterRecognizer(m_slide_type);
    QGestureRecognizer::unregisterRecognizer(m_zoom_type);
}

void GestureHelper::registerWidget(QWidget *widget)
{
    if (!widget)
        return;

    if (widget->contextMenuPolicy() == Qt::NoContextMenu)
        return;

    //widget->setAttribute(Qt::WA_AcceptTouchEvents, false);

    widget->removeEventFilter(this);

    //widget->grabGesture(Qt::TapGesture);
    widget->grabGesture(Qt::TapAndHoldGesture);
    //widget->grabGesture(Qt::PanGesture);
    //widget->grabGesture(Qt::PinchGesture);
    //widget->grabGesture(Qt::SwipeGesture);
    //widget->grabGesture(m_slide_type);
    //widget->grabGesture(m_zoom_type);

    widget->installEventFilter(this);
}

void GestureHelper::unregisterWidget(QWidget *widget)
{
    if (!widget)
        return;

    if (widget->contextMenuPolicy() == Qt::NoContextMenu)
        return;

    //widget->setAttribute(Qt::WA_AcceptTouchEvents, true);

    widget->removeEventFilter(this);

    //widget->ungrabGesture(Qt::TapGesture);
    widget->ungrabGesture(Qt::TapAndHoldGesture);
    //widget->ungrabGesture(Qt::PanGesture);
    //widget->ungrabGesture(Qt::PinchGesture);
    //widget->ungrabGesture(Qt::SwipeGesture);
    //widget->ungrabGesture(m_slide_type);
    //widget->ungrabGesture(m_zoom_type);
}

bool GestureHelper::eventFilter(QObject *watched, QEvent *event)
{
    switch (event->type()) {
    case QEvent::ActivationChange: {
        // do not grab inactive window's gesture.
        auto widget = qobject_cast<QWidget *>(watched);
        if (widget->isActiveWindow()) {
            widget->grabGesture(Qt::TapAndHoldGesture);
        } else {
            widget->ungrabGesture(Qt::TapAndHoldGesture);
        }
        return false;
    }
    case QEvent::TouchBegin: {
        m_is_touching = true;
        auto te = static_cast<QTouchEvent *>(event);
        m_touch_points = te->touchPoints();
        m_finger_count = te->touchPoints().count();
        if (m_finger_count == 1)
            m_hold_and_tap_pos = te->touchPoints().first().pos();
        else
            m_hold_and_tap_pos = QPointF();

        if (m_touch_points.count() > 2) {
            m_menu_popped_timer.stop();
        }
        break;
    }

    case QEvent::TouchUpdate: {
        auto te = static_cast<QTouchEvent *>(event);
        m_touch_points = te->touchPoints();
        m_finger_count = te->touchPoints().count();
        break;
    }

    case QEvent::TouchCancel:
    case QEvent::TouchEnd: {
        m_touch_points.clear();
        m_is_touching = false;
        m_finger_count = 0;
        break;
    }

    case QEvent::Gesture: {
        auto e = static_cast<QGestureEvent *>(event);
        auto widget = qobject_cast<QWidget *>(watched);
        if (!widget->isActiveWindow())
            return false;

        if (auto twoFingerSlideGesture = static_cast<LINGMO::TwoFingerSlideGesture *>(e->gesture(m_slide_type))) {
            //qDebug()<<"has slide gesture";
            //qDebug()<<"slide"<<twoFingerSlideGesture->state()<<twoFingerSlideGesture->direction()<<twoFingerSlideGesture->totalDelta();
            if (twoFingerSlideGesture->state() == Qt::GestureUpdated) {
                // some view doesn't have scroller but also can scroll, such as process list view in lingmo-system-monitor.
                // we send the wheel event to trigger the scroll when slide gesture updated.
                if (!QScroller::hasScroller(widget)) {
                    bool isHorizal = twoFingerSlideGesture->direction() == LINGMO::TwoFingerSlideGesture::Horizal;
                    bool isVertical = twoFingerSlideGesture->direction() == LINGMO::TwoFingerSlideGesture::Vertical;
                    if (isVertical) {
                        QWheelEvent we(twoFingerSlideGesture->hotSpot(), twoFingerSlideGesture->delta() * 10, Qt::NoButton, Qt::NoModifier, Qt::Vertical);
                        qApp->sendEvent(widget, &we);
                    } else if (isHorizal) {
                        QWheelEvent we(twoFingerSlideGesture->hotSpot(), twoFingerSlideGesture->delta() * 10, Qt::NoButton, Qt::NoModifier, Qt::Horizontal);
                        qApp->sendEvent(widget, &we);
                    }
                }
            }
        }

        if (auto twoFingerZoomGesture = static_cast<LINGMO::TwoFingerZoomGesture *>(e->gesture(m_zoom_type))) {
            //qDebug()<<"has zoom gesture"<<twoFingerZoomGesture->state()<<twoFingerZoomGesture->zoomDirection()<<twoFingerZoomGesture->startPoints()<<twoFingerZoomGesture->lastPoints()<<twoFingerZoomGesture->currentPoints();
            switch (twoFingerZoomGesture->zoomDirection()) {
            case LINGMO::TwoFingerZoomGesture::ZoomIn: {
                QWheelEvent we(twoFingerZoomGesture->hotSpot(), 100, Qt::NoButton, Qt::ControlModifier);
                qApp->sendEvent(watched, &we);
                break;
            }
            case LINGMO::TwoFingerZoomGesture::ZoomOut: {
                QWheelEvent we(twoFingerZoomGesture->hotSpot(), -100, Qt::NoButton, Qt::ControlModifier);
                qApp->sendEvent(watched, &we);
                break;
            }
            default:
                break;
            }
        }

        if (auto tapGesture = static_cast<QTapGesture *>(e->gesture(Qt::TapGesture))) {
            //qDebug()<<tapGesture->gestureType()<<tapGesture->position()<<tapGesture->state();
            switch (tapGesture->state()) {
            case Qt::GestureStarted:
                //mouse press event
                break;
            case Qt::GestureCanceled:
            case Qt::GestureFinished: {
                //mouse release event
                break;
            }
            default:
                break;
            }
        }

        if (auto tapAndHoldGesture = static_cast<QTapAndHoldGesture*>(e->gesture(Qt::TapAndHoldGesture))) {
            //qDebug()<<tapAndHoldGesture->gestureType()<<tapAndHoldGesture->position()<<tapAndHoldGesture->state();
            if (m_menu_popped_timer.isActive())
                return false;

            m_hold_and_tap_pos = QPoint();

            // check if there is a menu popped.
            // note that the right click in a widget which has context menu
            // will trigger the tap and hold gesture. I have no idea to deal
            // with this case, because i didn't find how to recognize if this
            // event is translated from mouse event.
            if (qobject_cast<QMenu *>(watched) || qobject_cast<QMenu *>(qApp->activePopupWidget())) {
                qDebug()<<"menu popped, may be right click";
                m_menu_popped = true;
            }

            switch (tapAndHoldGesture->state()) {
            case Qt::GestureStarted: {
                if (m_menu_popped || m_is_mouse_pressed) {
                    return false;
                } else {
                    m_menu_popped = true;
                    m_menu_popped_timer.start();
                    auto pos = widget->mapFromGlobal(tapAndHoldGesture->position().toPoint());
                    auto gpos = tapAndHoldGesture->position().toPoint();

                    if (m_is_native_mouse_move)
                        return false;

                    QMouseEvent me(QMouseEvent::MouseButtonRelease, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
                    qApp->sendEvent(widget, &me);
                    QContextMenuEvent ce(QContextMenuEvent::Other, pos, gpos, Qt::NoModifier);
                    qApp->sendEvent(widget, &ce);
                }
                break;
            }
            case Qt::GestureCanceled:
            case Qt::GestureFinished: {
                m_menu_popped = false;
                break;
            }
            default:
                break;
            }
        }

        if (auto swipeGesture = static_cast<QSwipeGesture *>(e->gesture(Qt::SwipeGesture))) {
            //qDebug()<<swipeGesture->gestureType()<<swipeGesture->state()<<swipeGesture->horizontalDirection()<<swipeGesture->verticalDirection()<<swipeGesture->swipeAngle();
        }

        if (auto panGesture = static_cast<QPanGesture*>(e->gesture(Qt::PanGesture))) {
            //qDebug()<<panGesture->gestureType()<<panGesture->state()<<panGesture->delta()<<panGesture->hotSpot();
            switch (panGesture->state()) {
            case Qt::GestureStarted: {
                auto widget = qobject_cast<QWidget *>(watched);
                auto pos = widget->mapFromGlobal(QCursor::pos());
                QMouseEvent me(QMouseEvent::MouseButtonRelease, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
                qApp->sendEvent(widget, &me);
                if (m_is_paning) {
                    return false;
                } else {
                    m_is_paning = true;
                }
                break;
            }
            case Qt::GestureCanceled:
            case Qt::GestureFinished: {
                m_is_paning = false;
                break;
            }
            default:
                break;
            }
            return false;
        }

        break;
    }

    case QEvent::MouseMove: {
        if (m_finger_count > 1)
            return true;

        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        auto widget = static_cast<QWidget *>(watched);
        if (!widget)
            return false;

        //qDebug()<<me->type()<<me->pos();

        bool isTranslatedFromTouch = me->source() == Qt::MouseEventSynthesizedByQt;
        if (!isTranslatedFromTouch) {
            // do not trigger tap and hold gesture handler
            m_is_native_mouse_move = true;
            return false;
        }
        m_is_native_mouse_move = false;

        // if we do a hold and tap gesture, we should not trigger mouse move event.
        // so we have to ignore the small offset of finger move.
        auto lastTapPoint = m_hold_and_tap_pos.toPoint();
        auto currentPoint = widget->mapTo(widget->topLevelWidget(), me->pos());

        if (!lastTapPoint.isNull()) {
            //qDebug()<<lastTapPoint<<currentPoint<<lenthSquared;
            if (QRect(-50, -50, 100, 100).contains(lastTapPoint - currentPoint)) {
                if (qobject_cast<QAbstractScrollArea *>(widget) || qobject_cast<QAbstractScrollArea *>(widget->parent()))
                    return true;
            }
        }

        if (m_is_paning) {
            return true;
        }

        return false;
    }

    case QEvent::DragEnter:
    case QEvent::DragMove: {
        if (m_is_paning)
            return true;
    }

    case QEvent::MouseButtonPress: {
        auto me = static_cast<QMouseEvent *>(event);
        if (me->source() == Qt::MouseEventNotSynthesized)
            m_is_mouse_pressed = true;
        else
            m_is_mouse_pressed = false;
        break;
    }
    case QEvent::MouseButtonRelease: {
        auto me = static_cast<QMouseEvent *>(event);
        if (me->source() == Qt::MouseEventNotSynthesized)
            m_is_mouse_pressed = false;
        break;
    }

    default:
        break;
    }

    return false;
}
