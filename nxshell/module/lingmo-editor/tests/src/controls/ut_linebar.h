// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEST_LINEBAR_H
#define TEST_LINEBAR_H
#include "gtest/gtest.h"
#include <QObject>

class test_linebar : public QObject, public::testing::Test
{
public:
    test_linebar();
};

#endif // TEST_LINEBAR_H
