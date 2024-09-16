// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ckeydowndeal.h"

#include "graphicsItem/cscenebackgrounditem.h"
#include "cgraphicsscene.h"

#include <QDebug>

CKeyDownDeal::CKeyDownDeal(QGraphicsScene *scene)
    : CKeyPressDealBase(Qt::Key_Down, scene)
{
}

bool CKeyDownDeal::focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene)
{
    item->initState();
    if (item->getDownItem() != nullptr) {
        scene->setCurrentFocusItem(item->getDownItem());
        item->getDownItem()->setItemFocus(true);
    } else {
        scene->setNextPage(item->getDate().addDays(7));
    }
    return true;
}
