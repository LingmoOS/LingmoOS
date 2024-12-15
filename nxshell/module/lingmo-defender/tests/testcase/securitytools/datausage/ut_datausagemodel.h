// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_DATAUSAGE_MODEL_H
#define UT_DATAUSAGE_MODEL_H

#include "gtest/gtest.h"
#include <QObject>

class DataUsageModelTest : public QObject
    , public testing::Test

{
    Q_OBJECT
public:
    DataUsageModelTest();

protected:
    virtual void SetUp();
    virtual void TearDown();
};

#endif // UT_DATAUSAGE_MODEL_H
