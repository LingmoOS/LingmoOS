/*
    SPDX-FileCopyrightText: 2022 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "fb2extractortest.h"
#include "extractors/fb2extractor.h"
#include "indexerextractortestsconfig.h"
#include "mimeutils.h"
#include "simpleextractionresult.h"

#include <QMimeDatabase>
#include <QTest>
#include <QTimeZone>

using namespace KFileMetaData;

QString Fb2ExtractorTest::testFilePath(const QString &fileName) const
{
    return QLatin1String(INDEXER_TESTS_SAMPLE_FILES_PATH) + QLatin1Char('/') + fileName;
}

void Fb2ExtractorTest::testNoExtraction_data()
{
    QTest::addColumn<QString>("fileName");

    QTest::addRow("uncompressed") << QStringLiteral("test.fb2");
    QTest::addRow("compressed") << QStringLiteral("test.fb2.zip");
}

void Fb2ExtractorTest::testNoExtraction()
{
    QFETCH(QString, fileName);

    Fb2Extractor plugin{this};

    const QString path = testFilePath(fileName);
    QMimeDatabase mimeDb;
    QString mimeType = MimeUtils::strictMimeType(path, mimeDb).name();
    QVERIFY(plugin.mimetypes().contains(mimeType));

    SimpleExtractionResult result(path, mimeType, ExtractionResult::ExtractNothing);
    plugin.extract(&result);

    QCOMPARE(result.types().size(), 1);
    QCOMPARE(result.types().constFirst(), Type::Document);
    QCOMPARE(result.properties().size(), 0);
}

void Fb2ExtractorTest::test_data()
{
    QTest::addColumn<QString>("fileName");

    QTest::addRow("uncompressed") << QStringLiteral("test.fb2");
    QTest::addRow("compressed") << QStringLiteral("test.fb2.zip");
}

void Fb2ExtractorTest::test()
{
    QFETCH(QString, fileName);

    Fb2Extractor plugin{this};

    const QString path = testFilePath(fileName);
    QMimeDatabase mimeDb;
    QString mimeType = MimeUtils::strictMimeType(path, mimeDb).name();
    QVERIFY(plugin.mimetypes().contains(mimeType));

    SimpleExtractionResult result(path, mimeType);
    plugin.extract(&result);

    QCOMPARE(result.types().size(), 1);
    QCOMPARE(result.types().constFirst(), Type::Document);

    QCOMPARE(result.text(), QStringLiteral("This is a sample PDF file for KFileMetaData. "));
    QCOMPARE(result.properties().value(Property::Author), QVariant(QStringLiteral("Happy Man")));
    QCOMPARE(result.properties().value(Property::Publisher), QVariant(QStringLiteral("Happy Publisher")));
    QCOMPARE(result.properties().value(Property::Title), QVariant(QStringLiteral("The Big Brown Bear")));
    QCOMPARE(result.properties().value(Property::Description), QVariant(QStringLiteral("Honey")));
    QCOMPARE(result.properties().value(Property::Language), QVariant(QStringLiteral("en")));
    QCOMPARE(result.properties().value(Property::Genre), QVariant(QStringLiteral("antique")));
    QCOMPARE(result.properties().value(Property::Generator), QVariant(QStringLiteral("calibre 4.99.4")));

    QDateTime dt(QDate(2014, 1, 1), QTime(0, 0, 0), QTimeZone::UTC);
    QCOMPARE(result.properties().value(Property::CreationDate), QVariant(dt));
    QCOMPARE(result.properties().value(Property::ReleaseYear), QVariant(2014));

    QCOMPARE(result.properties().size(), 9);
}

void Fb2ExtractorTest::testMetaDataOnly_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<QString>("mimeType");

    QTest::addRow("uncompressed") << QStringLiteral("test.fb2") << QStringLiteral("application/x-fictionbook+xml");
    QTest::addRow("compressed") << QStringLiteral("test.fb2.zip") << QStringLiteral("application/x-zip-compressed-fb2");
}

void Fb2ExtractorTest::testMetaDataOnly()
{
    QFETCH(QString, fileName);
    QFETCH(QString, mimeType);

    Fb2Extractor plugin{this};

    SimpleExtractionResult result(testFilePath(fileName), mimeType, ExtractionResult::ExtractMetaData);
    plugin.extract(&result);

    QVERIFY(!result.types().isEmpty());
    QVERIFY(!result.properties().isEmpty());
    QVERIFY(result.text().isEmpty());
}

QTEST_GUILESS_MAIN(Fb2ExtractorTest)

#include "moc_fb2extractortest.cpp"
