// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_SIMPLELISTMODEL_H
#define UT_SIMPLELISTMODEL_H

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QTest>

#include "../../src/dsettings.h"
#include "../../src/views/simplelistmodel.h"

class Ut_SimpleListModel : public ::testing::Test
{
public:
    Ut_SimpleListModel();
    SimpleListModel *m_simpleListModel;
protected:
    void SetUp();
    void TearDown();
};

#endif // UT_SIMPLELISTMODEL_H
