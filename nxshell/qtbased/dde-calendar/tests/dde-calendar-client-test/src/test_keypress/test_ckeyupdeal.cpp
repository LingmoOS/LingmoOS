// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_ckeyupdeal.h"

#include "../third-party_stub/stub.h"
#include "gtest/gtest.h"
#include "view/cgraphicsscene.h"
#include "KeyPress/ckeyupdeal.h"
#include "view/graphicsItem/cweekdaybackgrounditem.h"
#include "keypressstub.h"

test_CKeyUpDeal::test_CKeyUpDeal(QObject *parent)
    : QObject(parent)
{
}

TEST(UpHandle_test, test_CKeyUpDeal)
{
    KeyPressStub stub;
    CGraphicsScene *scene = new CGraphicsScene();
    CKeyUpDeal upDeal(scene);
    upDeal.dealEvent();
    delete scene;
}
