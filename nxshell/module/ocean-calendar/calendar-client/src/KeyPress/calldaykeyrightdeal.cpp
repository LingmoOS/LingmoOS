// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "calldaykeyrightdeal.h"

#include "graphicsItem/cweekdaybackgrounditem.h"
#include "cgraphicsscene.h"

CAllDayKeyRightDeal::CAllDayKeyRightDeal(QGraphicsScene *scene)
    : CKeyPressDealBase(Qt::Key_Right, scene)
{
}

bool CAllDayKeyRightDeal::focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene)
{
    CWeekDayBackgroundItem *backgroundItem = dynamic_cast<CWeekDayBackgroundItem *>(item);
    backgroundItem->initState();
    //如果没有下一个则切换时间
    scene->setActiveSwitching(true);
    if (backgroundItem->getRightItem() == nullptr) {
        scene->setNextPage(item->getDate().addDays(1), true);
    } else {
        scene->signalSwitchView(backgroundItem->getDate().addDays(1), true);
    }
    return true;
}
