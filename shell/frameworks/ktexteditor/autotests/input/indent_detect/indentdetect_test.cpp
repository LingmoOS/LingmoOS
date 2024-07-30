/*
    SPDX-FileCopyrightText: 2022 Waqar Ahmed <waqar.17a@gmail.com>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "indentdetect_test.h"
#include "kateconfig.h"
#include "katedocument.h"
#include "kateindentdetecter.cpp" // HACK
#include "kateindentdetecter.h"

#include <QDebug>
#include <QTest>

using namespace KTextEditor;

QTEST_MAIN(IndentDetectTest)

void IndentDetectTest::test_data()
{
    QString dir = QLatin1String(TEST_DATA_DIR) + QStringLiteral("/indent_detect/");

    // The document
    QTest::addColumn<QString>("docPath");
    QTest::addColumn<bool>("expected_useTabs");
    QTest::addColumn<int>("expected_indentWidth");

    QTest::addRow("2space") << QString(dir + QStringLiteral("2space.js")) << false << 2;
    QTest::addRow("4space") << QString(dir + QStringLiteral("4space.cpp")) << false << 4;
    QTest::addRow("tabs") << QString(dir + QStringLiteral("tab.c")) << true << 4;
    QTest::addRow("this_file") << QString::fromUtf8(__FILE__) << false << 4;
}

void IndentDetectTest::test()
{
    QFETCH(QString, docPath);
    QFETCH(bool, expected_useTabs);
    QFETCH(int, expected_indentWidth);

    DocumentPrivate doc;
    doc.config()->setAutoDetectIndent(true);
    doc.openUrl(QUrl::fromLocalFile(docPath));
    QVERIFY(!doc.isEmpty());

    int actualIndentWidth = doc.config()->indentationWidth();
    bool actual_useTabs = !doc.config()->replaceTabsDyn();

    QCOMPARE(actual_useTabs, expected_useTabs);
    if (!expected_useTabs) {
        QCOMPARE(actualIndentWidth, expected_indentWidth);
    }
}

void IndentDetectTest::bench()
{
    // kate document is fairly large, lets use it for benchmarking
    const QString file = QString::fromLatin1(TEST_DATA_DIR) + QStringLiteral("../../src/document/katedocument.cpp");
    DocumentPrivate doc;
    doc.openUrl(QUrl::fromLocalFile(file));
    QVERIFY(!doc.isEmpty());
    QVERIFY(QUrl::fromLocalFile(file).isValid());

    QBENCHMARK {
        KateIndentDetecter d(&doc);
        d.detect(doc.config()->indentationWidth(), doc.config()->replaceTabsDyn());
    }
}
