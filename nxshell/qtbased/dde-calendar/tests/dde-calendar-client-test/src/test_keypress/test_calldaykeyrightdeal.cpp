// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_calldaykeyrightdeal.h"

#include "../third-party_stub/stub.h"
#include "gtest/gtest.h"
#include "view/cgraphicsscene.h"
#include "KeyPress/calldaykeyrightdeal.h"
#include "view/graphicsItem/cweekdaybackgrounditem.h"
#include "keypressstub.h"

test_CAllDayKeyRightDeal::test_CAllDayKeyRightDeal(QObject *parent)
    : QObject(parent)
{
}

TEST(RightHandle_test, test_CAllDayKeyRightDeal)
{
    KeyPressStub stub;
    CGraphicsScene *scene = new CGraphicsScene();
    CAllDayKeyRightDeal alldayRightDeal(scene);
    alldayRightDeal.dealEvent();
    delete scene;
}
