// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef KEYPRESSSTUB_H
#define KEYPRESSSTUB_H

#include "../third-party_stub/stub.h"
#include "view/graphicsItem/cfocusitem.h"

#include <QObject>

class QGraphicsItem;
class KeyPressStub
{
public:
    KeyPressStub();
    ~KeyPressStub();
    Stub &getStub();

private:
    Stub stub;
};

extern bool itemFocus;
extern bool scene_activeSwitching;
extern bool WeekDayBackgroundItem_hasNextItem;
extern bool WeekDayBackgroundItem_showFocus;
extern QGraphicsItem *SceneCurrentItem;
extern CFocusItem::CItemType focusItemType;
extern QDate itemDate;

#endif // KEYPRESSSTUB_H
