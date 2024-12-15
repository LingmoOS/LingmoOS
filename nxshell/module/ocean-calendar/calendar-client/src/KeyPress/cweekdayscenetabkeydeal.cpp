// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cweekdayscenetabkeydeal.h"

#include "graphicsItem/cweekdaybackgrounditem.h"
#include "cgraphicsscene.h"

CWeekDaySceneTabKeyDeal::CWeekDaySceneTabKeyDeal(QGraphicsScene *scene)
    : CSceneTabKeyDeal(scene)
{
}

bool CWeekDaySceneTabKeyDeal::focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene)
{
    CWeekDayBackgroundItem *focusItem = dynamic_cast<CWeekDayBackgroundItem *>(item);
    if (focusItem != nullptr) {
        //如果当前背景是焦点显示则切换到另一个视图
        if (focusItem->getItemFocus()) {
            focusItem->setItemFocus(false);
            scene->setActiveSwitching(true);
            scene->signalSwitchView(focusItem->getDate());
            return true;
        } else {
            //如果该背景上还有切换显示的日程标签
            if (focusItem->hasNextSubItem() || focusItem->showFocus()) {
                return CSceneTabKeyDeal::focusItemDeal(item, scene);
            } else {
                focusItem->initState();
                scene->setActiveSwitching(true);
                scene->signalSwitchView(focusItem->getDate());
                return true;
            }
        }
    }
    return false;
}
