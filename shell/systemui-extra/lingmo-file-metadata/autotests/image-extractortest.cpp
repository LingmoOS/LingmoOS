/*
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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

#include "image-extractortest.h"
#include "extractors/image-extractor.h"
#include "simple-extraction-result.h"
#include "indexerextractortestsconfig.h"
#include "mime-utils.h"
#include <QTest>

using namespace LingmoUIFileMetadata;

void LingmoUIFileMetadata::ImageExtractorTest::testImageContent() {
    ImageExtractor plugin{this};
    QDir dir(QStringLiteral(INDEXER_TESTS_SAMPLE_FILES_PATH) + QStringLiteral("/imageTest"));
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

void ImageExtractorTest::testImageThumbnails() {
    ImageExtractor plugin{this};
    QDir dir(QStringLiteral(INDEXER_TESTS_SAMPLE_FILES_PATH) + QStringLiteral("/imageTest"));
    for (const QString &fileName : dir.entryList(QDir::NoDotAndDotDot | QDir::Files)) {
        QString filePath = dir.path() + "/" + fileName;
        QMimeDatabase mimeDb;
        QString mimeType = MimeUtils::strictMimeType(filePath, mimeDb).name();
        SimpleExtractionResult result(filePath, mimeType, ExtractionResult::ExtractThumbnail);
        result.setThumbnailRequest(ThumbnailRequest(QSize(1024, 1024),1));
        qDebug() << filePath;
        plugin.extract(&result);
    }
}

QTEST_GUILESS_MAIN(ImageExtractorTest)