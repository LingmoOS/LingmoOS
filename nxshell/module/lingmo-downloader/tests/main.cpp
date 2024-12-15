// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gtest/gtest.h"
#include <QApplication>
#include <QDir>
#include <iostream>
#include "dbinstance.h"
#include "settings.h"
#include "dlmapplication.h"

#if defined(CMAKE_SAFETYTEST_ARG_ON)
#include <sanitizer/asan_interface.h>
#endif

int main(int argc, char *argv[])
{
    Settings::getInstance()->initWidget();

    //qputenv("QT_QPA_PLATFORM", "offscreen");
    DlmApplication a(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    qDebug() << "start run test: " << QDateTime::currentDateTime();
    int ret = RUN_ALL_TESTS();
    qDebug() << "end run test" << QDateTime::currentDateTime();

#if defined(CMAKE_SAFETYTEST_ARG_ON)
    __sanitizer_set_report_path("asan.log");
#endif
    return ret;
}
