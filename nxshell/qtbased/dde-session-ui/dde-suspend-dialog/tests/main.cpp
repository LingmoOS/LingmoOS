// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QApplication>
#include <QDebug>

#ifdef QT_DEBUG
#include <sanitizer/asan_interface.h>
#endif

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication app(argc,argv);
    qDebug() << "start dde-suspend-dialog test cases ..............";
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    qDebug() << "end dde-suspend-dialog test cases ..............";
    
#ifdef QT_DEBUG
    __sanitizer_set_report_path("asan_dde-suspend-dialog.log");
#endif

    return ret;
}
