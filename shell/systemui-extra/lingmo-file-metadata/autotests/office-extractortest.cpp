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
#include "office-extractortest.h"
#include "indexerextractortestsconfig.h"
#include "extractors/office-extractor.h"
#include "simple-extraction-result.h"
#include "mime-utils.h"

#include <QTest>

using namespace LingmoUIFileMetadata;

QString testFilePath(const QString& baseName, const QString& extension)
{
    return QLatin1String(INDEXER_TESTS_OFFICE_FILES_PATH)
        + QLatin1Char('/') + baseName + QLatin1Char('.') + extension;
}

void OfficeExtractorTest::testContent_data()
{
    QTest::addColumn<QString>("fileType");

    QTest::addRow("Doc")
        << QStringLiteral("doc");

    QTest::addRow("WPS")
        << QStringLiteral("wps");

    QTest::addRow("Dot")
        << QStringLiteral("dot");

    QTest::addRow("Xls")
        << QStringLiteral("xls");

    QTest::addRow("Et")
        << QStringLiteral("et");

    QTest::addRow("Dps")
        << QStringLiteral("dps");

    QTest::addRow("PPT")
        << QStringLiteral("ppt");

    QTest::addRow("PPS")
        << QStringLiteral("pps");
}

void OfficeExtractorTest::testContent()
{
    QFETCH(QString, fileType);

    OfficeExtractor plugin{this};

    QMimeDatabase mimeDb;
    QString fileName = testFilePath(QStringLiteral("test"), fileType);
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();
    QVERIFY(plugin.mimetypes().contains(plugin.getSupportedMimeType(mimeType)));

    SimpleExtractionResult result(fileName, mimeType);
    plugin.extract(&result);

    QCOMPARE(result.text(), QStringLiteral("hwf"));
}
QTEST_GUILESS_MAIN(OfficeExtractorTest)
