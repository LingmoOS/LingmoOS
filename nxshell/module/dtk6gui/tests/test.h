// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
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

template<typename T>
bool waitforSpy(T &spy, int timeout = 5000)
{
    int times = timeout / 10 + 1;
    while(!spy.wait(10)) {
        if (--times <= 1)
            break;
    }
    return times > 1;
}

#endif // TEST_H
