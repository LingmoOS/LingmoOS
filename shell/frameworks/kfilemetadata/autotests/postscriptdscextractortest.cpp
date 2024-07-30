/*
    SPDX-FileCopyrightText: 2018 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "postscriptdscextractortest.h"
#include "simpleextractionresult.h"
#include "indexerextractortestsconfig.h"
#include "extractors/postscriptdscextractor.h"
#include "mimeutils.h"

#include <QTest>
#include <QTimeZone>

using namespace KFileMetaData;

QString PostscriptDscExtractorTest::testFilePath(const QString& fileName) const
{
    return QLatin1String(INDEXER_TESTS_SAMPLE_FILES_PATH) + QLatin1Char('/') + fileName;
}

void PostscriptDscExtractorTest::testNoExtraction()
{
    DscExtractor plugin{this};

    QString fileName = testFilePath(QStringLiteral("test.ps"));
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();
    QVERIFY(plugin.mimetypes().contains(mimeType));

    SimpleExtractionResult result(fileName, mimeType, ExtractionResult::ExtractNothing);
    plugin.extract(&result);

    QCOMPARE(result.types().size(), 1);
    QCOMPARE(result.types().constFirst(), Type::Document);
    QCOMPARE(result.properties().size(), 0);
}

void PostscriptDscExtractorTest::testPS()
{
    DscExtractor plugin{this};

    QString fileName = testFilePath(QStringLiteral("test.ps"));
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();
    QVERIFY(plugin.mimetypes().contains(mimeType));

    SimpleExtractionResult result(fileName, mimeType);
    plugin.extract(&result);

    QCOMPARE(result.types().size(), 1);
    QCOMPARE(result.types().constFirst(), Type::Document);

    QCOMPARE(result.properties().value(Property::Title).toString(), QStringLiteral("The Big Brown Bear"));
    QCOMPARE(result.properties().value(Property::PageCount).toInt(), 2);

    QDateTime dt(QDate(2018, 10, 28), QTime(21, 13, 39), QTimeZone::fromSecondsAheadOfUtc(+3600));
    QCOMPARE(result.properties().value(Property::CreationDate), QVariant(dt));

    QCOMPARE(result.properties().size(), 3);
}

void PostscriptDscExtractorTest::testEPS()
{
    DscExtractor plugin{this};

    QString fileName = testFilePath(QStringLiteral("test.eps"));
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();
    QVERIFY(plugin.mimetypes().contains(mimeType));

    SimpleExtractionResult result(fileName, mimeType);
    plugin.extract(&result);

    QCOMPARE(result.types().size(), 1);
    QCOMPARE(result.types().constFirst(), Type::Image);

    QCOMPARE(result.properties().value(Property::Title).toString(), QStringLiteral("The Big Brown Bear"));
    QCOMPARE(result.properties().value(Property::PageCount).toInt(), 1);

    QDateTime dt(QDate(2018, 10, 28), QTime(21, 13, 39), QTimeZone::fromSecondsAheadOfUtc(-5400));
    QCOMPARE(result.properties().value(Property::CreationDate), QVariant(dt));
    QCOMPARE(result.properties().size(), 3);
}

QTEST_GUILESS_MAIN(PostscriptDscExtractorTest)

#include "moc_postscriptdscextractortest.cpp"
