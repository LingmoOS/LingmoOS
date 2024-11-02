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

#include "taglib-extractortest.h"
#include "extractors/taglib-extractor.h"
#include "simple-extraction-result.h"
#include "indexerextractortestsconfig.h"
#include "mime-utils.h"
#include "property-info.h"
#include <QTest>

using namespace LingmoUIFileMetadata;


QString testFilePath(const QString& name)
{
    return QLatin1String(INDEXER_TESTS_SAMPLE_FILES_PATH) + QLatin1Char('/') + name;
}

void TaglibExtractorTest::testNoExtraction() {
    TaglibExtractor plugin{this};

    QString fileName = testFilePath(QStringLiteral("test.opus"));
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();
    QVERIFY(plugin.mimetypes().contains(mimeType));
    SimpleExtractionResult result(testFilePath("test.opus"), mimeType, ExtractionResult::ExtractNothing);
    plugin.extract(&result);

    QCOMPARE(result.types().size(), 1);
    QCOMPARE(result.types().constFirst(), Type::Audio);
    QCOMPARE(result.properties().size(), 0);
}

void TaglibExtractorTest::testPropertyTypes() {
    TaglibExtractor plugin{this};

    SimpleExtractionResult resultOpus(testFilePath("test.opus"), "audio/opus");
    plugin.extract(&resultOpus);

    auto testForType = [](SimpleExtractionResult &result, Property::Property prop) {
        QCOMPARE(result.properties().value(prop).userType(), PropertyInfo(prop).valueType());
    };

    QCOMPARE(resultOpus.types().size(), 1);
    QCOMPARE(resultOpus.types().constFirst(), Type::Audio);
    testForType(resultOpus, Property::Title);
    testForType(resultOpus, Property::Artist);
    testForType(resultOpus, Property::Album);
    testForType(resultOpus, Property::AlbumArtist);
    testForType(resultOpus, Property::Genre);
    testForType(resultOpus, Property::Comment);
    testForType(resultOpus, Property::Composer);
    testForType(resultOpus, Property::Lyricist);
    testForType(resultOpus, Property::Conductor);
    testForType(resultOpus, Property::Arranger);
    testForType(resultOpus, Property::Ensemble);
    testForType(resultOpus, Property::Location);
    testForType(resultOpus, Property::Performer);
    testForType(resultOpus, Property::Language);
    testForType(resultOpus, Property::Publisher);
    testForType(resultOpus, Property::Label);
    testForType(resultOpus, Property::Author);
    testForType(resultOpus, Property::Copyright);
    testForType(resultOpus, Property::Compilation);
    testForType(resultOpus, Property::License);
    testForType(resultOpus, Property::Opus);
    testForType(resultOpus, Property::TrackNumber);
    testForType(resultOpus, Property::ReleaseYear);
    testForType(resultOpus, Property::Channels);
    testForType(resultOpus, Property::DiscNumber);
    testForType(resultOpus, Property::Rating);
    testForType(resultOpus, Property::ReplayGainAlbumGain);
    testForType(resultOpus, Property::ReplayGainAlbumPeak);
    testForType(resultOpus, Property::ReplayGainTrackGain);
    testForType(resultOpus, Property::ReplayGainTrackPeak);
}

void TaglibExtractorTest::testImageData_data() {
QTest::addColumn<QString>("fileName");

    QTest::addRow("aiff")
            << QStringLiteral("test.aif");

    QTest::addRow("ape")
            << QStringLiteral("test.ape");

    QTest::addRow("opus")
            << QStringLiteral("test.opus");

    QTest::addRow("ogg")
            << QStringLiteral("test.ogg");

    QTest::addRow("flac")
            << QStringLiteral("test.flac");

    QTest::addRow("mp3")
            << QStringLiteral("test.mp3");

    QTest::addRow("m4a")
            << QStringLiteral("test.m4a");

    QTest::addRow("mpc")
            << QStringLiteral("test.mpc");

//    QTest::addRow("speex")
//            << QStringLiteral("test.spx");

    QTest::addRow("wav")
            << QStringLiteral("test.wav");

    QTest::addRow("wavpack")
            << QStringLiteral("test.wv");

    QTest::addRow("wma")
            << QStringLiteral("test.wma");
}

