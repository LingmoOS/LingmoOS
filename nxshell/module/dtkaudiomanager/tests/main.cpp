// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QCoreApplication>
#include <QTimer>
#include <gtest/gtest.h>
#include <sanitizer/asan_interface.h>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);

    QTimer::singleShot(0, [] {
        int ret = RUN_ALL_TESTS();
        qApp->exit(ret);
    });
    return app.exec();
}
