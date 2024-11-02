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

#include "animation/floatanimationfactory.h"

#include <QParallelAnimationGroup>

std::unique_ptr<QAbstractAnimation>
FloatAnimationFactory::createShowAnimation(QObject *target,
                                           const AnimationInfo &animationInfo) {
    return createYAndOpacityAnimationGroup(target, animationInfo);
}

std::unique_ptr<QAbstractAnimation>
FloatAnimationFactory::createHideAnimation(QObject *target,
                                           const AnimationInfo &animationInfo) {
    return createYAndOpacityAnimationGroup(target, animationInfo);
}

std::unique_ptr<QAbstractAnimation>
FloatAnimationFactory::createFlipAnimation(QObject *target,
                                           const AnimationInfo &animationInfo) {
    return createPropertyAnimation(target, "y", animationInfo.startY,
                                   animationInfo.endY, animationInfo.duration);
}

std::unique_ptr<QAbstractAnimation>
FloatAnimationFactory::createYAndOpacityAnimationGroup(
    QObject *target, const AnimationInfo &animationInfo) {
    std::unique_ptr<QParallelAnimationGroup> animationGroup(
        new QParallelAnimationGroup());
    animationGroup->addAnimation(
        createPropertyAnimation(target, "y", animationInfo.startY,
                                animationInfo.endY, animationInfo.duration)
            .release());
    animationGroup->addAnimation(
        createPropertyAnimation(target, "opacity", animationInfo.startOpacity,
                                animationInfo.endOpacity,
                                animationInfo.duration)
            .release());
    return animationGroup;
}
