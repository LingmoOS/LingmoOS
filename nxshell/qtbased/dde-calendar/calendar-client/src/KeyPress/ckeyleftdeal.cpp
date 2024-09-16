// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ckeyleftdeal.h"

#include "graphicsItem/cscenebackgrounditem.h"
#include "cgraphicsscene.h"

#include <QDebug>

CKeyLeftDeal::CKeyLeftDeal(QGraphicsScene *scene)
    : CKeyPressDealBase(Qt::Key_Left, scene)
{
}

bool CKeyLeftDeal::focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene)
{
    item->initState();
    if (item->getLeftItem() != nullptr) {
        scene->setCurrentFocusItem(item->getLeftItem());
        item->getLeftItem()->setItemFocus(true);
    } else {
        scene->setPrePage(item->getDate().addDays(-1));
    }
    return true;
}