void TaglibExtractorTest::testImageData() {
    QFETCH(QString, fileName);
    QString testAudioFile;
    TaglibExtractor plugin{this};;

    testAudioFile = testFilePath(fileName);

    const QString mimeType = mimeDb.mimeTypeForFile(testAudioFile).name();

    QVERIFY2(!mimeType.isEmpty(), "Failed to determine mimetype");
    QVERIFY2(plugin.mimetypes().contains(mimeType), qPrintable(mimeType + " not supported by taglib"));

    SimpleExtractionResult result(testAudioFile, mimeType, ExtractionResult::ExtractImageData | ExtractionResult::ExtractThumbnail);
    result.setThumbnailRequest(ThumbnailRequest({512, 512}, 1));
    plugin.extract(&result);

    QFile imgFile(testFilePath("cover.jpg"));
    imgFile.open(QIODevice::ReadOnly);
    QByteArray coverImage = imgFile.readAll();
    imgFile.close();

    QCOMPARE(result.imageData().value(EmbeddedImageData::FrontCover), coverImage);

}

void TaglibExtractorTest::testCommonData_data() {
    QTest::addColumn<QString>("fileType");

    QTest::addRow("aiff")
            << QStringLiteral("aif");

    QTest::addRow("ape")
            << QStringLiteral("ape");

    QTest::addRow("flac")
            << QStringLiteral("flac");

    QTest::addRow("m4a")
            << QStringLiteral("m4a");

    QTest::addRow("mp3")
            << QStringLiteral("mp3");

    QTest::addRow("mpc")
            << QStringLiteral("mpc");

    QTest::addRow("ogg")
            << QStringLiteral("ogg");

    QTest::addRow("opus")
            << QStringLiteral("opus");

    QTest::addRow("speex")
            << QStringLiteral("spx");

    QTest::addRow("wav")
            << QStringLiteral("wav");

    QTest::addRow("wavpack")
            << QStringLiteral("wv");

    QTest::addRow("wma")
            << QStringLiteral("wma");
}

void TaglibExtractorTest::testCommonData() {
    QFETCH(QString, fileType);

    QString fileName = testFilePath(QStringLiteral("test.") + fileType);
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();
    TaglibExtractor plugin{this};

    QVERIFY(plugin.mimetypes().contains(mimeType));

    SimpleExtractionResult result(fileName, mimeType);
    plugin.extract(&result);

    QCOMPARE(result.types().size(), 1);
    QCOMPARE(result.types().constFirst(), Type::Audio);

    QCOMPARE(result.properties().value(Property::Title), QVariant(QStringLiteral("Title")));
    QCOMPARE(result.properties().value(Property::Artist), QVariant(QStringLiteral("Artist")));
    QCOMPARE(result.properties().value(Property::Album), QVariant(QStringLiteral("Album")));
    QCOMPARE(result.properties().value(Property::Genre), QVariant(QStringLiteral("Genre")));
    QCOMPARE(result.properties().value(Property::Comment), QVariant(QStringLiteral("Comment")));
    QCOMPARE(result.properties().value(Property::TrackNumber).toInt(), 1);
    QCOMPARE(result.properties().value(Property::ReleaseYear).toInt(), 2015);
}

void TaglibExtractorTest::testVorbisComment_data() {
    QTest::addColumn<QString>("fileType");

    QTest::addRow("flac")
        << QStringLiteral("flac");

    QTest::addRow("ogg")
        << QStringLiteral("ogg");

    QTest::addRow("opus")
        << QStringLiteral("opus");

    QTest::addRow("speex")
        << QStringLiteral("spx");
}

