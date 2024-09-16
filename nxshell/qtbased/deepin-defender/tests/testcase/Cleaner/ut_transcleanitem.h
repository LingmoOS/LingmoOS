// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_TRANSCLEANITEM_H
#define UT_TRANSCLEANITEM_H

#include "gtest/gtest.h"
#include <QObject>

class TransCleanItemTest : public QObject
    , public testing::Test

{
    Q_OBJECT
public:
    TransCleanItemTest();

protected:
    virtual void SetUp();
    virtual void TearDown();
};

#endif // UT_TRANSCLEANITEM_H
