// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include <QGuiApplication>
#include <QQuickStyle>
#include <QTimer>

int runTest(QGuiApplication &app)
{
    int ret = 0;
    QTimer::singleShot(0, &app, [&app, &ret]() {
        ret = RUN_ALL_TESTS();
        app.quit();
    });
    app.exec();
    return ret;
}

int main(int argc, char *argv[])
{
    // 编译时没有显示器，需要指定环境变量
    if (qEnvironmentVariable("DISPLAY").isEmpty())
        qputenv("QT_QPA_PLATFORM", "offscreen");

    QQuickStyle::setStyle("Chameleon");

    QGuiApplication app(argc, argv);
    app.setApplicationName(BIN_NAME);
    ::testing::InitGoogleTest(&argc, argv);

    return runTest(app);
}
