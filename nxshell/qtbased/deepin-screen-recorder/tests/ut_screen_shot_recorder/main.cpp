// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QApplication>
#include <gtest/gtest.h>
#include <QDebug>
#include <QCameraInfo>
#include <QDBusMessage>
#include <QPixmap>
#include <QtDBus>
#ifdef ENABLE_TSAN_TOOL
#include <sanitizer/asan_interface.h>
#endif
#include "test_all_interfaces.h"
#include "stub.h"
#include "../../src/main_window.h"

static Stub globalStub;

QList<QCameraInfo> availableCameras_stub(void* obj, QCamera::Position position = QCamera::UnspecifiedPosition)
{
    return QList<QCameraInfo>();
}

int quit_stub(void* obj)
{
    return 0;
}

QDBusMessage callWithArgumentList_stub(void *obj, QDBus::CallMode mode, const QString &method, const QList<QVariant> &args)
{
    return QDBusMessage();
}

bool isNull_stub(void *obj)
{
    return false;
}

bool MainWindow_saveImg_stub(void *obj, const QPixmap &pix, const QString& fileName, const char* format = nullptr)
{
    return true;
}

ACCESS_PRIVATE_FUN(MainWindow, bool(const QPixmap&, const QString&, const char*), saveImg);

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    qDebug() << "start test cases ..............";
    globalStub.set(ADDR(QCameraInfo, availableCameras), availableCameras_stub);
    globalStub.set(ADDR(QCameraInfo, isNull), isNull_stub);
    globalStub.set(get_private_fun::MainWindowsaveImg(), MainWindow_saveImg_stub);
    globalStub.set(ADDR(QDBusInterface, callWithArgumentList), callWithArgumentList_stub);
    //testing::GTEST_FLAG(output) = "xml:./report/report.xml";
    testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
#ifdef ENABLE_TSAN_TOOL
    __sanitizer_set_report_path("./asan_ut_screen_shot_recorder.log");
#endif
    //system("export ASAN_OPTIONS=halt_on_error=0");
    qDebug() << "end test cases ..............";
    //return app.exec();
    return ret;
}
