// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include <qcoreapplication.h>
#include <qresource.h>
#include <qtimer.h>
#include <sanitizer/asan_interface.h>

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    QCoreApplication app(argc, argv);
    QTimer::singleShot(0, [] {
        int ret = RUN_ALL_TESTS();
        qApp->exit(ret);
    });
#ifdef QT_DEBUG
    __sanitizer_set_report_path("asan_demo.log");
#endif
    return app.exec();
}
