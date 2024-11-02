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

#include "floatgeometrymanager.h"

#include <QMap>

#include "localsettings/localsettings.h"
#include "screenmanager.h"

// static
const QString FloatGeometryManager::floatGeometryGroup = "floatGeometry";
// static
const QString FloatGeometryManager::marginRatioMapKey = "marginRatioMap";
// static
const QString FloatGeometryManager::leftMarginRatioKey = "leftMarginRatio";
// static
const QString FloatGeometryManager::topMarginRatioKey = "topMarginRatio";

FloatGeometryManager::FloatGeometryManager(std::unique_ptr<Strategy> strategy,
                                           LocalSettings &viewSettings)
    : FloatGeometryManager(std::move(strategy), viewSettings, Scaler()) {}

FloatGeometryManager::FloatGeometryManager(std::unique_ptr<Strategy> strategy,
                                           LocalSettings &viewSettings,
                                           Scaler &&scaler)
    : GeometryManager(std::move(scaler)), strategy_(std::move(strategy)),
      viewSettings_(viewSettings) {
    loadMarginRatioMap();
}

FloatGeometryManager::~FloatGeometryManager() { saveMarginRatioMap(); }

void FloatGeometryManager::moveBy(int offsetX, int offsetY) {
    const QPoint offset(offsetX, offsetY);

    moveView(QPoint(calculateCurrentPosition() + offset));
}

void FloatGeometryManager::endDrag() {
    const QPoint currentPosition = calculateCurrentPosition();
    const QPoint normalizedPosition =
        calculateNormalizedPosition(currentPosition);
    if (normalizedPosition != currentPosition) {
        moveView(normalizedPosition);
    }
}

int FloatGeometryManager::calculateViewWidth() const {
    return strategy_->getViewWidth();
}

int FloatGeometryManager::calculateViewHeight() const {
    return strategy_->getViewHeight();
}

int FloatGeometryManager::calculateNormalizedX(int positionX) const {
    const auto geometry = ScreenManager::getPrimaryScreenGeometry();

    if (positionX < geometry.left()) {
        return geometry.left();
    }

    const auto viewWidth = calculateScaledViewWidth();
    if (positionX + viewWidth > geometry.right()) {
        return geometry.right() - viewWidth;
    }

    return positionX;
}

int FloatGeometryManager::calculateNormalizedY(int positionY) const {
    const auto geometry = ScreenManager::getPrimaryScreenGeometry();

    if (positionY < geometry.top()) {
        return geometry.top();
    }

    const auto viewHeight = calculateScaledViewHeight();
    if (positionY + viewHeight > geometry.bottom()) {
        return geometry.bottom() - viewHeight;
    }

    return positionY;
}

QPoint FloatGeometryManager::calculateNormalizedPosition(
    const QPoint &position) const {
    return QPoint(calculateNormalizedX(position.x()),
                  calculateNormalizedY(position.y()));
}

QPoint FloatGeometryManager::calculateCurrentPosition() const {
    return calculatePositionFromRatio(leftMarginRatio_, topMarginRatio_);
}

QPoint
FloatGeometryManager::calculatePositionFromRatio(float leftMarginRatio,
                                                 float topMarginRatio) const {
    const QSize marginSize = calculateMarginSize();

    return QPoint(marginSize.width() * leftMarginRatio,
                  marginSize.height() * topMarginRatio);
}

QPoint FloatGeometryManager::calculateNormalizedPositionFromRatio(
    float leftMarginRatio, float topMarginRatio) const {
    return calculateNormalizedPosition(
        calculatePositionFromRatio(leftMarginRatio, topMarginRatio));
}

QPoint FloatGeometryManager::calculateViewPosition() const {
    return calculateNormalizedPosition(calculateCurrentPosition());
}

QSize FloatGeometryManager::calculateMarginSize() const {
    const QSize viewPortSize = ScreenManager::getPrimaryScreenSize();
    const auto viewSize = calculateViewSize();

    const int horizontalMargin = viewPortSize.width() - viewSize.width();
    const int verticalMargin = viewPortSize.height() - viewSize.height();

    return QSize(horizontalMargin, verticalMargin);
}

QMap<QString, QVariant> FloatGeometryManager::getMarginRatioMap() const {
    QMap<QString, QVariant> marginRatioMap = {
        {leftMarginRatioKey, leftMarginRatio_},
        {topMarginRatioKey, topMarginRatio_}};

    return marginRatioMap;
}

float FloatGeometryManager::calculateLeftMarginRatio(float leftMargin) const {
    return leftMargin / calculateMarginSize().width();
}

float FloatGeometryManager::calculateTopMarginRatio(float topMargin) const {
    return topMargin / calculateMarginSize().height();
}

void FloatGeometryManager::updateMarginRatio(const QPoint &targetPosition) {
    leftMarginRatio_ = calculateLeftMarginRatio(targetPosition.x());
    topMarginRatio_ = calculateTopMarginRatio(targetPosition.y());
}

void FloatGeometryManager::saveMarginRatioMap() {
    viewSettings_.setValue(floatGeometryGroup, marginRatioMapKey,
                           getMarginRatioMap());
}

QMap<QString, QVariant> FloatGeometryManager::getDefaultMarginRatioMap() const {
    const QSize viewPortSize = ScreenManager::getPrimaryScreenSize();
    const auto viewSize = calculateViewSize();

    const int leftMargin =
        viewPortSize.width() -
        (viewSize.width() + strategy_->getDefaultRightMargin());
    const int topMargin =
        viewPortSize.height() -
        (viewSize.height() + strategy_->getDefaultBottomMargin());

    const float defaultLeftMarginRatio = calculateLeftMarginRatio(leftMargin);
    const float defaultTopMarginRatio = calculateTopMarginRatio(topMargin);

    QMap<QString, QVariant> viewDefaultMarginRatioMap = {
        {leftMarginRatioKey, defaultLeftMarginRatio},
        {topMarginRatioKey, defaultTopMarginRatio}};

    return viewDefaultMarginRatioMap;
}

void FloatGeometryManager::loadMarginRatioMap() {
    const auto marginRatioMap =
        viewSettings_
            .getValue(floatGeometryGroup, marginRatioMapKey,
                      getDefaultMarginRatioMap())
            .toMap();

    const float leftMarginRatio = marginRatioMap[leftMarginRatioKey].toFloat();
    const float topMarginRatio = marginRatioMap[topMarginRatioKey].toFloat();

    updateMarginRatio(
        calculateNormalizedPositionFromRatio(leftMarginRatio, topMarginRatio));
}

void FloatGeometryManager::moveView(const QPoint &targetPoint) {
    updateMarginRatio(targetPoint);

    const QPoint currentPosition = calculateCurrentPosition();
    emit viewMoved(currentPosition.x(), currentPosition.y());

    saveMarginRatioMap();
}
