// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <gmock/gmock-matchers.h>
#include <QApplication>
#include <DApplication>

#if defined(CMAKE_SAFETYTEST_ARG_ON)
#include <sanitizer/asan_interface.h>
#endif

DWIDGET_USE_NAMESPACE

//#include <QTest>

int main(int argc, char *argv[])

{
    qputenv("QT_QPA_PLATFORM","offscreen");
    DApplication app(argc, argv);

    testing::InitGoogleTest(&argc, argv);

    auto c = RUN_ALL_TESTS();

    #if defined(CMAKE_SAFETYTEST_ARG_ON)
    __sanitizer_set_report_path("asan.log");
    #endif

   return c;
}
