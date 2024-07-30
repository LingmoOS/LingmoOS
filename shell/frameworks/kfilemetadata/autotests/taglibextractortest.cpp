/*
    TagLibExtractor tests.

    SPDX-FileCopyrightText: 2015 Juan Palacios <jpalaciosdev@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "simpleextractionresult.h"
#include "propertyinfo.h"
//TODO: use QTESTFINDDATA and remove this
#include "indexerextractortestsconfig.h"
#include "extractors/taglibextractor.h"
#include "mimeutils.h"
#include "properties.h"

#include <QMimeDatabase>
#include <QObject>
#include <QSet>
#include <QTest>

class TagLibExtractorTest : public QObject
{
    Q_OBJECT
private:
    QString testFilePath(const QString& fileName) const;

private Q_SLOTS:
    void initTestCase();
    void testNoExtraction();
    void testPropertyTypes();
    void testCommonData();
    void testCommonData_data();
    void testVorbisComment();
    void testVorbisComment_data();
    void testVorbisCommentMultivalue();
    void testVorbisCommentMultivalue_data();
    void testId3();
    void testId3_data();
    void testApe();
    void testApe_data();
    void testMp4();
    void testMp4_data();
    void testAax();
    void testAax_data();
    void testAsf();
    void testAsf_data();
    void testId3Rating_data();
    void testId3Rating();
    void testWmaRating_data();
    void testWmaRating();
    void testNoMetadata();
    void testNoMetadata_data();
    void testRobustness();
    void testRobustness_data();
    void testImageData();
    void testImageData_data();
    void testModuleTrackerFormats();
    void testModuleTrackerFormats_data();

private:
    // Convenience function
    const QStringList propertyEnumNames(const QList<KFileMetaData::Property::Property>& key) const;
    QMimeDatabase mimeDb;
    QByteArray m_coverImage;
};

using namespace KFileMetaData;

QString TagLibExtractorTest::testFilePath(const QString& fileName) const
{
    return QLatin1String(INDEXER_TESTS_SAMPLE_FILES_PATH) + QLatin1Char('/') + fileName;
}

void TagLibExtractorTest::initTestCase()
{
    QFile imgFile(testFilePath("cover.jpg"));
    imgFile.open(QIODevice::ReadOnly);
    m_coverImage = imgFile.readAll();
}

const QStringList TagLibExtractorTest::propertyEnumNames(const QList<KFileMetaData::Property::Property>& keys) const
{
    QStringList result;
    for (auto key : keys) {
        result.append(PropertyInfo(key).name());
    }
    return result;
}

void TagLibExtractorTest::testNoExtraction()
{
    TagLibExtractor plugin{this};

    SimpleExtractionResult result(testFilePath("test.opus"), QStringLiteral("audio/x-opus+ogg"), ExtractionResult::ExtractNothing);
    plugin.extract(&result);

    QCOMPARE(result.types().size(), 1);
    QCOMPARE(result.types().constFirst(), Type::Audio);
    QCOMPARE(result.properties().size(), 0);
}

void TagLibExtractorTest::testPropertyTypes()
{
    TagLibExtractor plugin{this};

    SimpleExtractionResult resultOpus(testFilePath("test.opus"), "audio/x-opus+ogg");
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

void TagLibExtractorTest::testCommonData()
{
    QFETCH(QString, fileType);

    QString fileName = testFilePath(QStringLiteral("test.") + fileType);
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();
    TagLibExtractor plugin{this};

    QVERIFY(plugin.mimetypes().contains(mimeType));

    SimpleExtractionResult result(fileName, mimeType);
    plugin.extract(&result);

    QCOMPARE(result.types().size(), 1);
    QCOMPARE(result.types().constFirst(), Type::Audio);

    QCOMPARE(result.properties().value(Property::Title), QVariant(QStringLiteral("Title")));
    QCOMPARE(result.properties().value(Property::Artist), QVariant(QStringLiteral("Artist")));
    QCOMPARE(result.properties().value(Property::Album), QVariant(QStringLiteral("Album")));
    // ID3v1 only supports a limited set of Genres, not the ID3v2 freeform string "Genre"
    if (qstrcmp(QTest::currentDataTag(), "mp3 id3v1") == 0) {
        QCOMPARE(result.properties().value(Property::Genre), QVariant(QStringLiteral("Soul")));
    } else {
        QCOMPARE(result.properties().value(Property::Genre), QVariant(QStringLiteral("Genre")));
    }
    QCOMPARE(result.properties().value(Property::Comment), QVariant(QStringLiteral("Comment")));
    QCOMPARE(result.properties().value(Property::TrackNumber).toInt(), 1);
    QCOMPARE(result.properties().value(Property::ReleaseYear).toInt(), 2015);
}

void TagLibExtractorTest::testCommonData_data()
{
    QTest::addColumn<QString>("fileType");

    QTest::addRow("aiff")
        << QStringLiteral("aif")
        ;

    QTest::addRow("ape")
        << QStringLiteral("ape")
        ;

    QTest::addRow("flac")
        << QStringLiteral("flac")
        ;

    QTest::addRow("flac+ogg")
        << QStringLiteral("flac.ogg")
        ;

    QTest::addRow("m4a")
        << QStringLiteral("m4a")
        ;

    QTest::addRow("mp3 id3v2") << QStringLiteral("mp3") ;
    QTest::addRow("mp3 id3v1") << QStringLiteral("id3v1.mp3");

    QTest::addRow("mpc")
        << QStringLiteral("mpc")
        ;

    QTest::addRow("ogg")
        << QStringLiteral("ogg")
        ;

    QTest::addRow("opus")
        << QStringLiteral("opus")
        ;

    QTest::addRow("speex")
        << QStringLiteral("spx")
        ;

    QTest::addRow("wav")
        << QStringLiteral("wav")
        ;

    QTest::addRow("wavpack")
        << QStringLiteral("wv")
        ;

    QTest::addRow("wma")
        << QStringLiteral("wma")
        ;
}

void TagLibExtractorTest::testVorbisComment()
{
    QFETCH(QString, fileType);

    QString fileName = testFilePath(QStringLiteral("test.") + fileType);
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();

    TagLibExtractor plugin{this};
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

void TagLibExtractorTest::testVorbisComment_data()
{
    QTest::addColumn<QString>("fileType");

    QTest::addRow("flac")
        << QStringLiteral("flac")
        ;

    QTest::addRow("flac+ogg")
        << QStringLiteral("flac.ogg")
        ;

    QTest::addRow("ogg")
        << QStringLiteral("ogg")
        ;

    QTest::addRow("opus")
        << QStringLiteral("opus")
        ;

    QTest::addRow("speex")
        << QStringLiteral("spx")
        ;
}

void TagLibExtractorTest::testVorbisCommentMultivalue()
{
    QFETCH(QString, fileName);
    QFETCH(QString, mimeType);

    TagLibExtractor plugin{this};
    SimpleExtractionResult result(testFilePath(fileName), mimeType);
    plugin.extract(&result);

    QCOMPARE(result.properties().values(Property::Artist), QVariantList({QStringLiteral("Artist1"), QStringLiteral("Artist2")}));
    QCOMPARE(result.properties().values(Property::Genre), QVariantList({QStringLiteral("Genre1"), QStringLiteral("Genre2")}));
}

void TagLibExtractorTest::testVorbisCommentMultivalue_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<QString>("mimeType");

    QTest::addRow("ogg multivalue")
        << QStringLiteral("test_multivalue.ogg")
        << QStringLiteral("audio/ogg")
        ;
}

void TagLibExtractorTest::testId3()
{
    QFETCH(QString, fileType);

    QString fileName = testFilePath(QStringLiteral("test.") + fileType);
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();

    TagLibExtractor plugin{this};
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

void TagLibExtractorTest::testId3_data()
{
    QTest::addColumn<QString>("fileType");

    QTest::addRow("aiff")
        << QStringLiteral("aif")
        ;

    QTest::addRow("mp3")
        << QStringLiteral("mp3")
        ;

    QTest::addRow("wav")
        << QStringLiteral("wav")
        ;
}

void TagLibExtractorTest::testApe()
{
    QFETCH(QString, fileType);

    QString fileName = testFilePath(QStringLiteral("test.") + fileType);
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();

    TagLibExtractor plugin{this};
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

void TagLibExtractorTest::testApe_data()
{
    QTest::addColumn<QString>("fileType");

    QTest::addRow("ape")
        << QStringLiteral("ape")
        ;

    QTest::addRow("musepack")
        << QStringLiteral("mpc")
        ;

    QTest::addRow("wavpack")
        << QStringLiteral("wv")
        ;
}

void TagLibExtractorTest::testMp4()
{
    QFETCH(QString, fileType);

    QString fileName = testFilePath(QStringLiteral("test.") + fileType);
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();

    TagLibExtractor plugin{this};
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

void TagLibExtractorTest::testMp4_data()
{
    QTest::addColumn<QString>("fileType");

    QTest::addRow("mp4")
        << QStringLiteral("m4a")
        ;
}

void TagLibExtractorTest::testAax()
{
    QFETCH(QString, fileType);

    QString fileName = testFilePath(QStringLiteral("test.") + fileType);
    QString mimeType = QStringLiteral("audio/vnd.audible.aax");

    TagLibExtractor plugin{this};
    SimpleExtractionResult resultAax(fileName, mimeType);
    plugin.extract(&resultAax);

    QCOMPARE(resultAax.properties().value(Property::AlbumArtist), QVariant(QStringLiteral("Album Artist")));
    QCOMPARE(resultAax.properties().value(Property::Artist), QVariant(QStringLiteral("Artist")));
    QCOMPARE(resultAax.properties().value(Property::Genre), QVariant(QStringLiteral("Hörbuch")));
    QCOMPARE(resultAax.properties().value(Property::Copyright), QVariant(QStringLiteral("CopyrightHolder")));
    QCOMPARE(resultAax.properties().value(Property::Title), QVariant(QStringLiteral("TrackTitle")));
    QCOMPARE(resultAax.properties().value(Property::Channels).toInt(), 2);
}

void TagLibExtractorTest::testAax_data()
{
    QTest::addColumn<QString>("fileType");

    QTest::addRow("aax")  << QStringLiteral("aax");
    QTest::addRow("aaxc") << QStringLiteral("aaxc");
}

void TagLibExtractorTest::testAsf()
{
    QFETCH(QString, fileType);

    QString fileName = testFilePath(QStringLiteral("test.") + fileType);
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();

    TagLibExtractor plugin{this};
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

void TagLibExtractorTest::testAsf_data()
{
    QTest::addColumn<QString>("fileType");

    // ASF container format, used by WMA
    QTest::addRow("wma") << QStringLiteral("wma");
}

void TagLibExtractorTest::testId3Rating_data()
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

void TagLibExtractorTest::testId3Rating()
{
    QFETCH(QString, path);
    QFETCH(int, expectedRating);

    TagLibExtractor plugin{this};
    SimpleExtractionResult result(path, "audio/mpeg");
    plugin.extract(&result);

    QCOMPARE(result.properties().value(Property::Rating).toInt(), expectedRating);
}

void TagLibExtractorTest::testWmaRating()
{
    QFETCH(QString, path);
    QFETCH(int, expectedRating);

    TagLibExtractor plugin{this};
    SimpleExtractionResult result(path, "audio/x-ms-wma");
    plugin.extract(&result);

    QCOMPARE(result.properties().value(Property::Rating).toInt(), expectedRating);
}

void TagLibExtractorTest::testWmaRating_data()
{
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

void TagLibExtractorTest::testNoMetadata_data()
{
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
            << expectedKeys << QString()
               ;

    QTest::addRow("m4a")
            << QFINDTESTDATA("samplefiles/no-meta/test.m4a")
            << QStringLiteral("audio/mp4")
            << expectedKeys << QString()
               ;

    QTest::addRow("flac")
            << QFINDTESTDATA("samplefiles/no-meta/test.flac")
            << QStringLiteral("audio/flac")
            << expectedKeys << QString()
               ;

    QTest::addRow("opus")
            << QFINDTESTDATA("samplefiles/no-meta/test.opus")
            << QStringLiteral("audio/x-opus+ogg")
            << expectedKeys << QString()
               ;

    QTest::addRow("ogg")
            << QFINDTESTDATA("samplefiles/no-meta/test.ogg")
            << QStringLiteral("audio/ogg")
            << expectedKeys << QString()
               ;

    QTest::addRow("mpc")
            << QFINDTESTDATA("samplefiles/no-meta/test.mpc")
            << QStringLiteral("audio/x-musepack")
            << expectedKeys << QString()
               ;

    QTest::addRow("aax")
            << QFINDTESTDATA("samplefiles/no-meta/test.aax")
            << QStringLiteral("audio/vnd.audible.aax")
            << expectedKeys << QString()
               ;

}

void  TagLibExtractorTest::testNoMetadata()
{
    QFETCH(QString, path);
    QFETCH(QString, mimeType);
    QFETCH(QList<Property::Property>, expectedKeys);
    QFETCH(QString, failMessage);

    TagLibExtractor plugin{this};
    SimpleExtractionResult extracted(path, mimeType);
    plugin.extract(&extracted);

    const auto resultList = extracted.properties();
    const auto resultKeys = resultList.uniqueKeys();

    const QSet<KFileMetaData::Property::Property> resultKeySet(resultKeys.begin(), resultKeys.end());
    const QSet<KFileMetaData::Property::Property> expectedKeySet(expectedKeys.begin(), expectedKeys.end());

    const auto excessKeys = resultKeySet - expectedKeySet;
    const auto missingKeys = expectedKeySet - resultKeySet;

    if (!excessKeys.isEmpty()) {
        const auto propNames =  propertyEnumNames(excessKeys.values()).join(QLatin1String(", "));
        if (failMessage.isEmpty()) {
            const auto message = QStringLiteral("Excess properties: %1").arg(propNames);
            qWarning() << message;
        } else {
            QEXPECT_FAIL("", qPrintable(QStringLiteral("%1: %2").arg(failMessage).arg(propNames)), Continue);
        }
    } else if (!missingKeys.isEmpty()) {
        const auto message = QStringLiteral("Missing properties: %1")
                .arg(propertyEnumNames(missingKeys.values()).join(QLatin1String(", ")));
        qWarning() << message;
    }
    QCOMPARE(resultKeys, expectedKeys);
    if (!failMessage.isEmpty()) {
        const auto message = QStringLiteral("%1: %2")
                .arg(failMessage)
                .arg(propertyEnumNames(excessKeys.values()).join(QLatin1String(", ")));
        QEXPECT_FAIL("", qPrintable(message), Continue);
    }
    QCOMPARE(resultKeys, expectedKeys);
}

void TagLibExtractorTest::testRobustness_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("mimeType");

    QTest::addRow("ArcGIS GeoData spx")
            << QFINDTESTDATA("samplefiles/misdetected/test_arcgis_geodata.spx")
            << QStringLiteral("audio/x-speex+ogg");
}

void TagLibExtractorTest::testRobustness()
{
    QFETCH(QString, path);
    QFETCH(QString, mimeType);

    TagLibExtractor plugin{this};
    SimpleExtractionResult extracted(path, mimeType);
    plugin.extract(&extracted);
}

void TagLibExtractorTest::testImageData()
{
    QFETCH(QString, fileName);
    QString testAudioFile;
    TagLibExtractor plugin{this};;

    testAudioFile = testFilePath(fileName);
    const QString mimeType = mimeDb.mimeTypeForFile(testAudioFile).name();

    QVERIFY2(!mimeType.isEmpty(), "Failed to determine mimetype");
    QVERIFY2(plugin.mimetypes().contains(mimeType), qPrintable(mimeType + " not supported by taglib"));

    SimpleExtractionResult result(testAudioFile, mimeType, ExtractionResult::ExtractImageData);
    plugin.extract(&result);

    QCOMPARE(result.imageData().value(EmbeddedImageData::FrontCover), m_coverImage);
}

void TagLibExtractorTest::testImageData_data()
{
    QTest::addColumn<QString>("fileName");

    QTest::addRow("aiff")
            << QStringLiteral("test.aif")
            ;

    QTest::addRow("ape")
            << QStringLiteral("test.ape")
            ;

    QTest::addRow("opus")
            << QStringLiteral("test.opus")
            ;

    QTest::addRow("ogg")
            << QStringLiteral("test.ogg")
            ;

    QTest::addRow("flac")
            << QStringLiteral("test.flac")
            ;

    QTest::addRow("flac+ogg")
        << QStringLiteral("test.flac.ogg")
        ;

    QTest::addRow("mp3")
        << QStringLiteral("test.mp3")
        ;

    QTest::addRow("m4a")
            << QStringLiteral("test.m4a")
            ;

    QTest::addRow("mpc")
            << QStringLiteral("test.mpc")
            ;

    QTest::addRow("speex")
            << QStringLiteral("test.spx")
            ;

    QTest::addRow("wav")
            << QStringLiteral("test.wav")
            ;

    QTest::addRow("wavpack")
            << QStringLiteral("test.wv")
            ;

    QTest::addRow("wma")
            << QStringLiteral("test.wma")
            ;
}

void TagLibExtractorTest::testModuleTrackerFormats()
{
    QFETCH(QString, fileType);
    QFETCH(KFileMetaData::PropertyMultiMap, expectedProperties);

    QString fileName = testFilePath(QStringLiteral("test.") + fileType);
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();

    TagLibExtractor plugin{this};
    SimpleExtractionResult resultForMod(fileName, mimeType);
    plugin.extract(&resultForMod);

    QCOMPARE(resultForMod.properties().size(), expectedProperties.size());
    for (auto [property, value] : expectedProperties.asKeyValueRange()) {
        QVERIFY(resultForMod.properties().contains(property));
        QCOMPARE(resultForMod.properties().value(property), value);
    }
}

void TagLibExtractorTest::testModuleTrackerFormats_data()
{
    QTest::addColumn<QString>("fileType");
    QTest::addColumn<KFileMetaData::PropertyMultiMap>("expectedProperties");

    QTest::addRow("mod")
        << "mod"
        << KFileMetaData::PropertyMultiMap{
            {Property::Channels, 4},
            {Property::Title, QStringLiteral("Title Tag")},
            {Property::Generator, QStringLiteral("ProTracker")},
            {Property::Comment, QStringLiteral()}
            }
        ;
    QTest::addRow("s3m")
        << "s3m"
        << KFileMetaData::PropertyMultiMap{
            {Property::Channels, 16},
            {Property::Title, QStringLiteral("Title Tag")},
            {Property::Generator, QStringLiteral("ScreamTracker III")},
            {Property::Comment, QStringLiteral()}
            }
        ;
    QTest::addRow("it")
        << "it"
        << KFileMetaData::PropertyMultiMap{
            {Property::Channels, 1},
            {Property::Title, QStringLiteral("Title Tag")},
            {Property::Generator, QStringLiteral("Impulse Tracker")},
            {Property::Comment, QStringLiteral("Comment Tag")},
            }
        ;
    QTest::addRow("xm")
        << "xm"
        << KFileMetaData::PropertyMultiMap{
            {Property::Channels, 32},
            {Property::Title, QStringLiteral("Title Tag")},
            {Property::Generator, QStringLiteral("OpenMPT 1.31.05.00")},
            {Property::Comment, QStringLiteral()}
            }
        ;
}

QTEST_GUILESS_MAIN(TagLibExtractorTest)

#include "taglibextractortest.moc"
