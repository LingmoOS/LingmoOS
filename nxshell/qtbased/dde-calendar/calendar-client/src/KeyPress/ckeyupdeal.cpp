// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ckeyupdeal.h"

#include "graphicsItem/cscenebackgrounditem.h"
#include "cgraphicsscene.h"

#include <QDebug>

CKeyUpDeal::CKeyUpDeal(QGraphicsScene *scene)
    : CKeyPressDealBase(Qt::Key_Up, scene)
{
}

bool CKeyUpDeal::focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene)
{
    item->initState();
    if (item->getUpItem() != nullptr) {
        scene->setCurrentFocusItem(item->getUpItem());
        item->getUpItem()->setItemFocus(true);
    } else {
        scene->setPrePage(item->getDate().addDays(-7));
    }
    return true;
}
