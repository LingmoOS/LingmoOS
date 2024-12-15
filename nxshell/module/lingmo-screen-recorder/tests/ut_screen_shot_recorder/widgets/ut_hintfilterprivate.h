// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QTest>

#define private public
#define protected public
#include "../../src/widgets/filter.h"

using namespace testing;
class HintFilterPrivateTest:public testing::Test{

public:
    HintFilter *m_hintFilter;
    HintFilterPrivate *m_hintFilterPrivate;
    virtual void SetUp() override{
    }

    virtual void TearDown() override{
    }
};



