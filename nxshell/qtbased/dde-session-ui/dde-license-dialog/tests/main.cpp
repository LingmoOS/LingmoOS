// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QApplication>
#include <QDebug>
#include <QTimer>

#ifdef QT_DEBUG
#include <sanitizer/asan_interface.h>
#endif

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication app(argc,argv);
    int ret = 0;
    //Content创建了线程,需要App的aboutToQuit信号去停止线程,否则进程不停止.
    //但是不执行app.exec()就无法收到aboutToQuit信号,所以将test放在事件循环中运行.
    QTimer::singleShot(0, [ & ] {
        qDebug() << "start dde-license-dialog test cases ..............";
        ::testing::InitGoogleTest(&argc, argv);
        ret = RUN_ALL_TESTS();
        qDebug() << "end dde-license-dialog test cases ..............";
    });
    app.exec();

#ifdef QT_DEBUG
    __sanitizer_set_report_path("asan_dde-license-dialog.log");
#endif

    return ret;
}
