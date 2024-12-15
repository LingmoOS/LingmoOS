// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_ckeydowndeal.h"

#include "../third-party_stub/stub.h"
#include "gtest/gtest.h"
#include "view/cgraphicsscene.h"
#include "KeyPress/ckeydowndeal.h"
#include "view/graphicsItem/cweekdaybackgrounditem.h"
#include "keypressstub.h"

test_CKeyDownDeal::test_CKeyDownDeal(QObject *parent)
    : QObject(parent)
{
}

TEST(DownHandle_test, test_CKeyDownDeal)
{
    KeyPressStub stub;
    CGraphicsScene *scene = new CGraphicsScene();
    CKeyDownDeal downDeal(scene);
    downDeal.dealEvent();
    delete scene;
}
