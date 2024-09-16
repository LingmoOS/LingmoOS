// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "keypressstub.h"

#include "view/graphicsItem/cweekdaybackgrounditem.h"
#include "view/cgraphicsscene.h"
#include "KeyPress/cscenetabkeydeal.h"

#include <QDebug>

bool itemFocus = false;
bool scene_activeSwitching = false;
bool WeekDayBackgroundItem_hasNextItem = false;
bool WeekDayBackgroundItem_showFocus = false;
QGraphicsItem *SceneCurrentItem = nullptr;
CFocusItem::CItemType focusItemType = CFocusItem::CBACK;
QDate itemDate = QDate::currentDate();

//bbbbbbbbbbbbb CFocusItem stub  bbbbbbbbbbbbbbbbbbbbbb
bool getItemFocus_stub()
{
    return itemFocus;
}

CFocusItem::CItemType getItemType_stub()
{
    return focusItemType;
}

//eeeeeeeeeeeeeeeeeeeeeee

//  CGraphicsScene stub
void setActiveSwitching_stub(bool activeSwitching)
{
    scene_activeSwitching = activeSwitching;
}

QGraphicsItem *getCurrentFocusItem_stub(void *obj)
{
    Q_UNUSED(obj)
    return SceneCurrentItem;
}

void signalSwitchView_stub(const QDate &focusDate, bool setItemFocus = false)
{
    Q_UNUSED(focusDate);
    Q_UNUSED(setItemFocus);
}

// CWeekDayBackgroundItem  stub
void setItemFocus(void *obj, bool isFocus)
{
    Q_UNUSED(obj)
    itemFocus = isFocus;
}

bool hasNextSubItem_stub(void *obj)
{
    Q_UNUSED(obj)
    return WeekDayBackgroundItem_hasNextItem;
}

bool showFocus_stub(void *obj)
{
    Q_UNUSED(obj)
    return WeekDayBackgroundItem_showFocus;
}

void initState_stub(void *obj) {
    Q_UNUSED(obj)}

QDate getDate_stub(void *obj)
{
    Q_UNUSED(obj)
    return itemDate;
}

// SceneTabHandle stub
//bool focusItemDeal_stub(CSceneBackgroundItem *item, CGraphicsScene *scene)
//{
//    Q_UNUSED(item)
//    Q_UNUSED(scene)
//    return false;
//}

KeyPressStub::KeyPressStub()
{
    SceneCurrentItem = new CWeekDayBackgroundItem();
    typedef void (*fptr)(CWeekDayBackgroundItem *, bool);
    fptr A_foo = (fptr)(&CWeekDayBackgroundItem::setItemFocus); //obtaining an address
    stub.set(ADDR(CFocusItem, getItemFocus), getItemFocus_stub);
    stub.set(ADDR(CFocusItem, getItemType), getItemType_stub);
    stub.set(A_foo, setItemFocus);
    stub.set(ADDR(CGraphicsScene, setActiveSwitching), setActiveSwitching_stub);
    stub.set(ADDR(CGraphicsScene, signalSwitchView), signalSwitchView_stub);
    stub.set(ADDR(CGraphicsScene, getCurrentFocusItem), getCurrentFocusItem_stub);
    stub.set(ADDR(CWeekDayBackgroundItem, hasNextSubItem), hasNextSubItem_stub);
    stub.set(ADDR(CWeekDayBackgroundItem, showFocus), showFocus_stub);
    stub.set(ADDR(CWeekDayBackgroundItem, initState), initState_stub);
    stub.set(ADDR(CWeekDayBackgroundItem, getDate), getDate_stub);
    //    typedef bool (*sceneTabKeyHandle)(CSceneTabKeyDeal *, CSceneBackgroundItem *, CGraphicsScene *);
    //    sceneTabKeyHandle SceneTabHandle = (sceneTabKeyHandle)(&CSceneTabKeyDeal::focusItemDeal);
    //    stub.set(SceneTabHandle, focusItemDeal_stub);
}

KeyPressStub::~KeyPressStub()
{
    delete SceneCurrentItem;
    SceneCurrentItem = nullptr;
}

Stub &KeyPressStub::getStub()
{
    return stub;
}
