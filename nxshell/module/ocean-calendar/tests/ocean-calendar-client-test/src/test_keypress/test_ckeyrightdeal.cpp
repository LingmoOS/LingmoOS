// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_ckeyrightdeal.h"

#include "../third-party_stub/stub.h"
#include "gtest/gtest.h"
#include "view/cgraphicsscene.h"
#include "KeyPress/ckeyrightdeal.h"
#include "view/graphicsItem/cweekdaybackgrounditem.h"
#include "keypressstub.h"

test_CKeyRightDeal::test_CKeyRightDeal(QObject *parent)
    : QObject(parent)
{
}

TEST(RightHandle_test, test_CKeyRightDeal)
{
    KeyPressStub stub;
    CGraphicsScene *scene = new CGraphicsScene();
    CKeyRightDeal rightDeal(scene);
    rightDeal.dealEvent();
    delete scene;
}
