// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cscenetabkeydeal.h"

#include "graphicsItem/cscenebackgrounditem.h"
#include "cgraphicsscene.h"

#include <QDebug>
#include <QGraphicsView>

CSceneTabKeyDeal::CSceneTabKeyDeal(QGraphicsScene *scene)
    : CKeyPressDealBase(Qt::Key_Tab, scene)
{
}

bool CSceneTabKeyDeal::focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene)
{
    CSceneBackgroundItem *nextItem = qobject_cast<CSceneBackgroundItem *>(item->setNextItemFocusAndGetNextItem());
    if (nextItem == nullptr) {
        scene->setCurrentFocusItem(nullptr);
        item->setItemFocus(false);
        return false;
    } else {
        CFocusItem *focusItem = nextItem->getFocusItem();
        //如果当前焦点显示不为背景则定位到当前焦点item位置
        if (focusItem->getItemType() != CFocusItem::CBACK) {
            QGraphicsView *view = scene->views().at(0);
            QPointF point(scene->width() / 2, focusItem->rect().y());
            view->centerOn(point);
        }
        scene->setCurrentFocusItem(nextItem);
        return true;
    }
}