void TaglibExtractorTest::testVorbisComment() {
    QFETCH(QString, fileType);

    QString fileName = testFilePath(QStringLiteral("test.") + fileType);
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();

    TaglibExtractor plugin{this};
    QVERIFY(plugin.mimetypes().contains(mimeType));

    SimpleExtractionResult result(fileName, mimeType);
    plugin.extract(&result);

    QCOMPARE(result.properties().value(Property::AlbumArtist), QVariant(QStringLiteral("Album Artist")));
    QCOMPARE(result.properties().value(Property::Composer), QVariant(QStringLiteral("Composer")));
    QCOMPARE(result.properties().value(Property::Lyricist), QVariant(QStringLiteral("Lyricist")));
    QCOMPARE(result.properties().value(Property::Conductor), QVariant(QStringLiteral("Conductor")));
    QCOMPARE(result.properties().value(Property::Arranger), QVariant(QStringLiteral("Arranger")));
    QCOMPARE(result.properties().value(Property::Ensemble), QVariant(QStringLiteral("Ensemble")));
    QCOMPARE(result.properties().value(Property::Location), QVariant(QStringLiteral("Location")));
    QCOMPARE(result.properties().value(Property::Performer), QVariant(QStringLiteral("Performer")));
    QCOMPARE(result.properties().value(Property::Language), QVariant(QStringLiteral("Language")));
    QCOMPARE(result.properties().value(Property::Publisher), QVariant(QStringLiteral("Publisher")));
    QCOMPARE(result.properties().value(Property::Label), QVariant(QStringLiteral("Label")));
    QCOMPARE(result.properties().value(Property::Author), QVariant(QStringLiteral("Author")));
    QCOMPARE(result.properties().value(Property::Copyright), QVariant(QStringLiteral("Copyright")));
    QCOMPARE(result.properties().value(Property::Compilation), QVariant(QStringLiteral("Compilation")));
    QCOMPARE(result.properties().value(Property::License), QVariant(QStringLiteral("License")));
    QCOMPARE(result.properties().value(Property::Lyrics), QVariant(QStringLiteral("Lyrics")));
    QCOMPARE(result.properties().value(Property::Opus).toInt(), 1);
    QCOMPARE(result.properties().value(Property::Channels).toInt(), 1);
    QCOMPARE(result.properties().value(Property::DiscNumber).toInt(), 1);
    QCOMPARE(result.properties().value(Property::Rating).toInt(), 5);
    QCOMPARE(result.properties().value(Property::ReplayGainAlbumGain), QVariant(-9.90));
    QCOMPARE(result.properties().value(Property::ReplayGainAlbumPeak), QVariant(1.512));
    QCOMPARE(result.properties().value(Property::ReplayGainTrackGain), QVariant(-10.44));
    QCOMPARE(result.properties().value(Property::ReplayGainTrackPeak), QVariant(1.301));
}

void TaglibExtractorTest::testVorbisCommentMultivalue_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<QString>("mimeType");

    QTest::addRow("ogg multivalue")
        << QStringLiteral("test_multivalue.ogg")
        << QStringLiteral("audio/ogg");
}

void TaglibExtractorTest::testVorbisCommentMultivalue()
{
    QFETCH(QString, fileName);
    QFETCH(QString, mimeType);

    TaglibExtractor plugin{this};
    QVERIFY(plugin.mimetypes().contains(mimeType));

    SimpleExtractionResult result(testFilePath(fileName), mimeType);
    plugin.extract(&result);

    QCOMPARE(result.properties().values(Property::Artist), QVariantList({QStringLiteral("Artist1"), QStringLiteral("Artist2")}));
    QCOMPARE(result.properties().values(Property::Genre), QVariantList({QStringLiteral("Genre1"), QStringLiteral("Genre2")}));
}

void TaglibExtractorTest::testId3_data()
{
    QTest::addColumn<QString>("fileType");

    QTest::addRow("aiff")
        << QStringLiteral("aif");

    QTest::addRow("mp3")
        << QStringLiteral("mp3");

    QTest::addRow("wav")
        << QStringLiteral("wav");
}

