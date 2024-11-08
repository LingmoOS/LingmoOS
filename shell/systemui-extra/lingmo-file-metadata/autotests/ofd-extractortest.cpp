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

#include "ofd-extractortest.h"
#include "extractors/ofd-extractor.h"
#include "indexerextractortestsconfig.h"
#include "simple-extraction-result.h"
#include "mime-utils.h"
#include <QTest>

using namespace LingmoUIFileMetadata;

QString testFilePath(const QString& name)
{
    return QLatin1String(INDEXER_TESTS_OFFICE_FILES_PATH) + QLatin1Char('/') + name;
}

void OfdExtractorTest::testNoExtraction() {
    OfdExtractor plugin{this};

    QString fileName = testFilePath(QStringLiteral("test.ofd"));
    QMimeDatabase mimeDb;
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();
    QVERIFY(plugin.mimetypes().contains(mimeType));

    SimpleExtractionResult result(fileName, mimeType, ExtractionResult::ExtractNothing);
    plugin.extract(&result);

    QCOMPARE(result.types().size(), 1);
    QCOMPARE(result.types().constFirst(), Type::Document);
    QCOMPARE(result.properties().size(), 0);
}

void OfdExtractorTest::testAllData() {
    OfdExtractor plugin{this};

    QString fileName = testFilePath(QStringLiteral("test.ofd"));
    QMimeDatabase mimeDb;
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();
    QVERIFY(plugin.mimetypes().contains(mimeType));

    SimpleExtractionResult result(fileName, mimeType);
    plugin.extract(&result);

    QCOMPARE(result.text(), QStringLiteral("TestfileforKFileMetaData. "));
    QCOMPARE(result.properties().value(Property::Title), QVariant(QStringLiteral("KFileMetaData Title")));
    QCOMPARE(result.properties().value(Property::Subject), QVariant(QStringLiteral("KFileMetaData Subject")));
    QCOMPARE(result.properties().value(Property::CreationDate), QVariant(QStringLiteral("2023-5-17")));
    QCOMPARE(result.properties().value(Property::Generator), QVariant(QStringLiteral("WPS 文字")));
    QCOMPARE(result.properties().value(Property::Keywords), QVariant(QStringList("KFileMetaData keyword")));
}

void OfdExtractorTest::testMetaDataOnly() {
    OfdExtractor plugin{this};

    QString fileName = testFilePath(QStringLiteral("test.ofd"));
    QMimeDatabase mimeDb;
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();
    QVERIFY(plugin.mimetypes().contains(mimeType));

    SimpleExtractionResult result(fileName, mimeType, ExtractionResult::ExtractMetaData);
    plugin.extract(&result);

    QCOMPARE(result.types().size(), 1);
    QVERIFY(result.text().isEmpty());
    QCOMPARE(result.properties().size(), 5);
    QCOMPARE(result.properties().value(Property::Title), QVariant(QStringLiteral("KFileMetaData Title")));
    QCOMPARE(result.properties().value(Property::Subject), QVariant(QStringLiteral("KFileMetaData Subject")));
    QCOMPARE(result.properties().value(Property::CreationDate), QVariant(QStringLiteral("2023-5-17")));
    QCOMPARE(result.properties().value(Property::Generator), QVariant(QStringLiteral("WPS 文字")));
    QCOMPARE(result.properties().value(Property::Keywords), QVariant(QStringList("KFileMetaData keyword")));
}

QTEST_GUILESS_MAIN(OfdExtractorTest)