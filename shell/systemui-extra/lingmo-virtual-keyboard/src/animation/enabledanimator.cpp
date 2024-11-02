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

#include "animation/enabledanimator.h"

EnabledAnimator::EnabledAnimator(
    IsFloatModeCallback isFloatModeCallback,
    std::unique_ptr<AnimationFactory> animationFactory)
    : Animator(), isFloatModeCallback_(std::move(isFloatModeCallback)),
      animationFactory_(std::move(animationFactory)) {}

EnabledAnimator::~EnabledAnimator() { animation_.reset(); }

void EnabledAnimator::setAnimationFactory(
    std::unique_ptr<AnimationFactory> animationFactory) {
    animationFactory_ = std::move(animationFactory);
}

void EnabledAnimator::playShowAnimation(QWindow *view,
                                        const QRect &endGeometry) {
    view->show();
    playAnimation(createShowAnimation(view, endGeometry));
}

void EnabledAnimator::playHideAnimation(QWindow *view,
                                        const QRect &endGeometry) {
    playAnimation(createHideAnimation(view, endGeometry));
}

void EnabledAnimator::playFlipAnimation(QWindow *view,
                                        const QRect &endGeometry) {
    playAnimation(createFlipAnimation(view, endGeometry));
}

void EnabledAnimator::playAnimation(
    std::unique_ptr<QAbstractAnimation> animation) {
    animation_ = std::move(animation);
    animation_->start();
    connectAnimationFinishedSignal();
}

std::unique_ptr<QAbstractAnimation>
EnabledAnimator::createShowAnimation(QWindow *view, const QRect &endGeometry) {
    return animationFactory_->createShowAnimation(
        view, createShowAnimationInfo(view, endGeometry));
}

std::unique_ptr<QAbstractAnimation>
EnabledAnimator::createHideAnimation(QWindow *view, const QRect &endGeometry) {
    return animationFactory_->createHideAnimation(
        view, createHideAnimationInfo(view, endGeometry));
}

std::unique_ptr<QAbstractAnimation>
EnabledAnimator::createFlipAnimation(QWindow *view, const QRect &endGeometry) {
    return animationFactory_->createFlipAnimation(
        view, createBasicAnimationInfo(view, endGeometry));
}

AnimationFactory::AnimationInfo
EnabledAnimator::createShowAnimationInfo(QWindow *view,
                                         const QRect &endGeometry) {
    if (isFloatMode()) {
        return createFloatAnimationInfo(view, endGeometry, 1.0f);
    } else {
        return createExpansionAnimationInfo(view, endGeometry);
    }
}

AnimationFactory::AnimationInfo
EnabledAnimator::createHideAnimationInfo(QWindow *view,
                                         const QRect &endGeometry) {
    if (isFloatMode()) {
        return createFloatAnimationInfo(view, endGeometry, 0.0f);
    } else {
        return createExpansionAnimationInfo(view, endGeometry);
    }
}

AnimationFactory::AnimationInfo
EnabledAnimator::createBasicAnimationInfo(QWindow *view,
                                          const QRect &endGeometry) {
    AnimationFactory::AnimationInfo animationInfo;

    animationInfo.startY = view->y();
    animationInfo.endY = endGeometry.y();
    animationInfo.duration = calculateAnimationDuration(
        animationInfo.startY, animationInfo.endY, view->height());

    return animationInfo;
}

AnimationFactory::AnimationInfo
EnabledAnimator::createExpansionAnimationInfo(QWindow *view,
                                              const QRect &endGeometry) {
    return createBasicAnimationInfo(view, endGeometry);
}

AnimationFactory::AnimationInfo EnabledAnimator::createFloatAnimationInfo(
    QWindow *view, const QRect &endGeometry, float endOpacity) {
    AnimationFactory::AnimationInfo animationInfo =
        createBasicAnimationInfo(view, endGeometry);

    animationInfo.startOpacity = view->opacity();
    animationInfo.endOpacity = endOpacity;

    return animationInfo;
}

int EnabledAnimator::calculateAnimationDuration(int startY, int endY,
                                                int virtualKeyboardHeight) {
    auto factor =
        static_cast<float>(std::abs(endY - startY)) / virtualKeyboardHeight;
    return static_cast<int>(factor * animationDuration_);
}

void EnabledAnimator::connectAnimationFinishedSignal() {
    connect(animation_.get(), &QAbstractAnimation::finished, this,
            &Animator::animationFinished);
}
