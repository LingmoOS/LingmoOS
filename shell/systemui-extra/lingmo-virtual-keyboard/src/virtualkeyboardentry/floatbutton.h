/*
 * Copyright 2022 KylinSoft Co., Ltd.
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

#ifndef FLOATBUTTON_H
#define FLOATBUTTON_H

#include <functional>
#include <memory>

#include <QMouseEvent>
#include <QPaintEvent>
#include <QPushButton>
#include <QSize>
#include <QTimer>

class FloatButton : public QPushButton {
    Q_OBJECT

public:
    using MouseClickedCallback = std::function<void()>;

public:
    explicit FloatButton(MouseClickedCallback mouseClickedCallback);
    ~FloatButton() override = default;

signals:
    void mouseMoved(int x, int y);
    void mouseReleased();

public slots:
    void move(int x, int y);
    void resize(int width, int height);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    bool shouldPerformMouseClick() const;
    void processMouseReleaseEvent();
    void processMouseClickEvent();
    void processMouseMoveEvent(QMouseEvent *event);

    void updateManhattonLength(QMouseEvent *event);
    bool isFloatButtonMoved() const;

    void initStyle();

    void startClickTimer();
    void stopClickTimer();

private:
    int startX_ = -1;
    int startY_ = -1;

    int manhattonLength = 0;
    constexpr static int manhattonLengthThreshold = 10;

    MouseClickedCallback mouseClickedCallback_;
    std::unique_ptr<QTimer> clickTimer_ = nullptr;
    const int clickTimeThreshold_ = 600;
};

#endif // FLOATBUTTON_H
