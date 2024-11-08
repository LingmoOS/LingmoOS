/*
    SPDX-FileCopyrightText: 2019 Alexander Stippich <a.stippich@gmx.net>
    SPDX-FileCopyrightText: 2019 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "ffmpeg-extractortest.h"
#include "simple-extraction-result.h"
#include "indexerextractortestsconfig.h"
#include "extractors/ffmpeg-extractor.h"
#include "mime-utils.h"

#include <QTest>

using namespace LingmoUIFileMetadata;

namespace {

QString testFilePath(const QString& baseName, const QString& extension)
{
    return QLatin1String(INDEXER_TESTS_SAMPLE_FILES_PATH)
        + QLatin1Char('/') + baseName + QLatin1Char('.') + extension;
}

} // namespace

void ffmpegExtractorTest::testNoExtraction()
{
    QString fileName = testFilePath(QStringLiteral("test"), QStringLiteral("webm"));
    QMimeDatabase mimeDb;
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();

    FFmpegExtractor plugin{this};
    QVERIFY(plugin.mimetypes().contains(plugin.getSupportedMimeType(mimeType)));

    SimpleExtractionResult result(fileName, mimeType, ExtractionResult::ExtractNothing);
    plugin.extract(&result);

    QCOMPARE(result.types().size(), 1);
    QCOMPARE(result.types().constFirst(), Type::Video);
    QCOMPARE(result.properties().size(), 0);
}


void ffmpegExtractorTest::testVideoProperties_data()
{
    QTest::addColumn<QString>("fileType");

    QTest::addRow("WebM")
        << QStringLiteral("webm");

    QTest::addRow("Matroska Video")
        << QStringLiteral("mkv");

    QTest::addRow("Vorbis Video")
        << QStringLiteral("ogv");

    QTest::addRow("MPEG Transport")
        << QStringLiteral("ts");
}

// only for testing of intrinsic video properties
void ffmpegExtractorTest::testVideoProperties()
{
    QFETCH(QString, fileType);

    QString fileName = testFilePath(QStringLiteral("test"), fileType);
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();

    FFmpegExtractor plugin{this};

    QVERIFY(plugin.mimetypes().contains(plugin.getSupportedMimeType(mimeType)));

    SimpleExtractionResult result(fileName, mimeType);
    plugin.extract(&result);

    QCOMPARE(result.types().size(), 1);
    QCOMPARE(result.types().constFirst(), Type::Video);

    QCOMPARE(result.properties().value(Property::Width).toInt(), 1280);
    QCOMPARE(result.properties().value(Property::Height).toInt(), 720);
    QCOMPARE(result.properties().value(Property::FrameRate).toDouble(), 24.0/1.001);
    QCOMPARE(result.properties().value(Property::AspectRatio).toDouble(), 16.0/9);
}

void ffmpegExtractorTest::testMetaData_data()
{
    QTest::addColumn<QString>("fileType");

    QTest::addRow("WebM")
        << QStringLiteral("webm");

    QTest::addRow("Matroska Video")
        << QStringLiteral("mkv");

    QTest::addRow("Vorbis Video")
        << QStringLiteral("ogv");
}

void ffmpegExtractorTest::testMetaData()
{
    QFETCH(QString, fileType);

    QString fileName = testFilePath(QStringLiteral("test"), fileType);
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();

    FFmpegExtractor plugin{this};

    SimpleExtractionResult result(fileName, mimeType);
    plugin.extract(&result);

    QEXPECT_FAIL("Vorbis Video", "Not yet supported", Abort);
    QCOMPARE(result.properties().value(Property::Title).toString(), QStringLiteral("Title"));
    QCOMPARE(result.properties().value(Property::Copyright).toString(), QStringLiteral("Copyright"));
    QCOMPARE(result.properties().value(Property::Author).toString(), QStringLiteral("Author"));
    QCOMPARE(result.properties().value(Property::ReleaseYear).toInt(), 2019);
}

void ffmpegExtractorTest::testThumbnail() {
    QFETCH(QString, fileType);

    QString fileName = testFilePath(QStringLiteral("test"), fileType);
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();


    FFmpegExtractor plugin{this};

    SimpleExtractionResult result(fileName, mimeType, ExtractionResult::ExtractThumbnail);
    result.setThumbnailRequest(ThumbnailRequest(QSize(512, 512), 1));
    plugin.extract(&result);
}

void ffmpegExtractorTest::testThumbnail_data() {
QTest::addColumn<QString>("fileType");

    QTest::addRow("WebM")
        << QStringLiteral("webm");

    QTest::addRow("Matroska Video")
        << QStringLiteral("mkv");

    QTest::addRow("Vorbis Video")
        << QStringLiteral("ogv");

    QTest::addRow("MPEG Transport")
        << QStringLiteral("ts");
}

QTEST_GUILESS_MAIN(ffmpegExtractorTest)