void TaglibExtractorTest::testId3()
{
    QFETCH(QString, fileType);

    QString fileName = testFilePath(QStringLiteral("test.") + fileType);
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();

    TaglibExtractor plugin{this};
    QVERIFY(plugin.mimetypes().contains(mimeType));

    SimpleExtractionResult result(fileName, mimeType);
    plugin.extract(&result);

    QCOMPARE(result.properties().value(Property::AlbumArtist), QVariant(QStringLiteral("Album Artist")));
    QCOMPARE(result.properties().value(Property::Composer), QVariant(QStringLiteral("Composer")));
    QCOMPARE(result.properties().value(Property::Lyricist), QVariant(QStringLiteral("Lyricist")));
    QCOMPARE(result.properties().value(Property::Conductor), QVariant(QStringLiteral("Conductor")));
    QCOMPARE(result.properties().value(Property::Publisher), QVariant(QStringLiteral("Publisher")));
    QCOMPARE(result.properties().value(Property::Language), QVariant(QStringLiteral("Language")));
    QCOMPARE(result.properties().value(Property::Compilation), QVariant(QStringLiteral("Compilation")));
    QCOMPARE(result.properties().value(Property::Lyrics), QVariant(QStringLiteral("Lyrics")));
    QCOMPARE(result.properties().value(Property::Channels).toInt(), 1);
    QCOMPARE(result.properties().value(Property::DiscNumber).toInt(), 1);
    QCOMPARE(result.properties().value(Property::Rating).toInt(), 10);
    QCOMPARE(result.properties().value(Property::ReplayGainAlbumGain), QVariant(-3.33));
    QCOMPARE(result.properties().value(Property::ReplayGainAlbumPeak), QVariant(1.333));
    QCOMPARE(result.properties().value(Property::ReplayGainTrackGain), QVariant(3.33));
    QCOMPARE(result.properties().value(Property::ReplayGainTrackPeak), QVariant(1.369));
}

void TaglibExtractorTest::testApe_data() {
    QTest::addColumn<QString>("fileType");

    QTest::addRow("ape")
            << QStringLiteral("ape");

    QTest::addRow("musepack")
            << QStringLiteral("mpc");

    QTest::addRow("wavpack")
            << QStringLiteral("wv");
}

void TaglibExtractorTest::testApe()
{
    QFETCH(QString, fileType);

    QString fileName = testFilePath(QStringLiteral("test.") + fileType);
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();

    TaglibExtractor plugin{this};
    QVERIFY(plugin.mimetypes().contains(mimeType));

    SimpleExtractionResult result(fileName, mimeType);
    plugin.extract(&result);

    QCOMPARE(result.properties().value(Property::AlbumArtist), QVariant(QStringLiteral("Album Artist")));
    QCOMPARE(result.properties().value(Property::Composer), QVariant(QStringLiteral("Composer")));
    QCOMPARE(result.properties().value(Property::Conductor), QVariant(QStringLiteral("Conductor")));
    QCOMPARE(result.properties().value(Property::Arranger), QVariant(QStringLiteral("Arranger")));
    QCOMPARE(result.properties().value(Property::Ensemble), QVariant(QStringLiteral("Ensemble")));
    QCOMPARE(result.properties().value(Property::Location), QVariant(QStringLiteral("Location")));
    QCOMPARE(result.properties().value(Property::Performer), QVariant(QStringLiteral("Performer")));
    QCOMPARE(result.properties().value(Property::Language), QVariant(QStringLiteral("Language")));
    QCOMPARE(result.properties().value(Property::Publisher), QVariant(QStringLiteral("Publisher")));
    QCOMPARE(result.properties().value(Property::Label), QVariant(QStringLiteral("Label")));
    QCOMPARE(result.properties().value(Property::Author), QVariant(QStringLiteral("Author")));
    QCOMPARE(result.properties().value(Property::Copyright), QVariant(QStringLiteral("Copyright")));
    QCOMPARE(result.properties().value(Property::Compilation), QVariant(QStringLiteral("Compilation")));
    QCOMPARE(result.properties().value(Property::License), QVariant(QStringLiteral("License")));
    QCOMPARE(result.properties().value(Property::Lyrics), QVariant(QStringLiteral("Lyrics")));
    QCOMPARE(result.properties().value(Property::Channels).toInt(), 1);
    QCOMPARE(result.properties().value(Property::DiscNumber).toInt(), 1);
    QCOMPARE(result.properties().value(Property::Rating).toInt(), 4);
    QCOMPARE(result.properties().value(Property::ReplayGainAlbumGain), QVariant(-9.44));
    QCOMPARE(result.properties().value(Property::ReplayGainAlbumPeak), QVariant(1.099));
    QCOMPARE(result.properties().value(Property::ReplayGainTrackGain), QVariant(-5.23));
    QCOMPARE(result.properties().value(Property::ReplayGainTrackPeak), QVariant(1.234));
}

void TaglibExtractorTest::testMp4_data() {
    QTest::addColumn<QString>("fileType");

    QTest::addRow("mp4")
        << QStringLiteral("m4a");
}

