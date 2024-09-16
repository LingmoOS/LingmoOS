// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_PROLISTVIEW_H
#define UT_PROLISTVIEW_H

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QTest>
#include <QMenu>

#include"../stub.h"
#include "../../src/dsettings.h"

class Ut_ProListView : public ::testing::Test
{
public:
    Ut_ProListView();
};

#endif // UT_PROLISTVIEW_H
