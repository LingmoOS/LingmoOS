// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_HOMEPAGE_H
#define UT_HOMEPAGE_H

#include "gtest/gtest.h"
class HomePage;

class UT_HomePage : public ::testing::Test
{
public:
    UT_HomePage();

    virtual void SetUp() override;
    virtual void TearDown() override;

protected:
    HomePage *m_homepage {nullptr};
};

#endif // UT_HOMEPAGE_H
