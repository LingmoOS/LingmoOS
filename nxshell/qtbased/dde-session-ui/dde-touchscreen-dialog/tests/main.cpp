// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QApplication>
#include <QDebug>

#ifdef QT_DEBUG
#include <sanitizer/asan_interface.h>
#endif

#include <gtest/gtest.h>

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication app(argc,argv);
    qDebug() << "start dde-touchscreen-dialog test cases ..............";
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    qDebug() << "end dde-touchscreen-dialog test cases ..............";
    
#ifdef QT_DEBUG
    __sanitizer_set_report_path("asan_dde-touchscreen-dialog.log");
#endif

    return ret;
}
