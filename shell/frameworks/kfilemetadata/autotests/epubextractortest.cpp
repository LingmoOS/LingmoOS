/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "epubextractortest.h"
#include "simpleextractionresult.h"
#include "indexerextractortestsconfig.h"
#include "extractors/epubextractor.h"
#include "mimeutils.h"

#include <QTest>
#include <QMimeDatabase>
#include <QTimeZone>

using namespace KFileMetaData;

QString EPubExtractorTest::testFilePath(const QString& fileName) const
{
    return QLatin1String(INDEXER_TESTS_SAMPLE_FILES_PATH) + QLatin1Char('/') + fileName;
}

void EPubExtractorTest::testNoExtraction()
{
    EPubExtractor plugin{this};

    QString fileName = testFilePath(QStringLiteral("test.epub"));
    QMimeDatabase mimeDb;
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();
    QVERIFY(plugin.mimetypes().contains(mimeType));

    SimpleExtractionResult result(fileName, mimeType, ExtractionResult::ExtractNothing);
    plugin.extract(&result);

    QCOMPARE(result.types().size(), 1);
    QCOMPARE(result.types().constFirst(), Type::Document);
    QCOMPARE(result.properties().size(), 0);
}

void EPubExtractorTest::test()
{
    EPubExtractor plugin{this};

    QString fileName = testFilePath(QStringLiteral("test.epub"));
    QMimeDatabase mimeDb;
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();
    QVERIFY(plugin.mimetypes().contains(mimeType));

    SimpleExtractionResult result(fileName, mimeType);
    plugin.extract(&result);

    QCOMPARE(result.types().size(), 1);
    QCOMPARE(result.types().constFirst(), Type::Document);

    // We're doing a contains instead of an exact check cause the epub file contains
    // a ton of css and other garbage.
    QVERIFY(result.text().contains(QStringLiteral("This is a sample PDF file for KFileMetaData.")));
    QCOMPARE(result.properties().value(Property::Author), QVariant(QStringLiteral("Happy Man")));
    QCOMPARE(result.properties().value(Property::Publisher), QVariant(QStringLiteral("Happy Publisher")));
    QCOMPARE(result.properties().value(Property::Title), QVariant(QStringLiteral("The Big Brown Bear")));
    QCOMPARE(result.properties().value(Property::Subject), QVariant(QStringLiteral("Baloo KFileMetaData")));
    QCOMPARE(result.properties().value(Property::Description), QVariant(QStringLiteral("Honey")));

    QDateTime dt(QDate(2014, 1, 1), QTime(1, 1, 1), QTimeZone::UTC);
    QCOMPARE(result.properties().value(Property::CreationDate), QVariant(dt));
    QCOMPARE(result.properties().value(Property::ReleaseYear), QVariant(2014));

    QCOMPARE(result.properties().size(), 7);
}

void EPubExtractorTest::testRepeated()
{
    EPubExtractor plugin{this};

    QString fileName = testFilePath(QStringLiteral("test_repeated.epub"));
    QMimeDatabase mimeDb;
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();
    QVERIFY(plugin.mimetypes().contains(mimeType));

    SimpleExtractionResult result(fileName, mimeType);
    plugin.extract(&result);

    QCOMPARE(result.types().size(), 1);
    QCOMPARE(result.types().constFirst(), Type::Document);

    QCOMPARE(result.properties().value(Property::Author), QVariant(QStringLiteral("Happy Man")));
    QCOMPARE(result.properties().value(Property::Publisher), QVariant(QStringLiteral("Happy Publisher")));
    QCOMPARE(result.properties().value(Property::Title), QVariant(QStringLiteral("The Big Brown Bear")));
    QCOMPARE(result.properties().values(Property::Subject),
        QVariantList({QStringLiteral("Test with repeated keys"), QStringLiteral("Baloo KFileMetaData")})
    );
    QCOMPARE(result.properties().value(Property::Description), QVariant(QStringLiteral("Honey")));

    QDateTime dt(QDate(2012, 1, 1), QTime(0, 0, 0), QTimeZone::UTC);
    QCOMPARE(result.properties().value(Property::CreationDate), QVariant(dt));
    QCOMPARE(result.properties().value(Property::ReleaseYear), QVariant(2012));

    QCOMPARE(result.properties().size(), 9);
}

void EPubExtractorTest::testMetaDataOnly()
{
    EPubExtractor plugin{this};

    SimpleExtractionResult result(testFilePath("test.epub"), "application/epub+zip", ExtractionResult::ExtractMetaData);
    plugin.extract(&result);

    QVERIFY(!result.types().isEmpty());
    QVERIFY(!result.properties().isEmpty());
    QVERIFY(result.text().isEmpty());
}


QTEST_GUILESS_MAIN(EPubExtractorTest)

#include "moc_epubextractortest.cpp"
