// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QApplication>
#include <QTimer>
/**
  添加Qt事件循环,兼容gtest.
 */
int runTest(QCoreApplication &app)
{
    int ret = 0;
    QTimer::singleShot(0, &app, [&app, &ret](){
        ret = RUN_ALL_TESTS();
        app.quit();
    });
    app.exec();
    return ret;
}

int main(int argc, char *argv[])
{
    // gerrit编译时没有显示器，需要指定环境变量
    qputenv("QT_QPA_PLATFORM", "offscreen");

    QApplication app(argc, argv);
    testing::InitGoogleTest(&argc, argv);

    return runTest(app);
}
