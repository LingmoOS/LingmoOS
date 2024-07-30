/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "extractors/kritaextractor.h"
#include "indexerextractortestsconfig.h"
#include "mimeutils.h"
#include "kritaextractortest.h"
#include "simpleextractionresult.h"

#include <QMimeDatabase>
#include <QTest>

using namespace KFileMetaData;
using namespace KFileMetaData::Property;

QString KritaExtractorTest::testFilePath(const QString &fileName) const
{
    return QLatin1String(INDEXER_TESTS_SAMPLE_FILES_PATH) + QLatin1Char('/') + fileName;
}

void KritaExtractorTest::testNoExtraction()
{
    KritaExtractor plugin{this};

    QString fileName = testFilePath(QStringLiteral("test.kra"));
    QMimeDatabase mimeDb;
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();
    QVERIFY(plugin.mimetypes().contains(mimeType));

    SimpleExtractionResult result(fileName, mimeType, ExtractionResult::ExtractNothing);
    plugin.extract(&result);

    QCOMPARE(result.types().size(), 1);
    QCOMPARE(result.types().constFirst(), Type::Image);
    QCOMPARE(result.properties().size(), 0);
}

void KritaExtractorTest::test()
{
    KritaExtractor plugin{this};

    QString fileName = testFilePath(QStringLiteral("test.kra"));
    QMimeDatabase mimeDb;
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();
    QVERIFY(plugin.mimetypes().contains(mimeType));

    SimpleExtractionResult result(fileName, mimeType);
    plugin.extract(&result);

    QCOMPARE(result.types().size(), 1);
    QCOMPARE(result.types().constFirst(), Type::Image);

    QCOMPARE(result.properties().value(Width).toInt(), 25);
    QCOMPARE(result.properties().value(Height).toInt(), 50);
    QCOMPARE(result.properties().value(Author).toString(), QStringLiteral("Konqi"));
    QCOMPARE(result.properties().value(License).toString(), QStringLiteral("License"));
    QCOMPARE(result.properties().value(Description).toString(), QStringLiteral("Description"));
    QCOMPARE(result.properties().value(Keywords).toString(), QStringLiteral("Keyword"));

    QDateTime dt(QDate(2023, 5, 26), QTime(14, 55, 31));
    QCOMPARE(result.properties().value(CreationDate).userType(), QMetaType::QDateTime);
    QCOMPARE(result.properties().value(CreationDate).toDateTime(), dt);
}

QTEST_GUILESS_MAIN(KritaExtractorTest)

#include "moc_kritaextractortest.cpp"