void TaglibExtractorTest::testMp4() {
    QFETCH(QString, fileType);

    QString fileName = testFilePath(QStringLiteral("test.") + fileType);
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();

    TaglibExtractor plugin{this};
    QVERIFY(plugin.mimetypes().contains(mimeType));

    SimpleExtractionResult resultMp4(fileName, mimeType);
    plugin.extract(&resultMp4);

    QCOMPARE(resultMp4.properties().value(Property::AlbumArtist), QVariant(QStringLiteral("Album Artist")));
    QCOMPARE(resultMp4.properties().value(Property::Composer), QVariant(QStringLiteral("Composer")));
    QCOMPARE(resultMp4.properties().value(Property::Copyright), QVariant(QStringLiteral("Copyright")));
    QCOMPARE(resultMp4.properties().value(Property::Lyrics), QVariant(QStringLiteral("Lyrics")));
    QCOMPARE(resultMp4.properties().value(Property::Channels).toInt(), 2);
    QCOMPARE(resultMp4.properties().value(Property::DiscNumber).toInt(), 1);
    QCOMPARE(resultMp4.properties().value(Property::Rating).toInt(), 8);
}

void TaglibExtractorTest::testAax_data() {
    QTest::addColumn<QString>("fileType");

    QTest::addRow("aax")
        << QStringLiteral("aax");
}

void TaglibExtractorTest::testAax() {
    QFETCH(QString, fileType);

    QString fileName = testFilePath(QStringLiteral("nocoverage_test.") + fileType);
    QString mimeType = QStringLiteral("audio/vnd.audible.aax");

    TaglibExtractor plugin{this};
    QVERIFY(plugin.mimetypes().contains(mimeType));

    SimpleExtractionResult resultAax(fileName, mimeType);
    plugin.extract(&resultAax);

    QCOMPARE(resultAax.properties().value(Property::AlbumArtist), QVariant(QStringLiteral("Album Artist")));
    QCOMPARE(resultAax.properties().value(Property::Artist), QVariant(QStringLiteral("Artist")));
    QCOMPARE(resultAax.properties().value(Property::Genre), QVariant(QStringLiteral("Hörbuch")));
    QCOMPARE(resultAax.properties().value(Property::Copyright), QVariant(QStringLiteral("CopyrightHolder")));
    QCOMPARE(resultAax.properties().value(Property::Title), QVariant(QStringLiteral("TrackTitle")));
    QCOMPARE(resultAax.properties().value(Property::Channels).toInt(), 2);
}

void TaglibExtractorTest::testAsf_data() {
    QTest::addColumn<QString>("fileType");

    // ASF container format, used by WMA
    QTest::addRow("wma") << QStringLiteral("wma");
}

void TaglibExtractorTest::testAsf() {
    QFETCH(QString, fileType);

    QString fileName = testFilePath(QStringLiteral("test.") + fileType);
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();

    TaglibExtractor plugin{this};
    QVERIFY(plugin.mimetypes().contains(mimeType));

    SimpleExtractionResult result(fileName, mimeType);
    plugin.extract(&result);

    QCOMPARE(result.properties().value(Property::AlbumArtist), QVariant(QStringLiteral("Album Artist")));
    QCOMPARE(result.properties().value(Property::Rating).toInt(), 6);
    QCOMPARE(result.properties().value(Property::DiscNumber).toInt(), 1);
    QCOMPARE(result.properties().value(Property::Conductor), QVariant(QStringLiteral("Conductor")));
    QCOMPARE(result.properties().value(Property::Composer), QVariant(QStringLiteral("Composer")));
    QCOMPARE(result.properties().value(Property::Author), QVariant(QStringLiteral("Author")));
    QCOMPARE(result.properties().value(Property::Lyricist), QVariant(QStringLiteral("Lyricist")));
    QCOMPARE(result.properties().value(Property::Copyright), QVariant(QStringLiteral("Copyright")));
    QCOMPARE(result.properties().value(Property::Publisher), QVariant(QStringLiteral("Publisher")));
}

