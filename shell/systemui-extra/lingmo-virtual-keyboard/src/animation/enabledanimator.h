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

#ifndef ENABLEDANIMATOR_H
#define ENABLEDANIMATOR_H

#include <functional>

#include "animation/animationfactory.h"
#include "animation/animator.h"

class EnabledAnimator : public Animator {
    Q_OBJECT

    using IsFloatModeCallback = std::function<bool()>;

public:
    EnabledAnimator(IsFloatModeCallback isFloatModeCallback,
                    std::unique_ptr<AnimationFactory> animationFactory);
    ~EnabledAnimator() override;

    void playShowAnimation(QWindow *view, const QRect &endGeometry) override;
    void playHideAnimation(QWindow *view, const QRect &endGeometry) override;
    void playFlipAnimation(QWindow *view, const QRect &endGeometry) override;

    void
    setAnimationFactory(std::unique_ptr<AnimationFactory> animationFactory);

private:
    bool isFloatMode() const {
        if (isFloatModeCallback_ == nullptr) {
            return false;
        }

        return isFloatModeCallback_();
    }

    void playAnimation(std::unique_ptr<QAbstractAnimation> animation);

    std::unique_ptr<QAbstractAnimation>
    createShowAnimation(QWindow *view, const QRect &endGeometry);
    std::unique_ptr<QAbstractAnimation>
    createHideAnimation(QWindow *view, const QRect &endGeometry);
    std::unique_ptr<QAbstractAnimation>
    createFlipAnimation(QWindow *view, const QRect &endGeometry);

    AnimationFactory::AnimationInfo
    createShowAnimationInfo(QWindow *view, const QRect &endGeometry);
    AnimationFactory::AnimationInfo
    createHideAnimationInfo(QWindow *view, const QRect &endGeometry);
    AnimationFactory::AnimationInfo
    createBasicAnimationInfo(QWindow *view, const QRect &endGeometry);
    AnimationFactory::AnimationInfo
    createExpansionAnimationInfo(QWindow *view, const QRect &endGeometry);
    AnimationFactory::AnimationInfo
    createFloatAnimationInfo(QWindow *view, const QRect &endGeometry,
                             float endOpacity);

    int calculateAnimationDuration(int startY, int endY,
                                   int virtualKeyboardHeight);
    void connectAnimationFinishedSignal();

private:
    const int animationDuration_ = 100;

    IsFloatModeCallback isFloatModeCallback_;

    std::unique_ptr<QAbstractAnimation> animation_ = nullptr;
    std::unique_ptr<AnimationFactory> animationFactory_ = nullptr;
};

#endif // ENABLEDANIMATOR_H
