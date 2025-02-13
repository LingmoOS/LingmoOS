/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZhangYong <zhangyong@uniontech.com>
 *
 * Maintainer: ZhangYong <ZhangYong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <QtTest>
#include <QCoreApplication>
#include <QTest>

// add necessary includes here
#include <QLineEdit>

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#ifndef SYSTEM_MIPS
#include <sanitizer/asan_interface.h>
#endif

#include <DApplication>

#define private public
#define protected public

using namespace Dtk::Core;
using namespace Dtk::Widget;

#define QMYTEST_MAIN(TestObject) \
    QT_BEGIN_NAMESPACE \
    QTEST_ADD_GPU_BLACKLIST_SUPPORT_DEFS \
    QT_END_NAMESPACE \
    int main(int argc, char *argv[]) \
    { \
        DApplication *dAppNew = new DApplication(argc, argv); \
        dAppNew->setAttribute(Qt::AA_UseHighDpiPixmaps); \
        dAppNew->setOrganizationName("deepin"); \
        dAppNew->setApplicationName("deepin-ocr"); \
        dAppNew->loadTranslator(QList<QLocale>() << QLocale::system()); \
        QTEST_DISABLE_KEYPAD_NAVIGATION \
        QTEST_ADD_GPU_BLACKLIST_SUPPORT \
        TestObject tc; \
        QTEST_SET_MAIN_SOURCE_PATH \
        return QTest::qExec(&tc, argc, argv); \
    } \

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

    void testQString_data();
    void testQString();

    void testFloat_data();
    void testFloat();

    void testGui_data();
    void testGui();
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
    testing::GTEST_FLAG(output) = "xml:./report/report_deepin-ocr.xml";
    int argc = 1;
    const auto arg0 = "dummy";
    char *argv0 = const_cast<char *>(arg0);

    testing::InitGoogleTest(&argc, &argv0);
    int ret = RUN_ALL_TESTS();
    Q_UNUSED(ret)

    // 内存分析屏蔽mips
#ifndef SYSTEM_MIPS
    __sanitizer_set_report_path("asan_deepin-ocr.log");
#endif
}

void QTestMain::testQString_data()
{
    QTest::addColumn<QString>("string");
    QTest::addColumn<QString>("result");
    QTest::newRow("lower") << "hello" << "HELLO";
    QTest::newRow("mix") << "heLLo" << "HELLO";
    QTest::newRow("upper") << "HELLO" << "HELLO";
}

void QTestMain::testQString()
{
    QFETCH(QString, string);
    QFETCH(QString, result);
    QCOMPARE(string.toUpper(), result);
    QBENCHMARK{
        Q_UNUSED(string.toUpper())
    }
}

void QTestMain::testFloat_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<double>("score");
    QTest::newRow("zhangsan") << "zhangsan" << 60.0;
    QTest::newRow("lisi") << "lisi" << 56.0;
    QTest::newRow("wanger") << "wanger" << 48.0;
}

void QTestMain::testFloat()
{
    QFETCH(QString, name);
    QFETCH(double, score);
    QVERIFY2(score >= 30.0, name.toLocal8Bit() + " score: " + QString::number(score).toLocal8Bit());
}

void QTestMain::testGui_data()
{
    QTest::addColumn<QTestEventList>("event");
    QTest::addColumn<QString>("result");

    QTestEventList list1;
    list1.addKeyClicks("hello world");
    QTest::newRow("item 0 ") << list1 << QString("hello world");

    QTestEventList list2;
    list2.addKeyClicks("abs0");
    list2.addKeyClick(Qt::Key_Backspace);
    QTest::newRow("item 1") << list2 << QString("abs");
}

void QTestMain::testGui()
{
//    QFETCH(QTestEventList, event);
//    QFETCH(QString, result);

//    QLineEdit lineEdit;
//    event.simulate(&lineEdit);
//    QCOMPARE(lineEdit.text(), result);
}

QMYTEST_MAIN(QTestMain)

#include "test_qtestmain.moc"
