// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QDebug>
#include <QApplication>

// 首先要添加安全性测试的接口头文件
#if defined(CMAKE_SAFETYTEST_ARG_ON)
#include <sanitizer/asan_interface.h>
#endif

int main(int argc, char *argv[])
{
    qDebug() << "start libdeepin-font-manager test cases ..............";

    qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication application(argc, argv);

    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    qDebug() << "end libdeepin-font-manager test cases ..............";

    // 之后在程序函数主入口添加log输出的代码
#if defined(CMAKE_SAFETYTEST_ARG_ON)
    __sanitizer_set_report_path("asan.log");
#endif

    return ret;
}
