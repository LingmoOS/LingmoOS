// SPDX-FileCopyrightText: 2017 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ut_dutil.h"
#include <QGuiApplication>

#ifdef QT_DEBUG
#include <sanitizer/asan_interface.h>
#endif

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("tests");
    app.setOrganizationName("deepin");
#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
    DTimedLoop loop;
#endif

    testing::InitGoogleTest(&argc, argv);
    int retVal = RUN_ALL_TESTS();

#ifdef QT_DEBUG
    __sanitizer_set_report_path("asan.log");
#endif

#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
    return loop.exec(0, "main execution") + retVal;
#else
    return 0;
#endif
}
