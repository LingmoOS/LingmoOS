// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QApplication>
#include <QDebug>

#include <gtest/gtest.h>
#ifdef QT_DEBUG
#include <sanitizer/asan_interface.h>
#endif

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication app(argc,argv);
    qDebug() << "start ocean-bluetooth-dialog test cases ..............";
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    qDebug() << "end ocean-bluetooth-dialog test cases ..............";

#ifdef QT_DEBUG
    __sanitizer_set_report_path("asan_ocean-bluetooth-dialog.log");
#endif

    return ret;
}
