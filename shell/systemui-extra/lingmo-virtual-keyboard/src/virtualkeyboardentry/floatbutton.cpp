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

#include "virtualkeyboardentry/floatbutton.h"

#include <QBitmap>
#include <QPainter>
#include <QVariant>

FloatButton::FloatButton(MouseClickedCallback mouseClickedCallback)
    : mouseClickedCallback_(std::move(mouseClickedCallback)) {
    // 主题框架默认禁用了move消息，因此，QPushButton需要禁用主题框架
    setProperty("useStyleWindowManager", QVariant(false));
    initStyle();
}

void FloatButton::move(int x, int y) { QPushButton::move(x, y); }

void FloatButton::resize(int width, int height) {
    setFixedSize(width, height);
    setIconSize(size());
    QPushButton::resize(width, height);
}

void FloatButton::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        startX_ = event->pos().x();
        startY_ = event->pos().y();

        startClickTimer();
    }

    QPushButton::mousePressEvent(event);
}

bool FloatButton::shouldPerformMouseClick() const {
    return !isFloatButtonMoved() && clickTimer_->isActive();
}

void FloatButton::processMouseReleaseEvent() {
    emit mouseReleased();

    manhattonLength = 0;

    startX_ = -1;
    startY_ = -1;
}

void FloatButton::processMouseClickEvent() {
    if (!mouseClickedCallback_) {
        return;
    }
    mouseClickedCallback_();
}

void FloatButton::mouseReleaseEvent(QMouseEvent *event) {
    QPushButton::mouseReleaseEvent(event);

    if (event->button() != Qt::LeftButton) {
        return;
    }

    bool couldPerformMouseClick = shouldPerformMouseClick();
    stopClickTimer();

    if (couldPerformMouseClick) {
        processMouseClickEvent();

        return;
    }

    if (isFloatButtonMoved()) {
        processMouseReleaseEvent();
    }
}

void FloatButton::updateManhattonLength(QMouseEvent *event) {
    int offsetX = event->pos().x() - startX_;
    int offsetY = event->pos().y() - startY_;

    manhattonLength += std::abs(offsetX) + std::abs(offsetY);
}

bool FloatButton::isFloatButtonMoved() const {
    return manhattonLength > manhattonLengthThreshold;
}

void FloatButton::processMouseMoveEvent(QMouseEvent *event) {
    if (!isFloatButtonMoved()) {
        updateManhattonLength(event);
    } else {
        emit mouseMoved(event->pos().x() - startX_, event->pos().y() - startY_);
    }
}

void FloatButton::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        processMouseMoveEvent(event);
    }
    QPushButton::mouseMoveEvent(event);
}

void FloatButton::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QBrush(Qt::white));
    painter.setPen(Qt::transparent);

    QRect rect = this->rect();
    rect.setWidth(rect.width());
    rect.setHeight(rect.height());

    painter.drawEllipse(rect);

    QPushButton::paintEvent(event);
}

void FloatButton::initStyle() {
    setWindowTitle("lingmo-virtual-keyboard-float-button");
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("QPushButton{border-image: "
                  "url(:/floatbutton/img/floatbuttondefault.svg);}"
                  "QPushButton:hover{border-image: "
                  "url(:/floatbutton/img/floatbuttonhovered.svg);}"
                  "QPushButton:pressed{border-image: "
                  "url(:/floatbutton/img/floatbuttonpressed.svg);}");
}

void FloatButton::startClickTimer() {
    clickTimer_.reset(new QTimer());
    clickTimer_->setSingleShot(true);
    clickTimer_->start(clickTimeThreshold_);
}

void FloatButton::stopClickTimer() {
    if (clickTimer_ == nullptr) {
        clickTimer_.reset();
    }
}
