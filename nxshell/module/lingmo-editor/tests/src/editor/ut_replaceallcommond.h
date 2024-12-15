// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEST_REPLACEALLCOMMOND_H
#define TEST_REPLACEALLCOMMOND_H


#include "gtest/gtest.h"
#include <QObject>


class test_replaceallcommond: public QObject, public::testing::Test
{
        Q_OBJECT
public:
    test_replaceallcommond();
};


#endif // TEST_REPLACEALLCOMMOND_H
