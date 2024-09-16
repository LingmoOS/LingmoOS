// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_H
#define TEST_H

#include <gtest/gtest.h>

class DTest : public ::testing::Test
{
};

template<typename T>
class DTestWithParam : public ::testing::TestWithParam<T>
{
};
#endif // TEST_H

