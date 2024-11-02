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

#include "geometrymanager.h"

#include "screenmanager.h"

Scaler::Scaler(ScaleFactorCallback widthScaleFactorCallback,
               ScaleFactorCallback heightScaleFactorCallback,
               ScaleFactorCallback contentScaleFactorCallback)
    : widthScaleFactorCallback_(std::move(widthScaleFactorCallback)),
      heightScaleFactorCallback_(std::move(heightScaleFactorCallback)),
      contentScaleFactorCallback_(std::move(contentScaleFactorCallback)) {}

float Scaler::getScaleFactor(ScaleFactorCallback callback) {
    if (callback == nullptr) {
        return 1.0f;
    }

    return callback();
}

float Scaler::getWidthScaleFactor() const {
    return getScaleFactor(widthScaleFactorCallback_);
}

float Scaler::getHeightScaleFactor() const {
    return getScaleFactor(heightScaleFactorCallback_);
}

float Scaler::getContentScaleFactor() const {
    return getScaleFactor(contentScaleFactorCallback_);
}

GeometryManager::GeometryManager(Scaler &&scaler)
    : QObject(), scaler_(std::move(scaler)) {}

QSize GeometryManager::calculateViewSize() const {
    return QSize(calculateScaledViewWidth(), calculateScaledViewHeight());
}

QRect GeometryManager::geometry() const {
    return QRect(calculateViewPosition(), calculateViewSize());
}

void GeometryManager::updateGeometry() {
    QPoint position = calculateViewPosition();
    emit viewMoved(position.x(), position.y());

    QSize size = calculateViewSize();
    emit viewResized(size.width(), size.height());
}

int GeometryManager::calculateScaledViewWidth() const {
    return calculateViewWidth() * scaler_.getWidthScaleFactor();
}

int GeometryManager::calculateScaledViewHeight() const {
    return calculateViewHeight() * scaler_.getHeightScaleFactor();
}

int GeometryManager::getViewContentWidth() const {
    return calculateViewWidth() * scaler_.getContentScaleFactor();
}

int GeometryManager::getViewContentHeight() const {
    return calculateViewHeight() * scaler_.getContentScaleFactor();
}
