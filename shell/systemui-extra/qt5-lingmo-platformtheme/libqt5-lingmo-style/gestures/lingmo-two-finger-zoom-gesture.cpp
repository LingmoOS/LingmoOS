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

#include "lingmo-two-finger-zoom-gesture.h"

#include <QTouchEvent>
#include <QtMath>
#include <QWidget>

#include <QDebug>

using namespace LINGMO;

TwoFingerZoomGesture::TwoFingerZoomGesture(QObject *parent) : QGesture(parent)
{

}

LINGMO::TwoFingerZoomGestureRecognizer::TwoFingerZoomGestureRecognizer() : QGestureRecognizer()
{

}

QGesture *TwoFingerZoomGestureRecognizer::create(QObject *target)
{
    if (target && target->isWidgetType())
        static_cast<QWidget *>(target)->setAttribute(Qt::WA_AcceptTouchEvents);
    return new TwoFingerZoomGesture;
}

QGestureRecognizer::Result TwoFingerZoomGestureRecognizer::recognize(QGesture *gesture, QObject *watched, QEvent *event)
{
    if (auto touchEvent = static_cast<QTouchEvent *>(event)) {
        auto zoomGesture = static_cast<TwoFingerZoomGesture *>(gesture);

        switch (touchEvent->type()) {
        case QEvent::TouchBegin:
            gesture->setHotSpot(touchEvent->touchPoints().first().screenPos());
            return QGestureRecognizer::MayBeGesture;
            break;
        case QEvent::TouchUpdate: {
            if (touchEvent->touchPoints().count() != 2)
                return QGestureRecognizer::Ignore;

            switch (zoomGesture->zoomDirection()) {
            case TwoFingerZoomGesture::Invalid: {
                zoomGesture->m_start_points.first = touchEvent->touchPoints().first().pos().toPoint();
                zoomGesture->m_start_points.second = touchEvent->touchPoints().last().pos().toPoint();
                zoomGesture->m_last_points = zoomGesture->m_start_points;
                zoomGesture->m_current_points = zoomGesture->m_start_points;

                zoomGesture->m_start_points_distance = (zoomGesture->m_start_points.first - zoomGesture->m_start_points.second).manhattanLength();
                zoomGesture->m_last_points_distance = zoomGesture->m_start_points_distance;

                zoomGesture->m_zoom_direction = TwoFingerZoomGesture::Unkown;
                return QGestureRecognizer::TriggerGesture;
            }
            case TwoFingerZoomGesture::Unkown: {
                zoomGesture->m_last_points = zoomGesture->m_current_points;
                zoomGesture->m_current_points.first = touchEvent->touchPoints().first().pos().toPoint();
                zoomGesture->m_current_points.second = touchEvent->touchPoints().last().pos().toPoint();

                qreal currentPointsDistance = (zoomGesture->m_current_points.first - zoomGesture->m_current_points.second).manhattanLength();
                qreal totalDelta = currentPointsDistance - zoomGesture->m_start_points_distance;

                //qDebug()<<zoomGesture->m_start_points_distance<<currentPointsDistance<<totalDelta;

                // only total delta is enough the gesture would be triggered.
                // note that once zoom direction ensured, it won't change until
                // the gesture cancelled.
                if (qAbs(totalDelta) > 100) {
                    zoomGesture->m_last_points_distance = currentPointsDistance;
                    if (totalDelta > 0) {
                        zoomGesture->m_zoom_direction = TwoFingerZoomGesture::ZoomIn;
                    } else {
                        zoomGesture->m_zoom_direction = TwoFingerZoomGesture::ZoomOut;
                    }
                    return QGestureRecognizer::TriggerGesture;
                } else {
                    return QGestureRecognizer::MayBeGesture;
                }

                break;
            }
            case TwoFingerZoomGesture::ZoomIn: {
                // check if gesture should trigger or cancel
                auto tmp = zoomGesture->m_current_points;
                zoomGesture->m_current_points.first = touchEvent->touchPoints().first().pos().toPoint();
                zoomGesture->m_current_points.second = touchEvent->touchPoints().last().pos().toPoint();
                qreal currentPointsDistance = (zoomGesture->m_current_points.first - zoomGesture->m_current_points.second).manhattanLength();
                qreal distanceOffset = currentPointsDistance - zoomGesture->m_last_points_distance;
                if (distanceOffset > 0) {
                    // trigger zoom in
                    zoomGesture->m_last_points = tmp;
                    return QGestureRecognizer::TriggerGesture;
                } else {
                    if (qAbs(distanceOffset) < 100) {
                        return QGestureRecognizer::Ignore;
                    } else {
                        return QGestureRecognizer::CancelGesture;
                    }
                }
            }
            case TwoFingerZoomGesture::ZoomOut: {
                // check if gesture should trigger or cancel
                auto tmp = zoomGesture->m_current_points;
                zoomGesture->m_current_points.first = touchEvent->touchPoints().first().pos().toPoint();
                zoomGesture->m_current_points.second = touchEvent->touchPoints().last().pos().toPoint();
                qreal currentPointsDistance = (zoomGesture->m_current_points.first - zoomGesture->m_current_points.second).manhattanLength();
                qreal distanceOffset = currentPointsDistance - zoomGesture->m_last_points_distance;
                if (distanceOffset < 0) {
                    // trigger zoom out
                    zoomGesture->m_last_points = tmp;
                    return QGestureRecognizer::TriggerGesture;
                } else {
                    if (qAbs(distanceOffset) < 100) {
                        return QGestureRecognizer::Ignore;
                    } else {
                        return QGestureRecognizer::CancelGesture;
                    }
                }
            }
            }
            break;
        }

        case QEvent::TouchCancel:
            reset(gesture);
            return QGestureRecognizer::CancelGesture;

        case QEvent::TouchEnd:
            reset(gesture);
            return QGestureRecognizer::FinishGesture;

        default:
            break;
        }
    }

    return QGestureRecognizer::Ignore;
}

void TwoFingerZoomGestureRecognizer::reset(QGesture *gesture)
{
    auto zoomGesture = static_cast<TwoFingerZoomGesture *>(gesture);
    zoomGesture->m_start_points.first = QPoint();
    zoomGesture->m_start_points.second = QPoint();
    zoomGesture->m_current_points.first = QPoint();
    zoomGesture->m_current_points.second = QPoint();
    zoomGesture->m_zoom_direction = TwoFingerZoomGesture::Invalid;
    QGestureRecognizer::reset(gesture);
}