void TaglibExtractorTest::testId3Rating_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<int>("expectedRating");

    QTest::addRow("WMP")
        << QFINDTESTDATA("samplefiles/mp3_rating/testWMP.mp3")
        << 0 ;
    QTest::addRow("WMP1")
        << QFINDTESTDATA("samplefiles/mp3_rating/testWMP1.mp3")
        << 2 ;
    QTest::addRow("WMP2")
        << QFINDTESTDATA("samplefiles/mp3_rating/testWMP2.mp3")
        << 4 ;
    QTest::addRow("WMP3")
        << QFINDTESTDATA("samplefiles/mp3_rating/testWMP3.mp3")
        << 6 ;
    QTest::addRow("WMP4")
        << QFINDTESTDATA("samplefiles/mp3_rating/testWMP4.mp3")
        << 8 ;
    QTest::addRow("WMP5")
        << QFINDTESTDATA("samplefiles/mp3_rating/testWMP5.mp3")
        << 10 ;
    QTest::addRow("MM")
        << QFINDTESTDATA("samplefiles/mp3_rating/testMM.mp3")
        << 0 ;
    QTest::addRow("MM1")
        << QFINDTESTDATA("samplefiles/mp3_rating/testMM1.mp3")
        << 1 ;
    QTest::addRow("MM2")
        << QFINDTESTDATA("samplefiles/mp3_rating/testMM2.mp3")
        << 2 ;
    QTest::addRow("MM3")
        << QFINDTESTDATA("samplefiles/mp3_rating/testMM3.mp3")
        << 3 ;
    QTest::addRow("MM4")
        << QFINDTESTDATA("samplefiles/mp3_rating/testMM4.mp3")
        << 4 ;
    QTest::addRow("MM5")
        << QFINDTESTDATA("samplefiles/mp3_rating/testMM5.mp3")
        << 5 ;
    QTest::addRow("MM6")
        << QFINDTESTDATA("samplefiles/mp3_rating/testMM6.mp3")
        << 6 ;
    QTest::addRow("MM7")
        << QFINDTESTDATA("samplefiles/mp3_rating/testMM7.mp3")
        << 7 ;
    QTest::addRow("MM8")
        << QFINDTESTDATA("samplefiles/mp3_rating/testMM8.mp3")
        << 8 ;
    QTest::addRow("MM9")
        << QFINDTESTDATA("samplefiles/mp3_rating/testMM9.mp3")
        << 9 ;
    QTest::addRow("MM10")
        << QFINDTESTDATA("samplefiles/mp3_rating/testMM10.mp3")
        << 10 ;
}

void TaglibExtractorTest::testId3Rating() {
    QFETCH(QString, path);
    QFETCH(int, expectedRating);

    TaglibExtractor plugin{this};
    SimpleExtractionResult result(path, "audio/mpeg");
    plugin.extract(&result);

    QCOMPARE(result.properties().value(Property::Rating).toInt(), expectedRating);
}

void TaglibExtractorTest::testWmaRating_data() {
    QTest::addColumn<QString>("path");
    QTest::addColumn<int>("expectedRating");

    QTest::addRow("WMP0")
        << QFINDTESTDATA("samplefiles/wma_rating/test0.wma")
        << 0 ;
    QTest::addRow("WMP1")
        << QFINDTESTDATA("samplefiles/wma_rating/test1.wma")
        << 2 ;
    QTest::addRow("WMP2")
        << QFINDTESTDATA("samplefiles/wma_rating/test2.wma")
        << 4 ;
    QTest::addRow("WMP3")
        << QFINDTESTDATA("samplefiles/wma_rating/test3.wma")
        << 6 ;
    QTest::addRow("WMP4")
        << QFINDTESTDATA("samplefiles/wma_rating/test4.wma")
        << 8 ;
    QTest::addRow("WMP5")
        << QFINDTESTDATA("samplefiles/wma_rating/test5.wma")
        << 10 ;
}

void TaglibExtractorTest::testWmaRating() {
    QFETCH(QString, path);
    QFETCH(int, expectedRating);

    TaglibExtractor plugin{this};
    SimpleExtractionResult result(path, "audio/x-ms-wma");
    plugin.extract(&result);

    QCOMPARE(result.properties().value(Property::Rating).toInt(), expectedRating);
}

