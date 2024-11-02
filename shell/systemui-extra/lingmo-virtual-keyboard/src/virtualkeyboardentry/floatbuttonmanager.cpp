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

#include "virtualkeyboardentry/floatbuttonmanager.h"

#include <QEvent>
#include <QGuiApplication>
#include <QIcon>
#include <QMouseEvent>
#include <QScreen>
#include <QTime>

#include "geometrymanager/floatgeometrymanager.h"
#include "geometrymanager/geometrymanager.h"
#include "virtualkeyboardentry/floatbuttonstrategy.h"

FloatButtonManager::FloatButtonManager(
    const VirtualKeyboardManager &virtualKeyboardManager,
    const FcitxVirtualKeyboardService &fcitxVirtualKeyboardService,
    LocalSettings &floatButtonSettings)
    : virtualKeyboardManager_(virtualKeyboardManager),
      fcitxVirtualKeyboardService_(fcitxVirtualKeyboardService),
      floatButtonSettings_(floatButtonSettings) {
    initGeometryManager();

    initInternalSignalConnections();
    initScreenSignalConnections();
}

void FloatButtonManager::enableFloatButton() { setFloatButtonEnabled(true); }

void FloatButtonManager::disableFloatButton() { setFloatButtonEnabled(false); }

void FloatButtonManager::initGeometryManager() {
    geometryManager_.reset(new FloatGeometryManager(
        std::unique_ptr<FloatGeometryManager::Strategy>(
            new FloatButtonStrategy()),
        floatButtonSettings_));
}

void FloatButtonManager::initScreenSignalConnections() {
    connect(QGuiApplication::primaryScreen(), &QScreen::geometryChanged, this,
            &FloatButtonManager::onScreenResolutionChanged);
}

void FloatButtonManager::initInternalSignalConnections() {
    connect(this, &FloatButtonManager::floatButtonEnabled, this,
            &FloatButtonManager::initFloatButton);
    connect(this, &FloatButtonManager::floatButtonDisabled, this,
            &FloatButtonManager::destroyFloatButton);
}

void FloatButtonManager::initFloatButton() {
    fcitxVirtualKeyboardService_.hideVirtualKeyboard();

    createFloatButton();

    connectFloatButtonSignals();

    geometryManager_->updateGeometry();
}

void FloatButtonManager::destroyFloatButton() {
    if (floatButton_ == nullptr) {
        return;
    }

    // 销毁之前必须隐藏，否则会导致虚拟键盘进程
    // 直接退出
    floatButton_->hide();

    floatButton_.reset();
}

void FloatButtonManager::onScreenResolutionChanged() {
    if (!floatButtonEnabled_ || !floatButton_->isVisible()) {
        return;
    }

    geometryManager_->updateGeometry();
}

void FloatButtonManager::showFloatButton() {
    if (!floatButtonEnabled_) {
        return;
    }

    floatButton_->show();
}

void FloatButtonManager::hideFloatButton() {
    if (!floatButtonEnabled_) {
        return;
    }

    floatButton_->hide();
}

void FloatButtonManager::createFloatButton() {
    floatButton_.reset(new FloatButton(
        [this]() { fcitxVirtualKeyboardService_.showVirtualKeyboard(); }));
    floatButton_->setWindowFlags(Qt::FramelessWindowHint |
                                 Qt::BypassWindowManagerHint | Qt::Tool);

    floatButton_->show();
}

void FloatButtonManager::connectFloatButtonSignals() {
    connect(floatButton_.get(), &FloatButton::mouseMoved,
            geometryManager_.get(), &FloatGeometryManager::moveBy);
    connect(floatButton_.get(), &FloatButton::mouseReleased,
            geometryManager_.get(), &FloatGeometryManager::endDrag);

    connect(geometryManager_.get(), &FloatGeometryManager::viewMoved,
            floatButton_.get(), &FloatButton::move);
    connect(geometryManager_.get(), &FloatGeometryManager::viewResized,
            floatButton_.get(), &FloatButton::resize);

    connect(&virtualKeyboardManager_,
            &VirtualKeyboardManager::virtualKeyboardVisibiltyChanged,
            floatButton_.get(), [this](bool visible) {
                if (visible) {
                    hideFloatButton();
                } else {
                    showFloatButton();
                }
            });
}

void FloatButtonManager::updateFloatButtonEnabled(bool enabled) {
    floatButtonEnabled_ = enabled;

    if (floatButtonEnabled_) {
        emit floatButtonEnabled();
    } else {
        emit floatButtonDisabled();
    }
}

void FloatButtonManager::setFloatButtonEnabled(bool enabled) {
    if (floatButtonEnabled_ == enabled) {
        return;
    }

    updateFloatButtonEnabled(enabled);
}
