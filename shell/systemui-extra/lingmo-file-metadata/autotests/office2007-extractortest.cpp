/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2022 kirito <baijunjie@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/


#include "office2007-extractortest.h"
#include "indexerextractortestsconfig.h"
#include "extractors/office2007-extractor.h"
#include "simple-extraction-result.h"
#include "mime-utils.h"

#include <QTest>

using namespace LingmoUIFileMetadata;

QString testFilePath(const QString& baseName, const QString& extension)
{
    return QLatin1String(INDEXER_TESTS_SAMPLE_FILES_PATH)
        + QLatin1Char('/') + baseName + QLatin1Char('.') + extension;
}

void Office2007ExtractorTest::testNoExtraction()
{
    Office2007Extractor plugin{this};

    QString fileName = testFilePath("test_libreoffice", "docx");
    QMimeDatabase mimeDb;
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();
    QVERIFY(plugin.mimetypes().contains(plugin.getSupportedMimeType(mimeType)));

    SimpleExtractionResult result(fileName, mimeType, ExtractionResult::ExtractNothing);
    plugin.extract(&result);

    QVERIFY(result.types().isEmpty());
    QVERIFY(result.properties().isEmpty());
    QVERIFY(result.text().isEmpty());
}


void Office2007ExtractorTest::test()
{
    Office2007Extractor plugin{this};

    QMimeDatabase mimeDb;
    QString fileName = testFilePath("test_libreoffice", "docx");
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();
    QVERIFY(plugin.mimetypes().contains(plugin.getSupportedMimeType(mimeType)));

    SimpleExtractionResult result(fileName, mimeType, ExtractionResult::ExtractPlainText | ExtractionResult::ExtractMetaData | ExtractionResult::ExtractThumbnail);
    result.setThumbnailRequest(ThumbnailRequest({512, 512}, 1));
    plugin.extract(&result);

    QCOMPARE(result.types().size(), 1);
    QCOMPARE(result.types().at(0), Type::Document);
    QCOMPARE(result.properties().value(Property::Title), QVariant(QStringLiteral("KFileMetaData Title")));
    QCOMPARE(result.properties().value(Property::Subject), QVariant(QStringLiteral("KFileMetaData Subject")));
    QCOMPARE(result.properties().value(Property::Keywords), QVariant(QStringLiteral("KFileMetaData keyword")));
    QCOMPARE(result.properties().value(Property::Description), QVariant(QStringLiteral("KFileMetaData comment")));
    QCOMPARE(result.properties().value(Property::Language), QVariant(QStringLiteral("en-US")));
    QVERIFY(result.properties().value(Property::Generator).toString().contains(QStringLiteral("LibreOffice")));

    QCOMPARE(result.properties().size(), 7);
    QCOMPARE(result.text(), QStringLiteral("Test file for KFileMetaData. "));

    QDateTime dt(QDate(2014, 07, 01), QTime(17, 37, 40));
    dt.setTimeSpec(Qt::UTC);
    QCOMPARE(result.properties().value(Property::CreationDate).toDateTime(), QVariant((dt)));
}
QTEST_GUILESS_MAIN(Office2007ExtractorTest)
