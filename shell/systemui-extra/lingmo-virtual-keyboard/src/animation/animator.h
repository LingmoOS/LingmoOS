/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <memory>

#include <QObject>
#include <QWindow>

class Animator : public QObject {
    Q_OBJECT

public:
    Animator() = default;
    virtual ~Animator() override = default;

    virtual void playShowAnimation(QWindow *view, const QRect &endGeometry) = 0;
    virtual void playHideAnimation(QWindow *view, const QRect &endGeometry) = 0;
    virtual void playFlipAnimation(QWindow *view, const QRect &endGeometry) = 0;

signals:
    void animationFinished();
};

#endif // ANIMATOR_H
