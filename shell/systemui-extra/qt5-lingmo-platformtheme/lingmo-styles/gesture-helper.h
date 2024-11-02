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

#ifndef GESTUREHELPER_H
#define GESTUREHELPER_H

#include <QObject>
#include <QTimer>
#include <QPointF>
#include <QTouchEvent>

namespace LINGMO {
class TwoFingerSlideGestureRecognizer;
class TwoFingerZoomGestureRecognizer;
}

/*!
 * \brief The GestureHelper class
 * \note
 * this class only handle hold and tap gesture for now.
 */
class GestureHelper : public QObject
{
    Q_OBJECT
public:

    explicit GestureHelper(QObject *parent = nullptr);
    ~GestureHelper();

    void registerWidget(QWidget *widget);
    void unregisterWidget(QWidget *widget);

    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    bool m_is_touching = false;
    int m_finger_count = 0;

    QList<QTouchEvent::TouchPoint> m_touch_points;

    // hold and tap gesture
    bool m_menu_popped = false;
    QTimer m_menu_popped_timer;
    QPointF m_hold_and_tap_pos;
    bool m_is_native_mouse_move = false;

    // pan gesture
    bool m_is_paning = false;
    QTimer m_disable_paning_flag_delayer;

    // pinch gesture
    bool m_is_pinching = false;
    QTimer m_pinch_operation_delayer;

    LINGMO::TwoFingerSlideGestureRecognizer *m_slider;
    LINGMO::TwoFingerZoomGestureRecognizer *m_zoomer;

    Qt::GestureType m_slide_type;
    Qt::GestureType m_zoom_type;

    bool m_is_mouse_pressed = false;
};

#endif // GESTUREHELPER_H
