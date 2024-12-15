// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QApplication>
#include <QDebug>
#include <QTimer>

#include <gtest/gtest.h>
#ifdef QT_DEBUG
#include <sanitizer/asan_interface.h>
#endif

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication app(argc,argv);
    int ret = 0;
    QTimer::singleShot(0, [ & ] {
        qDebug() << "start ocean-welcome test cases ..............";
        ::testing::InitGoogleTest(&argc, argv);
        ret = RUN_ALL_TESTS();
        qDebug() << "end ocean-welcome test cases ..............";
    });
    app.exec();
    
#ifdef QT_DEBUG
    __sanitizer_set_report_path("asan_ocean-welcome.log");
#endif

    return ret;
}
