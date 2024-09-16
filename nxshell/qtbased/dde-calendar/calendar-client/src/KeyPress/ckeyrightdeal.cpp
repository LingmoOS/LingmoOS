// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ckeyrightdeal.h"

#include "graphicsItem/cscenebackgrounditem.h"
#include "cgraphicsscene.h"

#include <QDebug>

CKeyRightDeal::CKeyRightDeal(QGraphicsScene *scene)
    : CKeyPressDealBase(Qt::Key_Right, scene)
{
}

bool CKeyRightDeal::focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene)
{
    item->initState();
    if (item->getRightItem() != nullptr) {
        scene->setCurrentFocusItem(item->getRightItem());
        item->getRightItem()->setItemFocus(true);
    } else {
        scene->setNextPage(item->getDate().addDays(1));
    }
    return true;
}
