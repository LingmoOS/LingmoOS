#if 0
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QApplication>
#include <DLog>
#include <QTranslator>
#include <DApplicationSettings>
#include "application.h"
#include "controller/commandline.h"
#include "service/defaultimageviewer.h"
#include <QTimer>
using namespace Dtk::Core;
int main(int argc, char *argv[])
{

    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    Application::loadDXcbPlugin();
    Application a(argc, argv);
    a.setAttribute(Qt::AA_ForceRasterWidgets);
    testing::InitGoogleTest();
    if (!service::isDefaultImageViewer()) {
        qDebug() << "Set defaultImage viewer succeed:" << service::setDefaultImageViewer(true);
    } else {
        qDebug() << "Deepin Image Viewer is defaultImage!";
    }
    RUN_ALL_TESTS();

    return a.exec();
//  return  RUN_ALL_TESTS();
//return a.exec();

}
#else

// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtTest>
#include <QCoreApplication>

#include <DApplication>
#include <DLog>
#include <QTranslator>
#include <DApplicationSettings>
#include <QTimer>


DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#ifndef __mips__
#include <sanitizer/asan_interface.h>
#endif
#define QMYTEST_MAIN(TestObject) \
    QT_BEGIN_NAMESPACE \
    QTEST_ADD_GPU_BLACKLIST_SUPPORT_DEFS \
    QT_END_NAMESPACE \
    int main(int argc, char *argv[]) \
    { \
        testing::InitGoogleTest(&argc,argv); \
        DApplication *a = new DApplication(argc, argv); \
        a->setAttribute(Qt::AA_Use96Dpi, true); \
        QTEST_DISABLE_KEYPAD_NAVIGATION \
        QTEST_ADD_GPU_BLACKLIST_SUPPORT \
        TestObject tc; \
        QTEST_SET_MAIN_SOURCE_PATH \
        return QTest::qExec(&tc, argc, argv); \
    }

class QTestMain : public QObject
{
    Q_OBJECT

public:
    QTestMain();
    ~QTestMain();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testGTest();
};

QTestMain::QTestMain()
{

}

QTestMain::~QTestMain()
{

}

void QTestMain::initTestCase()
{
}

void QTestMain::cleanupTestCase()
{

}

void QTestMain::testGTest()
{
    int ret = RUN_ALL_TESTS();
#ifndef __mips__
    __sanitizer_set_report_path("asan.log");
#endif
    Q_UNUSED(ret)
}

QMYTEST_MAIN(QTestMain)

#include "main.moc"
#endif
