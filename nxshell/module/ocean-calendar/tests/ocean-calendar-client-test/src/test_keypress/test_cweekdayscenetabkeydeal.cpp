// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_cweekdayscenetabkeydeal.h"

#include "../third-party_stub/stub.h"
#include "gtest/gtest.h"
#include "view/cgraphicsscene.h"
#include "KeyPress/cweekdayscenetabkeydeal.h"
#include "view/graphicsItem/cweekdaybackgrounditem.h"
#include "keypressstub.h"

test_CWeekDaySceneTabKeyDeal::test_CWeekDaySceneTabKeyDeal(QObject *parent)
    : QObject(parent)
{
}

TEST(WeekDaySceneTabHandle_test, test_CWeekDaySceneTabKeyDeal)
{
    KeyPressStub stub;
    CGraphicsScene *scene = new CGraphicsScene();
    CWeekDaySceneTabKeyDeal weektabDeal(scene);
    weektabDeal.dealEvent();
    delete scene;
}
