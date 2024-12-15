// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "touchgestureoperation.h"

#include <QEvent>
#include <QGestureEvent>
#include <QtMath>
#include <QMouseEvent>

touchGestureOperation::touchGestureOperation(QWidget *parent)
    : m_parentWidget(parent)
{
    if (m_parentWidget) {
        m_parentWidget->setAttribute(Qt::WA_AcceptTouchEvents);
        //截获相应的gesture手势
        m_parentWidget->grabGesture(Qt::TapGesture);
        m_parentWidget->grabGesture(Qt::TapAndHoldGesture);
        m_parentWidget->grabGesture(Qt::PanGesture);
    }
}

bool touchGestureOperation::event(QEvent *e)
{
    bool _result {false};
    if (e->type() == QEvent::Gesture) {
        //手势触发
        _result = gestureEvent(dynamic_cast<QGestureEvent *>(e));
    }
    QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>(e);
    if (e->type() == QEvent::MouseButtonPress && mouseEvent->source() == Qt::MouseEventSynthesizedByQt) {
        //触摸点击转换鼠标点击事件
        m_mouseState = M_PRESS;
        m_beginTouchPoint = mouseEvent->pos();
        _result = true;
    }
    if (e->type() == QEvent::MouseMove && mouseEvent->source() == Qt::MouseEventSynthesizedByQt) {
        //触摸移动转换鼠标移动事件
        //如果移动距离大与5则为触摸移动状态
        QPointF currentPoint = mouseEvent->pos();
        if (QLineF(m_beginTouchPoint, currentPoint).length() > 5) {
            m_mouseState = M_MOVE;
            //如果为单指点击状态则转换为滑动状态
            switch (m_touchState) {
            case T_SINGLE_CLICK: {
                m_touchState = T_SLIDE;
                break;
            }
            case T_SLIDE: {
                //计算滑动方向和距离
                calculateAzimuthAngle(m_beginTouchPoint, currentPoint);
                //如果移动距离大于15则更新
                if (m_movelenght > 15) {
                    m_update = true;
                    m_beginTouchPoint = currentPoint;
                }
                break;
            }
            default:
                break;
            }
        }
        _result = true;
    }
    if (e->type() == QEvent::MouseButtonRelease && mouseEvent->source() == Qt::MouseEventSynthesizedByQt) {
        m_mouseState = M_NONE;
        _result = true;
    }
    return _result;
}

bool touchGestureOperation::isUpdate() const
{
    return m_update;
}

void touchGestureOperation::setUpdate(bool b)
{
    m_update = b;
}

touchGestureOperation::TouchState touchGestureOperation::getTouchState() const
{
    return m_touchState;
}

touchGestureOperation::TouchMovingDirection touchGestureOperation::getMovingDir() const
{
    return m_movingDir;
}

touchGestureOperation::TouchMovingDirection touchGestureOperation::getTouchMovingDir(QPointF &startPoint, QPointF &stopPoint, qreal &movingLine)
{
    TouchMovingDirection _result {T_MOVE_NONE};
    qreal angle = 0.0000;
    qreal dx = stopPoint.rx() - startPoint.rx();
    qreal dy = stopPoint.ry() - startPoint.ry();
    //计算方位角
    angle = qAtan2(dy, dx) * 180 / M_PI;
    qreal line = qSqrt(dx * dx + dy * dy);
    //如果移动距离大于10则有效
    if (line > 10) {
        if ((angle <= -45) && (angle >= -135)) {
            _result = TouchMovingDirection::T_TOP;
        } else if ((angle > -45) && (angle < 45)) {
            _result = TouchMovingDirection::T_RIGHT;
        } else if ((angle >= 45) && (angle <= 135)) {
            _result = TouchMovingDirection::T_BOTTOM;
        } else {
            _result = TouchMovingDirection::T_LEFT;
        }
    }
    movingLine = line;
    return _result;
}

bool touchGestureOperation::gestureEvent(QGestureEvent *event)
{
    if (QGesture *tap = event->gesture(Qt::TapGesture))
        tapGestureTriggered(dynamic_cast<QTapGesture *>(tap));
    if (QGesture *pan = event->gesture(Qt::PanGesture))
        panTriggered(dynamic_cast<QPanGesture *>(pan));
    return true;
}

void touchGestureOperation::tapGestureTriggered(QTapGesture *tap)
{
    switch (tap->state()) {
    case Qt::NoGesture: {
        break;
    }
    case Qt::GestureStarted: {
        m_beginTouchTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
        m_touchState = T_SINGLE_CLICK;
        break;
    }
    case Qt::GestureUpdated: {
        break;
    }
    case Qt::GestureFinished: {
        break;
    }
    default: {
        //GestureCanceled
    }
    }
}

void touchGestureOperation::panTriggered(QPanGesture *pan)
{
    switch (pan->state()) {
    case Qt::NoGesture: {
        break;
    }
    case Qt::GestureStarted: {
        break;
    }
    case Qt::GestureUpdated: {
        m_touchState = T_SLIDE;
        break;
    }
    case Qt::GestureFinished: {
        break;
    }
    default:
        //GestureCanceled
        break;
    }
}

void touchGestureOperation::calculateAzimuthAngle(QPointF &startPoint, QPointF &stopPoint)
{
    m_movingDir = getTouchMovingDir(startPoint, stopPoint, m_movelenght);
}
