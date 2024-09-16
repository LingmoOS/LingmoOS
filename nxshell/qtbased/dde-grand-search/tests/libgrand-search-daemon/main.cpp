// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#ifdef ENABLE_SANITIZER
#include <sanitizer/asan_interface.h>
#endif

#include <QCoreApplication>

#include <unistd.h>

static void noMessageHandler(QtMsgType type, const QMessageLogContext &context,
                                   const QString &message)
{
    return;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    testing::InitGoogleTest(&argc,argv);

    //不打印应用的输出日志
    qInstallMessageHandler(noMessageHandler);

    int ret = RUN_ALL_TESTS();

    // must be here
#ifdef ENABLE_SANITIZER
    __sanitizer_set_report_path("../../asan_dde-grand-search-daemon.log");
#endif
    return ret;
}
