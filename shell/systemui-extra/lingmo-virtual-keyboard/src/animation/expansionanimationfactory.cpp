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

#include "animation/expansionanimationfactory.h"

std::unique_ptr<QAbstractAnimation>
ExpansionAnimationFactory::createShowAnimation(
    QObject *target, const AnimationInfo &animationInfo) {
    return createYAnimation(target, animationInfo);
}

std::unique_ptr<QAbstractAnimation>
ExpansionAnimationFactory::createHideAnimation(
    QObject *target, const AnimationInfo &animationInfo) {
    return createYAnimation(target, animationInfo);
}

std::unique_ptr<QAbstractAnimation>
ExpansionAnimationFactory::createFlipAnimation(
    QObject *target, const AnimationInfo &animationInfo) {
    return createYAnimation(target, animationInfo);
}

std::unique_ptr<QAbstractAnimation> ExpansionAnimationFactory::createYAnimation(
    QObject *target, const AnimationInfo &animationInfo) {
    return createPropertyAnimation(target, "y", animationInfo.startY,
                                   animationInfo.endY, animationInfo.duration);
}
