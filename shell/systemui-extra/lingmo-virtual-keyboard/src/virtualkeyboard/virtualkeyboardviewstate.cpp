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

#include "virtualkeyboard/virtualkeyboardview.h"
#include "virtualkeyboardsettings/virtualkeyboardsettings.h"

class VirtualKeyboardView::State {
public:
    virtual ~State() = default;

    virtual bool isVirtualKeyboardVisible() = 0;

    virtual void show() = 0;
    virtual void hide() = 0;
    virtual void flip() = 0;

    virtual void onAnimationFinished() = 0;

protected:
    explicit State(VirtualKeyboardView &virtualKeyboardView)
        : virtualKeyboardView_(virtualKeyboardView) {}

protected:
    VirtualKeyboardView &virtualKeyboardView_;
};

class VirtualKeyboardView::VisibleState : public VirtualKeyboardView::State {
public:
    explicit VisibleState(VirtualKeyboardView &virtualKeyboardView)
        : State(virtualKeyboardView) {}
    ~VisibleState() override = default;

    bool isVirtualKeyboardVisible() override { return true; }

    void show() override {}
    void hide() override { virtualKeyboardView_.enterHidingState(); }
    void flip() override { virtualKeyboardView_.enterFlippingState(); }

    void onAnimationFinished() override {}
};

class VirtualKeyboardView::HidingState : public VirtualKeyboardView::State {
public:
    HidingState(VirtualKeyboardView &virtualKeyboardView)
        : State(virtualKeyboardView) {}
    ~HidingState() override = default;

    bool isVirtualKeyboardVisible() override { return false; }

    void show() override { virtualKeyboardView_.enterShowingState(); }
    void hide() override {}
    void flip() override {}

    void onAnimationFinished() override {
        virtualKeyboardView_.enterInvisibleState();
    }
};

class VirtualKeyboardView::ShowingState : public VirtualKeyboardView::State {
public:
    ShowingState(VirtualKeyboardView &virtualKeyboardView)
        : State(virtualKeyboardView) {}
    ~ShowingState() override = default;

    bool isVirtualKeyboardVisible() override { return true; }

    void show() override {}
    void hide() override { virtualKeyboardView_.enterHidingState(); }
    void flip() override {}

    void onAnimationFinished() override {
        virtualKeyboardView_.enterVisibleState();
    }
};

class VirtualKeyboardView::InvisibleState : public VirtualKeyboardView::State {
public:
    InvisibleState(VirtualKeyboardView &virtualKeyboardView)
        : State(virtualKeyboardView) {}
    ~InvisibleState() override = default;

    bool isVirtualKeyboardVisible() override { return false; }

    void show() override {
        virtualKeyboardView_.initView();

        virtualKeyboardView_.enterShowingState();
    }
    void hide() override {}
    void flip() override {}

    void onAnimationFinished() override {}
};

class VirtualKeyboardView::FlippingState : public VirtualKeyboardView::State {
public:
    FlippingState(VirtualKeyboardView &virtualKeyboardView)
        : State(virtualKeyboardView) {}
    ~FlippingState() override = default;

    bool isVirtualKeyboardVisible() override { return true; }

    void show() override {}
    void hide() override { virtualKeyboardView_.enterInvisibleState(); }
    void flip() override{};

    void onAnimationFinished() override {
        if (!virtualKeyboardView_.isFloatMode()) {
            virtualKeyboardView_.updateGeometry();
        }

        virtualKeyboardView_.enterVisibleState();
    }
};

void VirtualKeyboardView::initState() {
    visibleState_ = std::make_shared<VisibleState>(*this);
    hidingState_ = std::make_shared<HidingState>(*this);
    showingState_ = std::make_shared<ShowingState>(*this);
    invisibleState_ = std::make_shared<InvisibleState>(*this);
    flippingState_ = std::make_shared<FlippingState>(*this);

    updateCurrentState(invisibleState_);
}

bool VirtualKeyboardView::isVisible() const {
    return currentState_->isVirtualKeyboardVisible();
}

void VirtualKeyboardView::show() { currentState_->show(); }

void VirtualKeyboardView::hide() { currentState_->hide(); }

void VirtualKeyboardView::flip() { currentState_->flip(); }

void VirtualKeyboardView::setAnimator(std::shared_ptr<Animator> animator) {
    animator_ = animator;
    connect(animator_.get(), &Animator::animationFinished, this,
            [this]() { currentState_->onAnimationFinished(); });
}

void VirtualKeyboardView::updateCurrentState(std::shared_ptr<State> newState) {
    currentState_ = newState;
}

void VirtualKeyboardView::enterVisibleState() {
    updateCurrentState(visibleState_);

    if (!isFloatMode()) {
        emit raiseAppRequested();
    }
}

void VirtualKeyboardView::enterHidingState() {
    updateCurrentState(hidingState_);

    animator_->playHideAnimation(view_.get(), calculateInitialGeometry());
}

void VirtualKeyboardView::enterShowingState() {
    updateCurrentState(showingState_);

    animator_->playShowAnimation(view_.get(), geometry());
}

void VirtualKeyboardView::enterInvisibleState() {
    updateCurrentState(invisibleState_);

    destroyView();
}

void VirtualKeyboardView::enterFlippingState() {
    updateCurrentState(flippingState_);

    placementModeManager_->flipPlacementMode();

    if (isFloatMode()) {
        updateExpansionFlippingStartGeometry();

        emit fallAppRequested();
    }

    animator_->playFlipAnimation(view_.get(), geometry());
}
