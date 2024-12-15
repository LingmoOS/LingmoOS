// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEST_FINDBAR_H
#define TEST_FINDBAR_H
#include "gtest/gtest.h"
#include <QObject>

class test_findbar : public QObject, public::testing::Test
{
public:
    test_findbar();
};

#endif // TEST_FINDBAR_H
