// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_ckeyleftdeal.h"

#include "../third-party_stub/stub.h"
#include "gtest/gtest.h"
#include "view/cgraphicsscene.h"
#include "KeyPress/ckeyleftdeal.h"
#include "view/graphicsItem/cweekdaybackgrounditem.h"
#include "keypressstub.h"

test_CKeyLeftDeal::test_CKeyLeftDeal(QObject *parent)
    : QObject(parent)
{
}

TEST(LeftHandle_test, test_CKeyLeftDeal)
{
    KeyPressStub stub;
    CGraphicsScene *scene = new CGraphicsScene();
    CKeyLeftDeal leftDeal(scene);
    leftDeal.dealEvent();
    delete scene;
}
