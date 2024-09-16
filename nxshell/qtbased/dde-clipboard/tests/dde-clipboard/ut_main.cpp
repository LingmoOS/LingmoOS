// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QApplication>
#include <QDebug>

#include <DLog>

#ifdef QT_DEBUG
#include <sanitizer/asan_interface.h>
#endif

DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    // gerrit编译时没有显示器，需要指定环境变量
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication app(argc,argv);

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    qDebug() << "start dde-clipboard test cases ..............";
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    qDebug() << "end dde-clipboard test cases ..............";
#ifdef QT_DEBUG
    __sanitizer_set_report_path("asan_board.log");
#endif
    return ret;
}
