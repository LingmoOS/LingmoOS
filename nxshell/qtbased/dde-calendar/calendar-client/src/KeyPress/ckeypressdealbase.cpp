// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ckeypressdealbase.h"

#include "cgraphicsscene.h"
#include "graphicsItem/cscenebackgrounditem.h"

CKeyPressDealBase::CKeyPressDealBase(Qt::Key key, QGraphicsScene *scene)
    : m_key(key)
    , m_scene(scene)
{
}

CKeyPressDealBase::~CKeyPressDealBase()
{
}

/**
 * @brief CKeyPressDealBase::getKey 获取注册的key
 * @return
 */
Qt::Key CKeyPressDealBase::getKey() const
{
    return m_key;
}

bool CKeyPressDealBase::dealEvent()
{
    CGraphicsScene *scene = qobject_cast<CGraphicsScene *>(m_scene);
    if (scene != nullptr) {
        CSceneBackgroundItem *item = dynamic_cast<CSceneBackgroundItem *>(scene->getCurrentFocusItem());
        if (item != nullptr) {
            return focusItemDeal(item, scene);
        } else {
            return false;
        }
    }
    return false;
}
