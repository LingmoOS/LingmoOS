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

#ifndef ANIMATIONFACTORY_H
#define ANIMATIONFACTORY_H

#include <memory>

#include <QAbstractAnimation>
#include <QObject>
#include <QVariant>

class AnimationFactory {
public:
    struct AnimationInfo {
        int startY;
        int endY;
        float startOpacity = 1.0f;
        float endOpacity = 1.0f;
        int duration = 200;
    };

    AnimationFactory() = default;
    virtual ~AnimationFactory() = default;

    virtual std::unique_ptr<QAbstractAnimation>
    createShowAnimation(QObject *target,
                        const AnimationInfo &animationInfo) = 0;
    virtual std::unique_ptr<QAbstractAnimation>
    createHideAnimation(QObject *target,
                        const AnimationInfo &animationInfo) = 0;
    virtual std::unique_ptr<QAbstractAnimation>
    createFlipAnimation(QObject *target,
                        const AnimationInfo &animationInfo) = 0;

protected:
    static std::unique_ptr<QAbstractAnimation>
    createPropertyAnimation(QObject *target, const QByteArray &propertyName,
                            const QVariant &startValue,
                            const QVariant &endValue, int duration);
};

#endif // ANIMATIONFACTORY_H
