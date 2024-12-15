// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "calldaykeyleftdeal.h"

#include "graphicsItem/cweekdaybackgrounditem.h"
#include "cgraphicsscene.h"

CAllDayKeyLeftDeal::CAllDayKeyLeftDeal(QGraphicsScene *scene)
    : CKeyPressDealBase(Qt::Key_Left, scene)
{
}

bool CAllDayKeyLeftDeal::focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene)
{
    CWeekDayBackgroundItem *backgroundItem = dynamic_cast<CWeekDayBackgroundItem *>(item);
    backgroundItem->initState();
    //如果是第一个则切换时间
    scene->setActiveSwitching(true);
    if (backgroundItem->getLeftItem() == nullptr) {
        scene->setPrePage(item->getDate().addDays(-1), true);
    } else {
        scene->signalSwitchView(backgroundItem->getDate().addDays(-1), true);
    }
    return true;
}
