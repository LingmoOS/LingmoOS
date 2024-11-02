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

#ifndef EXPANSIONANIMATIONFACTORY_H
#define EXPANSIONANIMATIONFACTORY_H

#include "animation/animationfactory.h"

class ExpansionAnimationFactory : public AnimationFactory {
public:
    explicit ExpansionAnimationFactory() = default;
    ~ExpansionAnimationFactory() override = default;

    std::unique_ptr<QAbstractAnimation>
    createShowAnimation(QObject *target,
                        const AnimationInfo &animationInfo) override;
    std::unique_ptr<QAbstractAnimation>
    createHideAnimation(QObject *target,
                        const AnimationInfo &animationInfo) override;
    std::unique_ptr<QAbstractAnimation>
    createFlipAnimation(QObject *target,
                        const AnimationInfo &animationInfo) override;

private:
    static std::unique_ptr<QAbstractAnimation>
    createYAnimation(QObject *target, const AnimationInfo &animationInfo);
};

#endif // EXPANSIONANIMATIONFACTORY_H
