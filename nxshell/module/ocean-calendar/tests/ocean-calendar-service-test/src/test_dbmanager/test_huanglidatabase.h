// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_HUANGLIDATABASE_H
#define TEST_HUANGLIDATABASE_H

#include "dbmanager/huanglidatabase.h"
#include "gtest/gtest.h"
#include <QObject>

class test_huanglidatabase : public QObject, public::testing::Test
{
public:
    test_huanglidatabase();
    ~test_huanglidatabase();
protected:
    HuangLiDataBase *hlDb = nullptr;
};

#endif // TEST_HUANGLIDATABASE_H