void TaglibExtractorTest::testNoMetadata_data() {
    const auto expectedKeys = QList<Property::Property>{
            Property::BitRate,
            Property::Channels,
            Property::Duration,
            Property::SampleRate,
    };

    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("mimeType");
    QTest::addColumn<QList<Property::Property>>("expectedKeys");
    QTest::addColumn<QString>("failMessage");

    QTest::addRow("mp3")
            << QFINDTESTDATA("samplefiles/no-meta/test.mp3")
            << QStringLiteral("audio/mpeg")
            << expectedKeys << QString();

    QTest::addRow("m4a")
            << QFINDTESTDATA("samplefiles/no-meta/test.m4a")
            << QStringLiteral("audio/mp4")
            << expectedKeys << QString();

    QTest::addRow("flac")
            << QFINDTESTDATA("samplefiles/no-meta/test.flac")
            << QStringLiteral("audio/flac")
            << expectedKeys << QString();

    QTest::addRow("opus")
            << QFINDTESTDATA("samplefiles/no-meta/test.opus")
            << QStringLiteral("audio/opus")
            << expectedKeys << QString();

    QTest::addRow("ogg")
            << QFINDTESTDATA("samplefiles/no-meta/test.ogg")
            << QStringLiteral("audio/ogg")
            << expectedKeys << QString();

    QTest::addRow("mpc")
            << QFINDTESTDATA("samplefiles/no-meta/test.mpc")
            << QStringLiteral("audio/x-musepack")
            << expectedKeys << QString();

    QTest::addRow("aax")
            << QFINDTESTDATA("samplefiles/no-meta/test.aax")
            << QStringLiteral("audio/vnd.audible.aax")
            << expectedKeys << QString();
}

QStringList propertyEnumNames(const QList<Property::Property>& keys)
{
    QStringList result;
    for (auto key : keys) {
        result.append(PropertyInfo(key).name());
    }
    return result;
}


void TaglibExtractorTest::testNoMetadata() {
    QFETCH(QString, path);
    QFETCH(QString, mimeType);
    QFETCH(QList<Property::Property>, expectedKeys);
    QFETCH(QString, failMessage);

    TaglibExtractor plugin{this};
    QVERIFY(plugin.mimetypes().contains(mimeType));

    SimpleExtractionResult extracted(path, mimeType);
    plugin.extract(&extracted);

    const auto resultList = extracted.properties();
    const auto resultKeys = resultList.uniqueKeys();

    const QSet<Property::Property> resultKeySet(resultKeys.begin(), resultKeys.end());
    const QSet<Property::Property> expectedKeySet(expectedKeys.begin(), expectedKeys.end());

    const auto excessKeys = resultKeySet - expectedKeySet;
    const auto missingKeys = expectedKeySet - resultKeySet;

    if (!excessKeys.isEmpty()) {
        const auto propNames =  propertyEnumNames(excessKeys.values()).join(QLatin1String(", "));
        if (failMessage.isEmpty()) {
            const auto message = QStringLiteral("Excess properties: %1").arg(propNames);
            qWarning() << message;
        } else {
            QEXPECT_FAIL("", qPrintable(QStringLiteral("%1: %2").arg(failMessage, propNames)), Continue);
        }
    } else if (!missingKeys.isEmpty()) {
        const auto message = QStringLiteral("Missing properties: %1")
                .arg(propertyEnumNames(missingKeys.values()).join(QLatin1String(", ")));
        qWarning() << message;
    }
    QCOMPARE(resultKeys, expectedKeys);
    if (!failMessage.isEmpty()) {
        const auto message = QStringLiteral("%1: %2")
                .arg(failMessage,
                     propertyEnumNames(excessKeys.values()).join(QLatin1String(", ")));
        QEXPECT_FAIL("", qPrintable(message), Continue);
    }
    QCOMPARE(resultKeys, expectedKeys);
}

void TaglibExtractorTest::testRobustness_data() {
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("mimeType");

    QTest::addRow("ArcGIS GeoData spx")
            << QFINDTESTDATA("samplefiles/misdetected/test_arcgis_geodata.spx")
            << QStringLiteral("audio/x-speex+ogg");
}

void TaglibExtractorTest::testRobustness() {
    QFETCH(QString, path);
    QFETCH(QString, mimeType);

    TaglibExtractor plugin{this};
    QVERIFY(plugin.mimetypes().contains(mimeType));

    SimpleExtractionResult extracted(path, mimeType);
    plugin.extract(&extracted);
}

QTEST_GUILESS_MAIN(TaglibExtractorTest)