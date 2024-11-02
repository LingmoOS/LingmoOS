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

#include "virtualkeyboardmanager.h"

#include <QGuiApplication>
#include <QScreen>

#include "animation/disabledanimator.h"
#include "animation/enabledanimator.h"
#include "animation/expansionanimationfactory.h"
#include "animation/floatanimationfactory.h"
#include "virtualkeyboardsettings/virtualkeyboardsettings.h"
#include "virtualkeyboardstrategy.h"

VirtualKeyboardManager::VirtualKeyboardManager(
    HideVirtualKeyboardCallback hideVirtualKeyboardCallback)
    : hideVirtualKeyboardCallback_(std::move(hideVirtualKeyboardCallback)) {
    initVirtualKeyboardModel();

    initAppInputAreaManager();

    initVirtualKeyboardView();

    initScreenSignalConnections();
}

VirtualKeyboardManager::~VirtualKeyboardManager() {
    hideVirtualKeyboard();

    appInputAreaManager_.reset();

    view_.reset();
    model_.reset();
}

void VirtualKeyboardManager::showVirtualKeyboard() {
    if (isVirtualKeyboardVisible()) {
        return;
    }

    view_->show();

    visibiltyChanged();
}

void VirtualKeyboardManager::hideVirtualKeyboard() {
    if (!isVirtualKeyboardVisible()) {
        return;
    }

    appInputAreaManager_->fallInputArea();

    view_->hide();

    visibiltyChanged();
}

void VirtualKeyboardManager::hide() {
    if (!hideVirtualKeyboardCallback_) {
        return;
    }

    hideVirtualKeyboardCallback_();
}

void VirtualKeyboardManager::flipPlacementMode() { view_->flip(); }

void VirtualKeyboardManager::moveBy(int offsetX, int offsetY) {
    view_->moveBy(offsetX, offsetY);
}

void VirtualKeyboardManager::endDrag() { view_->endDrag(); }

void VirtualKeyboardManager::visibiltyChanged() {
    emit virtualKeyboardVisibiltyChanged(isVirtualKeyboardVisible());
}

bool VirtualKeyboardManager::isVirtualKeyboardVisible() const {
    return view_->isVisible();
}

void VirtualKeyboardManager::updatePreeditCaret(int index) {
    model_->setPreeditCaret(index);
}

void VirtualKeyboardManager::updatePreeditArea(const QString &preeditText) {
    model_->setPreeditText(preeditText);
}

void VirtualKeyboardManager::updateCandidateArea(
    const QStringList &candidateTextList, bool hasPrev, bool hasNext,
    int pageIndex, int globalCursorIndex) {
    model_->updateCandidateArea(QVariant(candidateTextList), hasPrev, hasNext,
                                pageIndex, globalCursorIndex);
}

void VirtualKeyboardManager::notifyIMActivated(const QString &uniqueName) {
    model_->setUniqueName(uniqueName);
}

void VirtualKeyboardManager::notifyIMDeactivated(
    const QString & /*uniqueName*/) {
    emit model_->imDeactivated();
}

void VirtualKeyboardManager::notifyIMListChanged() {
    model_->syncCurrentIMList();
}

void VirtualKeyboardManager::processResolutionChangedEvent() {
    if (isVirtualKeyboardVisible()) {
        view_->updateGeometry();
    }
}

void VirtualKeyboardManager::initAppInputAreaManager() {
    appInputAreaManager_.reset(new AppInputAreaManager(this));
}

std::unique_ptr<PlacementModeManager>
VirtualKeyboardManager::createPlacementModeManager() {
    return std::unique_ptr<PlacementModeManager>(
        new PlacementModeManager(viewSettings_));
}

Scaler VirtualKeyboardManager::createFloatModeScaler() {
    return Scaler(
        []() {
            return VirtualKeyboardSettings::getInstance()
                .calculateVirtualKeyboardScaleFactor();
        },
        []() {
            return VirtualKeyboardSettings::getInstance()
                .calculateVirtualKeyboardScaleFactor();
        },
        []() {
            return VirtualKeyboardSettings::getInstance()
                .calculateVirtualKeyboardScaleFactor();
        });
}

Scaler VirtualKeyboardManager::createExpansionModeScaler() {
    return Scaler([]() { return 1.0f; },
                  []() {
                      return VirtualKeyboardSettings::getInstance()
                          .calculateVirtualKeyboardScaleFactor();
                  },
                  []() {
                      return VirtualKeyboardSettings::getInstance()
                          .calculateVirtualKeyboardScaleFactor();
                  });
}

