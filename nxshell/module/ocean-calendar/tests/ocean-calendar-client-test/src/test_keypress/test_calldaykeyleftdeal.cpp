// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_calldaykeyleftdeal.h"

#include "../third-party_stub/stub.h"
#include "gtest/gtest.h"
#include "view/cgraphicsscene.h"
#include "KeyPress/calldaykeyleftdeal.h"
#include "view/graphicsItem/cweekdaybackgrounditem.h"
#include "keypressstub.h"

test_CAllDayKeyLeftDeal::test_CAllDayKeyLeftDeal(QObject *parent)
    : QObject(parent)
{
}

TEST(LeftHandle_test, test_CAllDayKeyLeftDeal)
{
    KeyPressStub stub;
    CGraphicsScene *scene = new CGraphicsScene();
    CAllDayKeyLeftDeal alldayLeftDeal(scene);
    alldayLeftDeal.dealEvent();
    delete scene;
}
