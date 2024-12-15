// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_SIMPLELISTDELEGATE_H
#define UT_SIMPLELISTDELEGATE_H

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QTest>

#include"../stub.h"
#include "../../src/views/simplelistdelegate.h"
#include "../../src/views/simplelistmodel.h"

class Ut_SimpleListDelegate : public ::testing::Test
{
public:
    Ut_SimpleListDelegate();

    SimpleListDelegate *m_simpleListDelegate;
    SimpleListModel *model;
protected:
    void SetUp();
    void TearDown();
};

#endif // UT_SIMPLELISTDELEGATE_H
