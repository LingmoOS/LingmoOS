/*
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 *
 */
#include "text-extractortest.h"
#include "extractors/text-extractor.h"
#include "indexerextractortestsconfig.h"
#include "simple-extraction-result.h"
#include "mime-utils.h"

#include <QTest>

using namespace LingmoUIFileMetadata;

QString testFilePath(const QString& name)
{
    return QLatin1String(INDEXER_TESTS_SAMPLE_FILES_PATH) + QLatin1Char('/') + name;
}

void TextExtractorTest::test()
{
    TextExtractor plugin{this};

    QString fileName = testFilePath(QStringLiteral("test.txt"));
    QMimeDatabase mimeDb;
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();
    QVERIFY(plugin.mimetypes().contains(mimeType));

    SimpleExtractionResult result(fileName, mimeType, ExtractionResult::ExtractPlainText | ExtractionResult::ExtractThumbnail);
    result.setThumbnailRequest(ThumbnailRequest({352, 247}, 1));
    plugin.extract(&result);

    QCOMPARE(result.text(), QStringLiteral("This is a test. This is a test. "));
    QCOMPARE(result.properties().value(Property::LineCount), 2);
}

QTEST_GUILESS_MAIN(TextExtractorTest)
