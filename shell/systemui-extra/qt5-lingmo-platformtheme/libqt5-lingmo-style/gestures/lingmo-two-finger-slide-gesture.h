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

#ifndef TWOFINGERSLIDEGESTURE_H
#define TWOFINGERSLIDEGESTURE_H

#include <QGesture>
#include <QGestureRecognizer>

namespace LINGMO {

class TwoFingerSlideGesture : public QGesture
{
    friend class TwoFingerSlideGestureRecognizer;
    Q_OBJECT
public:
    enum Direction {
        Invalid,
        Horizal,
        Vertical
    };

    explicit TwoFingerSlideGesture(QObject *parent = nullptr);
    ~TwoFingerSlideGesture(){}

    Direction direction() {return m_direction;}
    const QPoint startPos() {return m_start_pos;}
    const QPoint currentPos() {return m_current_pos;}
    int delta();
    int totalDelta();

private:
    QPoint m_start_pos;
    QPoint m_last_pos;
    QPoint m_current_pos;

    Direction m_direction = Invalid;
};

class TwoFingerSlideGestureRecognizer : public QGestureRecognizer
{
public:
    explicit TwoFingerSlideGestureRecognizer();

    QGesture *create(QObject *target) override;
    QGestureRecognizer::Result recognize(QGesture *gesture, QObject *watched, QEvent *event) override;
    void reset(QGesture *gesture) override;
};

}

#endif // TWOFINGERSLIDEGESTURE_H
