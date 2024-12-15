// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_Utils_H
#define UT_Utils_H

#include "gtest/gtest.h"
#include <QObject>

class Utils;
class UT_Utils : public QObject
    , public ::testing::Test
{
public:
    UT_Utils();
    virtual void SetUp() override;
    virtual void TearDown() override;
    Utils *utils;
};

#endif // UT_Utils_H
