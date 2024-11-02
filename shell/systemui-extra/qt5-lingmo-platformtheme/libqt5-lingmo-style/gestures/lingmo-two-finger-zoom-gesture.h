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

#ifndef TWOFINGERZOOMGESTURE_H
#define TWOFINGERZOOMGESTURE_H

#include <QGesture>
#include <QGestureRecognizer>

#include <QPair>

namespace LINGMO {

class TwoFingerZoomGesture : public QGesture
{
    friend class TwoFingerZoomGestureRecognizer;
    Q_OBJECT
public:
    enum Direction {
        Invalid,
        Unkown,
        ZoomIn,
        ZoomOut
    };

    explicit TwoFingerZoomGesture(QObject *parent = nullptr);
    ~TwoFingerZoomGesture(){}

    Direction zoomDirection() {return m_zoom_direction;}
    QPair<QPoint, QPoint> startPoints() {return m_start_points;}
    QPair<QPoint, QPoint> lastPoints() {return m_last_points;}
    QPair<QPoint, QPoint> currentPoints() {return m_current_points;}

private:
    QPair<QPoint, QPoint> m_start_points;
    QPair<QPoint, QPoint> m_last_points;
    QPair<QPoint, QPoint> m_current_points;

    qreal m_start_points_distance = -1;
    qreal m_last_points_distance = -1;

    Direction m_zoom_direction = Invalid;
};

class TwoFingerZoomGestureRecognizer : public QGestureRecognizer
{
public:
    explicit TwoFingerZoomGestureRecognizer();
    ~TwoFingerZoomGestureRecognizer(){}

    QGesture *create(QObject *target) override;
    QGestureRecognizer::Result recognize(QGesture *gesture, QObject *watched, QEvent *event) override;
    void reset(QGesture *gesture) override;
};

}

#endif // TWOFINGERZOOMGESTURE_H
