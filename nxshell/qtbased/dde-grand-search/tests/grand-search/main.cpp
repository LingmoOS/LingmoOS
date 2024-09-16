// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#ifdef ENABLE_SANITIZER
#include <sanitizer/asan_interface.h>
#endif

#include <DApplication>
#include <QAccessible>

#include <unistd.h>

static void noMessageHandler(QtMsgType type, const QMessageLogContext &context,
                                   const QString &message)
{
    return;
}

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    DApplication app(argc, argv);

    testing::InitGoogleTest(&argc,argv);

    //不打印应用的输出日志
    qInstallMessageHandler(noMessageHandler);

    int ret = RUN_ALL_TESTS();

    // 处理一些事件，释放deletelater
    app.processEvents();

    // Qt的Guipplication，再开启事件循环后会自动创建一个QAccessibleInterface，但没有释放。
    // 这里手动释放
    if (auto ifs = QAccessible::queryAccessibleInterface(&app)) {
        auto id = QAccessible::uniqueId(ifs);
        QAccessible::deleteAccessibleInterface(id);
    }

    QAccessible::cleanup();

    // must be here
#ifdef ENABLE_SANITIZER
    __sanitizer_set_report_path("../../asan_dde-grand-search.log");
#endif
    return ret;
}