std::unique_ptr<ExpansionGeometryManager>
VirtualKeyboardManager::createExpansionGeometryManager() {
    return std::unique_ptr<ExpansionGeometryManager>(
        new ExpansionGeometryManager(createExpansionModeScaler()));
}

std::unique_ptr<FloatGeometryManager>
VirtualKeyboardManager::createFloatGeometryManger() {
    return std::unique_ptr<FloatGeometryManager>(new FloatGeometryManager(
        std::unique_ptr<FloatGeometryManager::Strategy>(
            new VirtualKeyboardStrategy()),
        viewSettings_, createFloatModeScaler()));
}

void VirtualKeyboardManager::initVirtualKeyboardModel() {
    model_.reset(new VirtualKeyboardModel(this));

    connect(model_.get(), SIGNAL(backendConnectionDisconnected()), this,
            SLOT(hideVirtualKeyboard()));
}

void VirtualKeyboardManager::initVirtualKeyboardView() {
    view_.reset(new VirtualKeyboardView(
        *this, *model_, createPlacementModeManager(),
        createExpansionGeometryManager(), createFloatGeometryManger()));
    view_->setAnimator(createAnimator());

    connectVirtualKeyboardModelSignals();

    connectVirtualKeyboardViewSignals();

    connectVirtualKeyboardSettingsSignals();
}

void VirtualKeyboardManager::connectVirtualKeyboardModelSignals() {
    connect(model_.get(), SIGNAL(updateCandidateArea(const QVariant &, int)),
            view_.get(), SIGNAL(updateCandidateArea(const QVariant &, int)));
    connect(model_.get(), SIGNAL(imDeactivated()), view_.get(),
            SIGNAL(imDeactivated()));
}

void VirtualKeyboardManager::connectVirtualKeyboardViewSignals() {
    connect(
        view_.get(), &VirtualKeyboardView::raiseAppRequested, this,
        [this]() { appInputAreaManager_->raiseInputArea(view_->geometry()); });

    connect(view_.get(), &VirtualKeyboardView::fallAppRequested, this,
            [this]() { appInputAreaManager_->fallInputArea(); });
}

void VirtualKeyboardManager::connectVirtualKeyboardSettingsSignals() {
    connect(&VirtualKeyboardSettings::getInstance(),
            &VirtualKeyboardSettings::scaleFactorChanged, view_.get(),
            [this]() {
                view_->updateGeometry();
                raiseInputAreaIfNecessary();
            });

    connect(&VirtualKeyboardSettings::getInstance(),
            &VirtualKeyboardSettings::animationAvailabilityChanged, this,
            [this]() { view_->setAnimator(createAnimator()); });
}

void VirtualKeyboardManager::initScreenSignalConnections() {
    connect(QGuiApplication::primaryScreen(),
            SIGNAL(geometryChanged(const QRect &)), this,
            SLOT(processResolutionChangedEvent()));
}

void VirtualKeyboardManager::raiseInputAreaIfNecessary() {
    if (!view_->isVisible()) {
        return;
    }

    if (view_->isFloatMode()) {
        return;
    }

    appInputAreaManager_->raiseInputArea(view_->geometry());
}

std::unique_ptr<AnimationFactory>
VirtualKeyboardManager::createAnimationFactory() {
    if (view_->isFloatMode()) {
        return std::unique_ptr<AnimationFactory>(new FloatAnimationFactory());
    } else {
        return std::unique_ptr<AnimationFactory>(
            new ExpansionAnimationFactory());
    }
}

std::unique_ptr<Animator> VirtualKeyboardManager::createEnabledAnimator() {
    auto animator = std::unique_ptr<EnabledAnimator>(new EnabledAnimator(
        [this]() { return view_->isFloatMode(); }, createAnimationFactory()));

    EnabledAnimator *enabledAnimator = animator.get();
    connect(view_.get(), &VirtualKeyboardView::isFloatModeChanged,
            enabledAnimator, [this, enabledAnimator]() {
                enabledAnimator->setAnimationFactory(createAnimationFactory());
            });

    return animator;
}
std::unique_ptr<Animator> VirtualKeyboardManager::createDisabledAnimator() {
    return std::unique_ptr<Animator>(new DisabledAnimator());
}

std::unique_ptr<Animator> VirtualKeyboardManager::createAnimator() {
    if (VirtualKeyboardSettings::getInstance().isAnimationEnabled()) {
        return createEnabledAnimator();
    } else {
        return createDisabledAnimator();
    }
}
