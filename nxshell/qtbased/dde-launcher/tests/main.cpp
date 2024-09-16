// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <DLog>

#include <QApplication>
#include <QDebug>

#ifdef SANITIZER_CHECK
#include <sanitizer/asan_interface.h>
#endif

#include <gtest/gtest.h>

DCORE_USE_NAMESPACE

int main(int argc, char **argv)
{
    // gerrit编译时没有显示器，需要指定环境变量
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication app(argc,argv);

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    qDebug() << "start dde-launcher test cases ..............";
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    qDebug() << "end dde-launcher test cases ..............";

#ifdef SANITIZER_CHECK
    __sanitizer_set_report_path("asan.log");
#endif
    return ret;
}
