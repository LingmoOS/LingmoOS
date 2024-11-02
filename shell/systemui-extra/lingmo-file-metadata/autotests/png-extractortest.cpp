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

#include "png-extractortest.h"
#include "extractors/png-extractor.h"
#include "simple-extraction-result.h"
#include "indexerextractortestsconfig.h"
#include "mime-utils.h"
#include <QTest>

using namespace LingmoUIFileMetadata;

void PngExtractorTest::testNoExtraction() {
    PngExtractor plugin{this};
    QDir dir(QStringLiteral(INDEXER_TESTS_SAMPLE_FILES_PATH) + QStringLiteral("/pngTest"));
    for (const QString &fileName : dir.entryList(QDir::NoDotAndDotDot | QDir::Files)) {
        QString filePath = dir.path() + "/" + fileName;
        QMimeDatabase mimeDb;
        QString mimeType = MimeUtils::strictMimeType(filePath, mimeDb).name();
        SimpleExtractionResult result(filePath, mimeType, ExtractionResult::ExtractNothing);
        plugin.extract(&result);

        QCOMPARE(result.types().size(), 1);
        QCOMPARE(result.types().constFirst(), Type::Image);
        QCOMPARE(result.properties().size(), 0);
    }
}

void PngExtractorTest::testMetaDataOnly() {
    PngExtractor plugin{this};
    QDir dir(QStringLiteral(INDEXER_TESTS_SAMPLE_FILES_PATH) + QStringLiteral("/pngTest"));
    for (const QString &fileName : dir.entryList(QDir::NoDotAndDotDot | QDir::Files)) {
        QString filePath = dir.path() + "/" + fileName;
        QMimeDatabase mimeDb;
        QString mimeType = MimeUtils::strictMimeType(filePath, mimeDb).name();
        SimpleExtractionResult result(filePath, mimeType, ExtractionResult::ExtractMetaData);
        plugin.extract(&result);

        QCOMPARE(result.properties().value(Property::Author), QVariant("Author"));
        QCOMPARE(result.properties().value(Property::Title), QVariant("Title"));
        QCOMPARE(result.properties().value(Property::Copyright), QVariant("Copyright"));
        QCOMPARE(result.properties().value(Property::Generator), QVariant("Generator"));
        QCOMPARE(result.properties().value(Property::Comment), QVariant("Comment"));
    }
}

void PngExtractorTest::testPngContent() {
    PngExtractor plugin{this};
    QDir dir(QStringLiteral(INDEXER_TESTS_SAMPLE_FILES_PATH) + QStringLiteral("/pngTest"));
    for (const QString &fileName : dir.entryList(QDir::NoDotAndDotDot | QDir::Files)) {
        QString filePath = dir.path() + "/" + fileName;
        QMimeDatabase mimeDb;
        QString mimeType = MimeUtils::strictMimeType(filePath, mimeDb).name();
        SimpleExtractionResult result(filePath, mimeType, ExtractionResult::ExtractPlainText);
        qDebug() << QDateTime::currentDateTime().toString();
        plugin.extract(&result);
        qDebug() << QDateTime::currentDateTime().toString() << "======filePath" << filePath << result.text();
    }
}

void PngExtractorTest::testThumbnail() {
    PngExtractor plugin{this};
    QDir dir(QStringLiteral(INDEXER_TESTS_SAMPLE_FILES_PATH) + QStringLiteral("/pngTest"));
    for (const QString &fileName : dir.entryList(QDir::NoDotAndDotDot | QDir::Files)) {
        QString filePath = dir.path() + "/" + fileName;
        QMimeDatabase mimeDb;
        QString mimeType = MimeUtils::strictMimeType(filePath, mimeDb).name();

        SimpleExtractionResult result(filePath, mimeType, ExtractionResult::ExtractThumbnail);
        result.setThumbnailRequest(ThumbnailRequest(QSize(512, 512),1));
        plugin.extract(&result);
    }
}

QTEST_GUILESS_MAIN(PngExtractorTest)