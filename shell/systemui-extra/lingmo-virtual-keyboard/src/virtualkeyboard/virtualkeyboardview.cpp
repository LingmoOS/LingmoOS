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

#include "virtualkeyboard/virtualkeyboardview.h"
#include "virtualkeyboardsettings/virtualkeyboardsettings.h"

#include <QQmlContext>
#include <QQuickItem>

#include "screenmanager.h"

VirtualKeyboardView::VirtualKeyboardView(
    QObject &manager, QObject &model,
    std::unique_ptr<PlacementModeManager> placementModeManager,
    std::unique_ptr<ExpansionGeometryManager> expansionGeometryManager,
    std::unique_ptr<FloatGeometryManager> floatGeometryManager)
    : manager_(manager), model_(model),
      placementModeManager_(std::move(placementModeManager)),
      expansionGeometryManager_(std::move(expansionGeometryManager)),
      floatGeometryManager_(std::move(floatGeometryManager)) {

    connect(placementModeManager_.get(),
            &PlacementModeManager::isFloatModeChanged, this,
            &VirtualKeyboardView::isFloatModeChanged);
    connect(floatGeometryManager_.get(), &FloatGeometryManager::viewMoved, this,
            &VirtualKeyboardView::move);

    initState();
}

VirtualKeyboardView::~VirtualKeyboardView() {
    destroyView();

    currentState_.reset();
    visibleState_.reset();
    hidingState_.reset();
    showingState_.reset();
    invisibleState_.reset();
    flippingState_.reset();
}

void VirtualKeyboardView::moveBy(int offsetX, int offsetY) {
    if (!isFloatMode()) {
        return;
    }

    floatGeometryManager_->moveBy(offsetX, offsetY);
}

void VirtualKeyboardView::endDrag() {
    if (!isFloatMode()) {
        return;
    }

    floatGeometryManager_->endDrag();
}

QRect VirtualKeyboardView::geometry() const {
    return getCurrentGeometryManager().geometry();
}

void VirtualKeyboardView::updateGeometry() {
    if (!isVisible()) {
        return;
    }

    view_->setGeometry(geometry());

    emitContentGeometrySignals();
}

void VirtualKeyboardView::updateExpansionFlippingStartGeometry() {
    auto geometry = floatGeometryManager_->geometry();
    view_->setGeometry(geometry.x(), view_->y(), geometry.width(),
                       geometry.height());

    emitContentGeometrySignals();
}

void VirtualKeyboardView::move(int x, int y) {
    view_->setX(x);
    view_->setY(y);
}

void VirtualKeyboardView::initView() {
    view_.reset(new QQuickView());

    view_->rootContext()->setContextProperty("manager", &manager_);
    view_->rootContext()->setContextProperty("model", &model_);
    view_->rootContext()->setContextProperty("view", this);

    view_->setTitle("lingmo-virtual-keyboard");
    view_->setColor(QColor(Qt::transparent));
    view_->setSource(QUrl("qrc:/qml/VirtualKeyboard.qml"));
    view_->setFlags(Qt::Window | Qt::WindowDoesNotAcceptFocus |
                    Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
                    Qt::BypassWindowManagerHint);

    view_->setGeometry(calculateInitialGeometry());
    setViewOpacity();

    connectSignals();
}

QRect VirtualKeyboardView::calculateInitialGeometry() {
    int normalizedY =
        std::min(getScreenHeight(), geometry().y() + geometry().height());

    return QRect(geometry().x(), normalizedY, geometry().width(),
                 geometry().height());
}

int VirtualKeyboardView::getScreenHeight() {
    return ScreenManager::getPrimaryScreenSize().height();
}

void VirtualKeyboardView::connectSignals() {
    auto *rootObject = view_->rootObject();

    connect(this, SIGNAL(updateCandidateArea(const QVariant &, int)),
            rootObject, SIGNAL(qmlUpdateCandidateList(QVariant, int)));
    connect(this, SIGNAL(imDeactivated()), rootObject,
            SIGNAL(qmlImDeactivated()));
}

void VirtualKeyboardView::destroyView() {
    if (view_ == nullptr) {
        return;
    }

    if (view_->isVisible()) {
        view_->hide();
    }

    view_.release()->deleteLater();
}

void VirtualKeyboardView::emitContentGeometrySignals() {
    emit contentHeightChanged();
    emit contentWidthChanged();
}

int VirtualKeyboardView::getContentWidth() {
    return getCurrentGeometryManager().getViewContentWidth();
}

int VirtualKeyboardView::getContentHeight() {
    return getCurrentGeometryManager().getViewContentHeight();
}

void VirtualKeyboardView::setViewOpacity() {
    if (VirtualKeyboardSettings::getInstance().isAnimationEnabled()) {
        view_->setOpacity(isFloatMode() ? 0.0f : 1.0f);
    }
    connect(
        &VirtualKeyboardSettings::getInstance(),
        &VirtualKeyboardSettings::animationAvailabilityChanged, this, [this]() {
            if (!VirtualKeyboardSettings::getInstance().isAnimationEnabled()) {
                view_->setOpacity(1.0f);
            }
        });
}

GeometryManager &VirtualKeyboardView::getCurrentGeometryManager() const {
    if (isFloatMode()) {
        return *floatGeometryManager_;
    } else {
        return *expansionGeometryManager_;
    }
}
