// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_Tabbar_H
#define UT_Tabbar_H
#include "gtest/gtest.h"
#include <QObject>
#include"../../src/controls/tabbar.h"


class UT_Tabbar: public QObject, public::testing::Test
{
    Q_OBJECT

public:
    UT_Tabbar();
};

#endif // UT_Tabbar_H
