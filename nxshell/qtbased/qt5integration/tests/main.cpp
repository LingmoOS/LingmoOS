/*
 * SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */
#include <QApplication>
#include <gtest/gtest.h>

#ifdef QT_DEBUG
#include <sanitizer/asan_interface.h>
#endif

int main(int argc, char *argv[])
{
    // gerrit编译时没有显示器，需要指定环境变量
    qputenv("QT_QPA_PLATFORM", "offscreen");

    // 风格插件需要添加平台主题，用于创建图标引擎
    QByteArray platformThemePath;
#ifdef UT_PLATFORMPLUGIN_PATH
    platformThemePath = UT_PLATFORMPLUGIN_PATH;
    QApplication::setLibraryPaths(QApplication::libraryPaths() << QString(UT_PLATFORMPLUGIN_PATH).append("../"));
#endif
    if (!platformThemePath.isEmpty())
        qputenv("QT_QPA_PLATFORM_PLUGIN_PATH", platformThemePath);

    qputenv("QT_QPA_PLATFORMTHEME", "deepin");
    QApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();

#ifdef QT_DEBUG
    __sanitizer_set_report_path("asan.log");
#endif

    return ret;
}
