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

#include "config-button-animation-helper.h"
#include "config-button-animator.h"
#include <QWidget>
using namespace LINGMOConfigStyleSpace;

ConfigButtonAnimationHelper::ConfigButtonAnimationHelper(QObject *parent) : AnimationHelper(parent)
{

}

ConfigButtonAnimationHelper::~ConfigButtonAnimationHelper()
{

}

bool ConfigButtonAnimationHelper::registerWidget(QWidget *w)
{
    auto animator = new LINGMOConfigStyleSpace::ConfigButtonAnimator;
    bool result = animator->bindWidget(w);
    if (!result)
    {
        animator->deleteLater();
        animator = nullptr;
    }
    else
    {
        m_animators->insertMulti(w, animator);
    }

    connect(w, &QWidget::destroyed, this, [=](){
       unregisterWidget(w);
    });

    return result;
}

bool ConfigButtonAnimationHelper::unregisterWidget(QWidget *w)
{
    auto animators= m_animators->values(w);
    bool result = false;
    for (auto animator : animators) {
        if (animator) {
            result = animator->unboundWidget();
            delete animator;
            animator = nullptr;
        }
    }
    m_animators->remove(w);
    return result;
}

AnimatorIface *ConfigButtonAnimationHelper::animator(const QWidget *w)
{
    return m_animators->value(w);
}
