/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: baijunjie <baijunjie@kylinos.cn>
 * Modified by: baijunjie <baijunjie@kylinos.cn>
 *
 */

#include "uof-extractortest.h"
#include "extractors/uof-extractor.h"
#include "indexerextractortestsconfig.h"
#include "simple-extraction-result.h"
#include "mime-utils.h"
#include <QTest>

using namespace LingmoUIFileMetadata;

QString testFilePath(const QString& baseName, const QString& extension)
{
    return QLatin1String(INDEXER_TESTS_OFFICE_FILES_PATH)
           + QLatin1Char('/') + baseName + QLatin1Char('.') + extension;
}

void UofExtractorTest::testContent_data()
{
    QTest::addColumn<QString>("fileType");

    QTest::addRow("Uof")
        << QStringLiteral("uof");

    QTest::addRow("Uop")
        << QStringLiteral("uop");

    QTest::addRow("Uos")
        << QStringLiteral("uos");

    QTest::addRow("Uot")
        << QStringLiteral("uot");
}

void UofExtractorTest::testNoExtraction()
{
    UofExtractor plugin{this};

    QString fileName = testFilePath(QStringLiteral("test"), "uof");
    QMimeDatabase mimeDb;
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();
    QVERIFY(plugin.mimetypes().contains(mimeType));

    SimpleExtractionResult result(fileName, mimeType, ExtractionResult::ExtractNothing);
    plugin.extract(&result);

    QCOMPARE(result.types().size(), 1);
    QCOMPARE(result.types().constFirst(), Type::Document);
    QCOMPARE(result.properties().size(), 0);
}

void UofExtractorTest::testContent()
{
    QFETCH(QString, fileType);

    UofExtractor plugin{this};

    QMimeDatabase mimeDb;
    QString fileName = testFilePath(QStringLiteral("test"), fileType);
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();
    QVERIFY(plugin.mimetypes().contains(plugin.getSupportedMimeType(mimeType)));

    SimpleExtractionResult result(fileName, mimeType);
    plugin.extract(&result);

    QCOMPARE(result.text(), QStringLiteral("TestfileforLingmoUIFileMetaData "));
}

void UofExtractorTest::testMetaDataOnly()
{
    UofExtractor plugin{this};

    QStringList suffixes {"uof", "uot", "uos", "uop"};

    for (const QString& suffix : suffixes) {
        QString fileName = testFilePath(QStringLiteral("test"), suffix);
        QMimeDatabase mimeDb;
        QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();
        QVERIFY(plugin.mimetypes().contains(mimeType));

        SimpleExtractionResult result(fileName, mimeType, ExtractionResult::ExtractMetaData);
        plugin.extract(&result);

        QCOMPARE(result.properties().value(Property::Author), QVariant(QStringLiteral("LingmoUIFileMetaData")));
        QCOMPARE(result.properties().value(Property::Generator), QVariant(QStringLiteral("WPS Office")));
        QCOMPARE(result.properties().value(Property::CreationDate), QVariant(QStringLiteral("2023-06-19T16:25:00")));
    }

}

QTEST_GUILESS_MAIN(UofExtractorTest)

