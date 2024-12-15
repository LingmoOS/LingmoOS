// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_cscenetabkeydeal.h"

#include "../third-party_stub/stub.h"
#include "gtest/gtest.h"
#include "view/cgraphicsscene.h"
#include "KeyPress/cscenetabkeydeal.h"
#include "view/graphicsItem/cweekdaybackgrounditem.h"
#include "keypressstub.h"

test_CSceneTabKeyDeal::test_CSceneTabKeyDeal(QObject *parent)
    : QObject(parent)
{
}

TEST(SceneTabHandle_test, test_CSceneTabKeyDeal)
{
    KeyPressStub stub;
    CGraphicsScene *scene = new CGraphicsScene();
    CSceneTabKeyDeal tabDeal(scene);
    tabDeal.dealEvent();
    delete scene;
}
