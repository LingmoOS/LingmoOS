/*
 * Qt5-LINGMO's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: xibowen <xibowen@kylinos.cn>
 *
 */

#include "config-progressbar-animation-helper.h"

using namespace LINGMOConfigStyleSpace;

ConfigProgressBarAnimationHelper::ConfigProgressBarAnimationHelper(QObject *parent) : QObject(parent)
{
    animations = new QHash<QObject *, QVariantAnimation *>();
}



ConfigProgressBarAnimationHelper::~ConfigProgressBarAnimationHelper()
{
    for(auto item = animations->begin(); item != animations->end(); item++){
        if(item.value()){
//            delete item.value();
//            item.value() = nullptr;
        }
    }

    if(animations){
        delete animations;
        animations = nullptr;
    }
}



void ConfigProgressBarAnimationHelper::startAnimation(QVariantAnimation *animation)
{
    stopAnimation(animation->parent());
    connect(animation, SIGNAL(destroyed()), SLOT(_q_removeAnimation()), Qt::UniqueConnection);
    animations->insert(animation->parent(), animation);
    animation->start();
}



void ConfigProgressBarAnimationHelper::stopAnimation(QObject *target)
{
    QVariantAnimation *animation = animations->take(target);
    if (animation) {
        animation->stop();
        delete animation;
        animation = nullptr;
    }
}



QVariantAnimation* ConfigProgressBarAnimationHelper::animation(QObject *target)
{
    return animations->value(target);
}



void ConfigProgressBarAnimationHelper::_q_removeAnimation()
{
    QObject *animation = this->sender();
    if (animation)
        animations->remove(animation->parent());
}
