// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_SCIENTIFICMODULE_H
#define UT_SCIENTIFICMODULE_H

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QTest>

#include "../../src/dsettings.h"
#include"../stub.h"

class Ut_ScientificModule : public ::testing::Test
{
public:
    Ut_ScientificModule();
};

#endif // UT_SCIENTIFICMODULE_H
