// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEST_INSERTBLOCKBYTEXTCOMMOND_H
#define TEST_INSERTBLOCKBYTEXTCOMMOND_H


#include "gtest/gtest.h"
#include <QObject>
#include <QClipboard>

class test_insertblockbytextcommond: public QObject, public::testing::Test
{
        Q_OBJECT
public:
    test_insertblockbytextcommond();
};


#endif // TEST_INSERTBLOCKBYTEXTCOMMOND_H
