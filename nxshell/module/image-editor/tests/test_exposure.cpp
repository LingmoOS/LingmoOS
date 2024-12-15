// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//#include <QtTest/QtTest>
//#include <gtest/gtest.h>

//#include <time.h>

//#include "libvisualresult/src/exposure.h"


//#define EPSILON 0.00001

//TEST(exposure, exposure_bin_data)
//{
//    unsigned char list[6 * 6 * 3], test[6 * 6 * 3];
//    int exp = 100;

//    srand((int)time(0));

//    for (int i = 0; i < sizeof(list); i++) {
//        int temp = random() % 255;
//        list[i] = char(temp);
//        int test_tmp = list[i] * pow(2, exp / 100.0);
//        test[i] = test_tmp > 255 ? 255 : test_tmp;
//    }

//    exposure(list, 6, 6, exp);

//    for (int i = 0; i < sizeof(list); i++) {
//        ASSERT_TRUE(list[i] == test[i]);
//    }
//}
