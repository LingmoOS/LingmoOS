// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "gtest/gtest.h"
#include <QApplication>
#include "cusapplication.h"

#if defined(CMAKE_SAFETYTEST_ARG_ON)
#include <sanitizer/asan_interface.h>
#endif

int main(int argc, char *argv[])
{
//    qputenv("QT_QPA_PLATFORM", "offscreen");
    CusApplication a(argc, argv);

    testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();

#if defined(CMAKE_SAFETYTEST_ARG_ON)
    __sanitizer_set_report_path("asan.log");
#endif

    return ret;
}

