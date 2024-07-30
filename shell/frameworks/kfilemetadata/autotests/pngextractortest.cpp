/*
    SPDX-FileCopyrightText: 2022 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "pngextractortest.h"
#include "extractors/pngextractor.h"
#include "indexerextractortestsconfig.h"
#include "mimeutils.h"
#include "simpleextractionresult.h"

#include <QMimeDatabase>
#include <QTest>

using namespace KFileMetaData;
using namespace KFileMetaData::Property;

QString PngExtractorTest::testFilePath(const QString &fileName) const
{
    return QLatin1String(INDEXER_TESTS_SAMPLE_FILES_PATH) + QLatin1Char('/') + fileName;
}

void PngExtractorTest::testNoExtraction()
{
    PngExtractor plugin{this};

    QString fileName = testFilePath(QStringLiteral("test.png"));
    QMimeDatabase mimeDb;
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();
    QVERIFY(plugin.mimetypes().contains(mimeType));

    SimpleExtractionResult result(fileName, mimeType, ExtractionResult::ExtractNothing);
    plugin.extract(&result);

    QCOMPARE(result.types().size(), 1);
    QCOMPARE(result.types().constFirst(), Type::Image);
    QCOMPARE(result.properties().size(), 0);
}

void PngExtractorTest::test()
{
    PngExtractor plugin{this};

    QString fileName = testFilePath(QStringLiteral("test.png"));
    QMimeDatabase mimeDb;
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();
    QVERIFY(plugin.mimetypes().contains(mimeType));

    SimpleExtractionResult result(fileName, mimeType);
    plugin.extract(&result);

    QCOMPARE(result.types().size(), 1);
    QCOMPARE(result.types().constFirst(), Type::Image);

    QCOMPARE(result.properties().value(Author).toString(), QStringLiteral("Author"));
    QCOMPARE(result.properties().value(Copyright).toString(), QStringLiteral("Copyright"));
    QCOMPARE(result.properties().value(Comment).toString(), QStringLiteral("Comment"));
    QCOMPARE(result.properties().value(Generator).toString(), QStringLiteral("Generator"));
}

QTEST_GUILESS_MAIN(PngExtractorTest)

#include "moc_pngextractortest.cpp"
