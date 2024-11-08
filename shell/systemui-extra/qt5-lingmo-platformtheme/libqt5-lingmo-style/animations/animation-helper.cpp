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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "animation-helper.h"

#include <QWidget>
#include "animator-iface.h"

AnimationHelper::AnimationHelper(QObject *parent) : QObject(parent)
{
    m_animators = new QHash<const QWidget *, AnimatorIface *>();
}

AnimationHelper::~AnimationHelper()
{
    for(auto item = m_animators->begin(); item != m_animators->end(); item++){
        if(item.value()){
            delete item.value();
            item.value() = nullptr;
        }
    }
    if(m_animators){
        delete m_animators;
        m_animators = nullptr;
    }
}
