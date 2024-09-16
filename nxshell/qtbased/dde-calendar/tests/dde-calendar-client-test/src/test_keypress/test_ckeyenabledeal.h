// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_CKEYENABLEDEAL_H
#define TEST_CKEYENABLEDEAL_H

#include "keypressstub.h"
#include "view/cgraphicsscene.h"
#include "gtest/gtest.h"
#include "KeyPress/ckeyenabledeal.h"

#include <QObject>

class test_CKeyEnableDeal : public QObject
    , public ::testing::Test
{
    Q_OBJECT
public:
    test_CKeyEnableDeal();
    void SetUp();
    void TearDown();

public:
    KeyPressStub stub;
    QSharedPointer<CGraphicsScene> scene;
    QSharedPointer<QGraphicsView> view;
    QSharedPointer<CKeyEnableDeal> enableDeal;
};

extern CFocusItem *focusItem;

#endif // TEST_CKEYENABLEDEAL_H