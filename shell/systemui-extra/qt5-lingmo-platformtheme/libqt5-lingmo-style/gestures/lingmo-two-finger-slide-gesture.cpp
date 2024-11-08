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

#include "lingmo-two-finger-slide-gesture.h"

#include <QWidget>

#include <QTouchEvent>
#include <QtMath>

#include <QDebug>

using namespace LINGMO;

TwoFingerSlideGesture::TwoFingerSlideGesture(QObject *parent) : QGesture(parent)
{

}

int TwoFingerSlideGesture::delta()
{
    switch (m_direction) {
    case Horizal:
        return m_current_pos.x() - m_last_pos.x();
    case Vertical:
        return m_current_pos.y() - m_last_pos.y();
    case Invalid:
        return 0;
    }
    return 0;
}

int TwoFingerSlideGesture::totalDelta()
{
    switch (m_direction) {
    case Horizal:
        return m_current_pos.x() - m_start_pos.x();
    case Vertical:
        return m_current_pos.y() - m_start_pos.y();
    case Invalid:
        return 0;
    }
    return 0;
}

TwoFingerSlideGestureRecognizer::TwoFingerSlideGestureRecognizer() : QGestureRecognizer()
{

}

QGesture *TwoFingerSlideGestureRecognizer::create(QObject *target)
{
    //qDebug()<<"create";
    if (target && target->isWidgetType())
        static_cast<QWidget *>(target)->setAttribute(Qt::WA_AcceptTouchEvents);
    return new TwoFingerSlideGesture;
}

QGestureRecognizer::Result TwoFingerSlideGestureRecognizer::recognize(QGesture *gesture, QObject *watched, QEvent *event)
{
    if (auto touchEvent = static_cast<QTouchEvent *>(event)) {
        auto slideGesture = static_cast<TwoFingerSlideGesture *>(gesture);

        switch (event->type()) {
        case QEvent::TouchBegin: {
            slideGesture->m_start_pos = touchEvent->touchPoints().first().pos().toPoint();
            slideGesture->m_current_pos = touchEvent->touchPoints().first().pos().toPoint();
            slideGesture->m_last_pos = touchEvent->touchPoints().first().pos().toPoint();
            gesture->setHotSpot(touchEvent->touchPoints().first().screenPos());
            return QGestureRecognizer::MayBeGesture;
            break;
        }
        case QEvent::TouchUpdate: {
            // only support 2 fingers
            if (touchEvent->touchPoints().count() != 2)
                return QGestureRecognizer::Ignore;

            if (touchEvent->touchPointStates() & Qt::TouchPointPressed) {
                if (touchEvent->touchPoints().count() == 2) {
                    // update start point center
                    slideGesture->m_start_pos = (touchEvent->touchPoints().first().pos().toPoint() + touchEvent->touchPoints().last().pos().toPoint())/2;
                    //qDebug()<<"update start point"<<slideGesture->startPos();
                    return QGestureRecognizer::MayBeGesture;
                }
            }

            if (touchEvent->touchPointStates() & Qt::TouchPointMoved) {
                // initial slide gesture direction, note that
                // once direction ensured, it will not be changed
                // untill the gesture finished or cancelled.
                if (slideGesture->direction() == TwoFingerSlideGesture::Invalid) {
                    qreal distance = (touchEvent->touchPoints().first().pos().toPoint() - touchEvent->touchPoints().last().pos().toPoint()).manhattanLength();
                    // we should distinguish slide and pinch zoom gesture.
                    if (distance > 200)
                        return QGestureRecognizer::Ignore;

                    QPoint offset = (touchEvent->touchPoints().first().pos().toPoint() + touchEvent->touchPoints().last().pos().toPoint())/2 - slideGesture->startPos();
                    //qDebug()<<offset;
                    if (qAbs(offset.y()) > 50) {
                        slideGesture->m_direction = TwoFingerSlideGesture::Vertical;
                        slideGesture->m_current_pos = touchEvent->touchPoints().first().pos().toPoint();
                        return QGestureRecognizer::TriggerGesture;
                    } else if (qAbs(offset.x()) > 50) {
                        slideGesture->m_direction = TwoFingerSlideGesture::Horizal;
                        slideGesture->m_current_pos = touchEvent->touchPoints().first().pos().toPoint();
                        return QGestureRecognizer::TriggerGesture;
                    } else {
                        // if offset set not enough, ignore.
                        return QGestureRecognizer::Ignore;
                    }
                } else {
                    // update gesture
                    slideGesture->m_last_pos = slideGesture->m_current_pos;
                    slideGesture->m_current_pos = touchEvent->touchPoints().first().pos().toPoint();
                    return QGestureRecognizer::TriggerGesture;
                }
            }

            break;
        }
        case QEvent::TouchEnd: {
            reset(slideGesture);
            return QGestureRecognizer::FinishGesture;
        }
        case QEvent::TouchCancel: {
            reset(slideGesture);
            return QGestureRecognizer::CancelGesture;
        }
        default:
            break;
        }
    }

    return QGestureRecognizer::Ignore;
}

void TwoFingerSlideGestureRecognizer::reset(QGesture *gesture)
{
    //qDebug()<<"reset";
    auto slideGesture = static_cast<TwoFingerSlideGesture *>(gesture);
    slideGesture->m_current_pos = QPoint();
    slideGesture->m_last_pos = QPoint();
    slideGesture->m_start_pos = QPoint();
    slideGesture->m_direction = TwoFingerSlideGesture::Invalid;
    QGestureRecognizer::reset(gesture);
}
