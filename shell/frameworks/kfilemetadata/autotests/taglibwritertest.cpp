/*
    SPDX-FileCopyrightText: 2016 Varun Joshi <varunj.1011@gmail.com>
    SPDX-FileCopyrightText: 2018 Alexander Stippich <a.stippich@gmx.net>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "indexerextractortestsconfig.h"
#include "writers/taglibwriter.h"
#include "writedata.h"
#include <kfilemetadata/ExtractorCollection>
#include <kfilemetadata/Extractor>
#include <kfilemetadata/ExtractionResult>
#include <kfilemetadata/SimpleExtractionResult>

#include <QTest>
#include <QFile>
#include <QMap>

class TagLibWriterTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testCommonData();
    void testCommonData_data();
    void testExtendedData();
    void testExtendedData_data();
    void testRating();
    void testRating_data();
    void testComplexContactData();
    void testComplexContactData_data();
    void testImageWrite();
    void testImageWrite_data();
    void testImageDelete();
    void testImageDelete_data();
    void testImageDeleteInsert();
    void testImageDeleteInsert_data();
    void testMultiImage();
    void testMultiImage_data();

private:
    QByteArray m_coverImage;
};

using namespace KFileMetaData;

namespace
{
QString testFilePath(const QString &fileName)
{
    return QLatin1String(INDEXER_TESTS_SAMPLE_FILES_PATH) + QLatin1Char('/') + fileName;
}

void extractResult(const QString &mimeType, KFileMetaData::ExtractionResult &result)
{
    KFileMetaData::ExtractorCollection extractors;
    QList<KFileMetaData::Extractor*> extractorList = extractors.fetchExtractors(mimeType);
    if (extractorList.isEmpty()) {
        QFAIL("This mime type is not supported by the extractor. Likely a newer KDE Frameworks version is required.");
    }
    if (extractorList.size() > 1) {
        qWarning() << "Multiple extractors are available.";
    }
    KFileMetaData::Extractor* ex = extractorList.first();
    ex->extract(&result);
}
} // <anonymous> namespace

void TagLibWriterTest::initTestCase()
{
    QFile imgFile(testFilePath("cover.jpg"));
    imgFile.open(QIODevice::ReadOnly);
    m_coverImage = imgFile.readAll();
}

void TagLibWriterTest::testCommonData()
{
    QFETCH(QString, fileType);
    QFETCH(QString, mimeType);
    QFETCH(QString, stringSuffix);

    QString temporaryFileName = testFilePath(QStringLiteral("writertest.") + fileType);

    QFile::copy(testFilePath("test." + fileType), temporaryFileName);
    TagLibWriter writerPlugin{this};
    QCOMPARE(writerPlugin.writeMimetypes().contains(mimeType),true);

    WriteData data(temporaryFileName, mimeType);

    data.add(Property::Title, QString(QStringLiteral("Title1") + stringSuffix));
    data.add(Property::Artist, QString(QStringLiteral("Artist1") + stringSuffix));
    data.add(Property::Album, QString(QStringLiteral("Album1") + stringSuffix));
    data.add(Property::AlbumArtist, QString(QStringLiteral("AlbumArtist1") + stringSuffix));
    data.add(Property::Composer, QString(QStringLiteral("Composer1") + stringSuffix));
    data.add(Property::TrackNumber, 10);
    data.add(Property::DiscNumber, 2);
    data.add(Property::ReleaseYear, 1999);
    data.add(Property::Genre, QString(QStringLiteral("Genre1") + stringSuffix));
    data.add(Property::Comment, QString(QStringLiteral("Comment1") + stringSuffix));
    data.add(Property::Copyright, QString(QStringLiteral("Copyright1") + stringSuffix));

    writerPlugin.write(data);

    KFileMetaData::SimpleExtractionResult result(temporaryFileName, mimeType, KFileMetaData::ExtractionResult::ExtractMetaData);
    extractResult(mimeType, result);

    QCOMPARE(result.properties().value(Property::Title), QVariant(QStringLiteral("Title1") + stringSuffix));
    QCOMPARE(result.properties().value(Property::Artist), QVariant(QStringLiteral("Artist1") + stringSuffix));
    QCOMPARE(result.properties().value(Property::Album), QVariant(QStringLiteral("Album1") + stringSuffix));
    QCOMPARE(result.properties().value(Property::AlbumArtist), QVariant(QStringLiteral("AlbumArtist1") + stringSuffix));
    QCOMPARE(result.properties().value(Property::Composer), QVariant(QStringLiteral("Composer1") + stringSuffix));
    QCOMPARE(result.properties().value(Property::TrackNumber).toInt(), 10);
    QCOMPARE(result.properties().value(Property::DiscNumber).toInt(), 2);
    QCOMPARE(result.properties().value(Property::ReleaseYear).toInt(), 1999);
    QCOMPARE(result.properties().value(Property::Genre), QVariant(QStringLiteral("Genre1") + stringSuffix));
    QCOMPARE(result.properties().value(Property::Comment), QVariant(QStringLiteral("Comment1") + stringSuffix));
    QCOMPARE(result.properties().value(Property::Copyright), QVariant(QStringLiteral("Copyright1") + stringSuffix));

    QFile::remove(temporaryFileName);
}

void TagLibWriterTest::testCommonData_data()
{
    // Add some unicode characters, use codepoints to avoid any issues with
    // source encoding: "€µ"
    static const QChar data[2] = { QChar(0x20ac), QChar(0xb5) };
    QString unicodeTestStringSuffix(data, 2);

    QTest::addColumn<QString>("fileType");
    QTest::addColumn<QString>("mimeType");
    QTest::addColumn<QString>("stringSuffix");


    QTest::addRow("aiff")
        << QStringLiteral("aif")
        << QStringLiteral("audio/x-aiff")
        << QString()
        ;

    QTest::addRow("aiff_unicode")
        << QStringLiteral("aif")
        << QStringLiteral("audio/x-aiff")
        << unicodeTestStringSuffix
        ;

    QTest::addRow("ape")
        << QStringLiteral("ape")
        << QStringLiteral("audio/x-ape")
        << QString()
        ;

    QTest::addRow("ape_unicode")
        << QStringLiteral("ape")
        << QStringLiteral("audio/x-ape")
        << unicodeTestStringSuffix
        ;

    QTest::addRow("flac")
        << QStringLiteral("flac")
        << QStringLiteral("audio/flac")
        << QString()
        ;

    QTest::addRow("flac_unicode")
        << QStringLiteral("flac")
        << QStringLiteral("audio/flac")
        << unicodeTestStringSuffix
        ;

    QTest::addRow("m4a")
        << QStringLiteral("m4a")
        << QStringLiteral("audio/mp4")
        << QString()
        ;

    QTest::addRow("m4a_unicode")
        << QStringLiteral("m4a")
        << QStringLiteral("audio/mp4")
        << unicodeTestStringSuffix
        ;

    QTest::addRow("mp3")
        << QStringLiteral("mp3")
        << QStringLiteral("audio/mpeg")
        << QString()
        ;

    QTest::addRow("mp3_unicode")
        << QStringLiteral("mp3")
        << QStringLiteral("audio/mpeg")
        << unicodeTestStringSuffix
        ;

    QTest::addRow("mpc")
        << QStringLiteral("mpc")
        << QStringLiteral("audio/x-musepack")
        << QString()
        ;

    QTest::addRow("mpc_unicode")
        << QStringLiteral("mpc")
        << QStringLiteral("audio/x-musepack")
        << unicodeTestStringSuffix
        ;

    QTest::addRow("ogg")
        << QStringLiteral("ogg")
        << QStringLiteral("audio/ogg")
        << QString()
        ;

    QTest::addRow("ogg_unicode")
        << QStringLiteral("ogg")
        << QStringLiteral("audio/ogg")
        << unicodeTestStringSuffix
        ;

    QTest::addRow("opus")
        << QStringLiteral("opus")
        << QStringLiteral("audio/x-opus+ogg")
        << QString()
        ;

    QTest::addRow("opus_unicode")
        << QStringLiteral("opus")
        << QStringLiteral("audio/x-opus+ogg")
        << unicodeTestStringSuffix
        ;

    QTest::addRow("speex")
        << QStringLiteral("spx")
        << QStringLiteral("audio/x-speex+ogg")
        << QString()
        ;

    QTest::addRow("speex_unicode")
        << QStringLiteral("spx")
        << QStringLiteral("audio/x-speex+ogg")
        << unicodeTestStringSuffix
        ;

    QTest::addRow("wav")
        << QStringLiteral("wav")
        << QStringLiteral("audio/wav")
        << QString()
        ;

    QTest::addRow("wav_unicode")
        << QStringLiteral("wav")
        << QStringLiteral("audio/wav")
        << unicodeTestStringSuffix
        ;

    QTest::addRow("wavpack")
        << QStringLiteral("wv")
        << QStringLiteral("audio/x-wavpack")
        << QString()
        ;

    QTest::addRow("wavpack_unicode")
        << QStringLiteral("wv")
        << QStringLiteral("audio/x-wavpack")
        << unicodeTestStringSuffix
        ;

    QTest::addRow("wma")
        << QStringLiteral("wma")
        << QStringLiteral("audio/x-ms-wma")
        << QString()
        ;

    QTest::addRow("wma_unicode")
        << QStringLiteral("wma")
        << QStringLiteral("audio/x-ms-wma")
        << unicodeTestStringSuffix
        ;
}

void TagLibWriterTest::testExtendedData()
{
    QFETCH(QString, fileType);
    QFETCH(QString, mimeType);

    QString temporaryFileName = testFilePath(QStringLiteral("writertest.") + fileType);

    QFile::copy(testFilePath("test." + fileType), temporaryFileName);
    TagLibWriter writerPlugin{this};

    WriteData data(temporaryFileName, mimeType);

    data.add(Property::Composer, QStringLiteral("Composer1"));
    data.add(Property::Lyricist, QStringLiteral("Lyricist1"));
    data.add(Property::Conductor, QStringLiteral("Conductor1"));
    data.add(Property::Lyrics, QStringLiteral("Lyrics1"));
    data.add(Property::Language, QStringLiteral("Language1"));

    writerPlugin.write(data);

    KFileMetaData::SimpleExtractionResult result(temporaryFileName, mimeType, KFileMetaData::ExtractionResult::ExtractMetaData);
    extractResult(mimeType, result);

    QCOMPARE(result.properties().value(Property::Composer), QVariant(QStringLiteral("Composer1")));
    QCOMPARE(result.properties().value(Property::Lyricist), QVariant(QStringLiteral("Lyricist1")));
    QCOMPARE(result.properties().value(Property::Conductor), QVariant(QStringLiteral("Conductor1")));
    QCOMPARE(result.properties().value(Property::Lyrics), QVariant(QStringLiteral("Lyrics1")));
    QCOMPARE(result.properties().value(Property::Language), QVariant(QStringLiteral("Language1")));

    QFile::remove(temporaryFileName);
}

void TagLibWriterTest::testExtendedData_data()
{
    QTest::addColumn<QString>("fileType");
    QTest::addColumn<QString>("mimeType");

    QTest::addRow("aiff")
        << QStringLiteral("aif")
        << QStringLiteral("audio/x-aiff")
        ;

    QTest::addRow("ape")
        << QStringLiteral("ape")
        << QStringLiteral("audio/x-ape")
        ;

    QTest::addRow("flac")
        << QStringLiteral("flac")
        << QStringLiteral("audio/flac")
        ;

    QTest::addRow("mp3")
        << QStringLiteral("mp3")
        << QStringLiteral("audio/mpeg")
        ;

    QTest::addRow("mpc")
        << QStringLiteral("mpc")
        << QStringLiteral("audio/x-musepack")
        ;

    QTest::addRow("ogg")
        << QStringLiteral("ogg")
        << QStringLiteral("audio/ogg")
        ;

    QTest::addRow("opus")
        << QStringLiteral("opus")
        << QStringLiteral("audio/x-opus+ogg")
        ;

    QTest::addRow("speex")
        << QStringLiteral("spx")
        << QStringLiteral("audio/x-speex+ogg")
        ;

    QTest::addRow("wav")
        << QStringLiteral("wav")
        << QStringLiteral("audio/wav")
        ;

    QTest::addRow("wavpack")
        << QStringLiteral("wv")
        << QStringLiteral("audio/x-wavpack")
        ;
}

void TagLibWriterTest::testRating()
{
    QFETCH(QString, fileType);
    QFETCH(QString, mimeType);
    QFETCH(int, rating);

    QString temporaryFileName = testFilePath(QStringLiteral("writertest.") + fileType);

    QFile::copy(testFilePath("test.") + fileType, temporaryFileName);
    TagLibWriter writerPlugin{this};
    QCOMPARE(writerPlugin.writeMimetypes().contains(mimeType),true);

    WriteData data(temporaryFileName, mimeType);

    data.add(Property::Rating, rating);
    writerPlugin.write(data);

    KFileMetaData::SimpleExtractionResult result(temporaryFileName, mimeType, KFileMetaData::ExtractionResult::ExtractMetaData);
    extractResult(mimeType, result);

    QCOMPARE(result.properties().value(Property::Rating).toInt(), rating);

    QFile::remove(temporaryFileName);
}

void TagLibWriterTest::testRating_data()
{

    QTest::addColumn<QString>("fileType");
    QTest::addColumn<QString>("mimeType");
    QTest::addColumn<int>("rating");

    QTest::addRow("aiff")
        << QStringLiteral("aif")
        << QStringLiteral("audio/x-aiff")
        << 3
        ;

    QTest::addRow("ape")
        << QStringLiteral("ape")
        << QStringLiteral("audio/x-ape")
        << 1
        ;

    QTest::addRow("flac")
        << QStringLiteral("flac")
        << QStringLiteral("audio/flac")
        << 3
        ;

    QTest::addRow("m4a")
        << QStringLiteral("m4a")
        << QStringLiteral("audio/mp4")
        << 5
        ;

    QTest::addRow("mp3_0")
        << QStringLiteral("mp3")
        << QStringLiteral("audio/mpeg")
        << 0
        ;

    QTest::addRow("mp3_1")
        << QStringLiteral("mp3")
        << QStringLiteral("audio/mpeg")
        << 1
        ;

    QTest::addRow("mp3_2")
        << QStringLiteral("mp3")
        << QStringLiteral("audio/mpeg")
        << 2
        ;

    QTest::addRow("mp3_3")
        << QStringLiteral("mp3")
        << QStringLiteral("audio/mpeg")
        << 3
        ;

    QTest::addRow("mp3_4")
        << QStringLiteral("mp3")
        << QStringLiteral("audio/mpeg")
        << 4
        ;

    QTest::addRow("mp3_5")
        << QStringLiteral("mp3")
        << QStringLiteral("audio/mpeg")
        << 5
        ;

    QTest::addRow("mp3_6")
        << QStringLiteral("mp3")
        << QStringLiteral("audio/mpeg")
        << 6
        ;

    QTest::addRow("mp3_7")
        << QStringLiteral("mp3")
        << QStringLiteral("audio/mpeg")
        << 7
        ;

    QTest::addRow("mp3_8")
        << QStringLiteral("mp3")
        << QStringLiteral("audio/mpeg")
        << 8
        ;

    QTest::addRow("mp3_9")
        << QStringLiteral("mp3")
        << QStringLiteral("audio/mpeg")
        << 9
        ;

    QTest::addRow("mp3_10")
        << QStringLiteral("mp3")
        << QStringLiteral("audio/mpeg")
        << 10
        ;

    QTest::addRow("mpc")
        << QStringLiteral("mpc")
        << QStringLiteral("audio/x-musepack")
        << 7
        ;

    QTest::addRow("opus")
        << QStringLiteral("opus")
        << QStringLiteral("audio/x-opus+ogg")
        << 6
        ;

    QTest::addRow("speex")
        << QStringLiteral("spx")
        << QStringLiteral("audio/x-speex+ogg")
        << 8
        ;

    QTest::addRow("wav")
        << QStringLiteral("wav")
        << QStringLiteral("audio/wav")
        << 4
        ;

    QTest::addRow("wavpack")
        << QStringLiteral("wv")
        << QStringLiteral("audio/x-wavpack")
        << 9
        ;

    QTest::addRow("wma_0")
        << QStringLiteral("wma")
        << QStringLiteral("audio/x-ms-wma")
        << 0
        ;

    QTest::addRow("wma_2")
        << QStringLiteral("wma")
        << QStringLiteral("audio/x-ms-wma")
        << 2
        ;

    QTest::addRow("wma_4")
        << QStringLiteral("wma")
        << QStringLiteral("audio/x-ms-wma")
        << 4
        ;

    QTest::addRow("wma_5")
        << QStringLiteral("wma")
        << QStringLiteral("audio/x-ms-wma")
        << 5
        ;

    QTest::addRow("wma_6")
        << QStringLiteral("wma")
        << QStringLiteral("audio/x-ms-wma")
        << 6
        ;

    QTest::addRow("wma_8")
        << QStringLiteral("wma")
        << QStringLiteral("audio/x-ms-wma")
        << 8
        ;

    QTest::addRow("wma_10")
        << QStringLiteral("wma")
        << QStringLiteral("audio/x-ms-wma")
        << 10
        ;
}

void TagLibWriterTest::testComplexContactData()
{
    QFETCH(QString, fileExtension);
    QFETCH(QString, mimeType);

    QString temporaryFileName = testFilePath(QStringLiteral("writertest.") + fileExtension);

    QFile::copy(testFilePath("test." + fileExtension), temporaryFileName);
    TagLibWriter writerPlugin{this};

    WriteData data(temporaryFileName, mimeType);

    const QMap<Property::Property, QString> properties = {
        { Property::Artist, QStringLiteral("Artist1 feat Artist2") },
        { Property::AlbumArtist, QStringLiteral("Artist1 feat. Artist2") },
        { Property::Composer, QStringLiteral("Composer1; Composer2") },
        { Property::Lyricist, QStringLiteral("Lyricist1 ft Lyricist2") },
        { Property::Genre, QStringLiteral("Genre1; Genre2") },
    };

    QMap<Property::Property, QString>::const_iterator it;
    for (it = properties.begin(); it != properties.end(); ++it) {
        data.add(it.key(), it.value());
    }

    writerPlugin.write(data);

    KFileMetaData::SimpleExtractionResult result(temporaryFileName, mimeType, KFileMetaData::ExtractionResult::ExtractMetaData);
    extractResult(mimeType, result);

    for (it = properties.begin(); it != properties.end(); ++it) {
         QCOMPARE(result.properties().value(it.key()), it.value());
    }

    QFile::remove(temporaryFileName);
}

void TagLibWriterTest::testComplexContactData_data()
{
    QTest::addColumn<QString>("fileExtension");
    QTest::addColumn<QString>("mimeType");

    QTest::addRow("aiff")
        << QStringLiteral("aif")
        << QStringLiteral("audio/x-aiff")
        ;

    QTest::addRow("ape")
        << QStringLiteral("ape")
        << QStringLiteral("audio/x-ape")
        ;

    QTest::addRow("flac")
        << QStringLiteral("flac")
        << QStringLiteral("audio/flac")
        ;

    QTest::addRow("mp3")
        << QStringLiteral("mp3")
        << QStringLiteral("audio/mpeg")
        ;

    QTest::addRow("mpc")
        << QStringLiteral("mpc")
        << QStringLiteral("audio/x-musepack")
        ;

    QTest::addRow("ogg")
        << QStringLiteral("ogg")
        << QStringLiteral("audio/ogg")
        ;

    QTest::addRow("opus")
        << QStringLiteral("opus")
        << QStringLiteral("audio/x-opus+ogg")
        ;

    QTest::addRow("speex")
        << QStringLiteral("spx")
        << QStringLiteral("audio/x-speex+ogg")
        ;

    QTest::addRow("wav")
        << QStringLiteral("wav")
        << QStringLiteral("audio/wav")
        ;

    QTest::addRow("wavpack")
        << QStringLiteral("wv")
        << QStringLiteral("audio/x-wavpack")
        ;
}

void TagLibWriterTest::testImageWrite()
{
    QFETCH(QString, fileName);
    QFETCH(QString, mimeType);

    QString testFileName = testFilePath(QStringLiteral("writer") + fileName);

    QFile::copy(testFilePath(fileName), testFileName);
    WriteData data(testFileName, mimeType);

    QMap<EmbeddedImageData::ImageType, QByteArray> writeImages;
    writeImages.insert(EmbeddedImageData::ImageType::FrontCover, m_coverImage);
    data.addImageData(writeImages);
    
    TagLibWriter writerPlugin{this};
    writerPlugin.write(data);
    
    SimpleExtractionResult result(testFileName, mimeType, ExtractionResult::ExtractImageData);
    extractResult(mimeType, result);

    QCOMPARE(result.imageData().value(EmbeddedImageData::FrontCover), writeImages.value(EmbeddedImageData::FrontCover));

    QFile::remove(testFileName);
}

void TagLibWriterTest::testImageWrite_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<QString>("mimeType");

    QTest::addRow("aiff")
            << QStringLiteral("test.aif")
            << QStringLiteral("audio/x-aiff")
            ;

    QTest::addRow("ape")
            << QStringLiteral("test.ape")
            << QStringLiteral("audio/x-ape")
            ;

    QTest::addRow("opus")
            << QStringLiteral("test.opus")
            << QStringLiteral("audio/x-opus+ogg")
            ;

    QTest::addRow("ogg")
            << QStringLiteral("test.ogg")
            << QStringLiteral("audio/ogg")
            ;

    QTest::addRow("flac")
            << QStringLiteral("test.flac")
            << QStringLiteral("audio/flac")
            ;

    QTest::addRow("mp3")
            << QStringLiteral("test.mp3")
            << QStringLiteral("audio/mpeg")
            ;

    QTest::addRow("m4a")
            << QStringLiteral("test.m4a")
            << QStringLiteral("audio/mp4")
            ;

    QTest::addRow("mpc")
            << QStringLiteral("test.mpc")
            << QStringLiteral("audio/x-musepack")
            ;

    QTest::addRow("speex")
            << QStringLiteral("test.spx")
            << QStringLiteral("audio/x-speex+ogg")
            ;

    QTest::addRow("wav")
            << QStringLiteral("test.wav")
            << QStringLiteral("audio/wav")
            ;

    QTest::addRow("wavpack")
            << QStringLiteral("test.wv")
            << QStringLiteral("audio/x-wavpack")
            ;

    QTest::addRow("wma")
            << QStringLiteral("test.wma")
            << QStringLiteral("audio/x-ms-wma")
            ;
}

void TagLibWriterTest::testImageDelete()
{
    QFETCH(QString, fileName);
    QFETCH(QString, mimeType);

    QString testFileName = testFilePath(QStringLiteral("writer") + fileName);

    QFile::copy(testFilePath(fileName), testFileName);
    WriteData data(testFileName, mimeType);

    QMap<EmbeddedImageData::ImageType, QByteArray> writeImages;
    writeImages.insert(EmbeddedImageData::ImageType::FrontCover, QByteArray());
    data.addImageData(writeImages);
    
    TagLibWriter writerPlugin{this};
    writerPlugin.write(data);
    
    SimpleExtractionResult result(testFileName, mimeType, ExtractionResult::ExtractImageData);
    extractResult(mimeType, result);

    QVERIFY(!result.imageData().contains(EmbeddedImageData::FrontCover));

    QFile::remove(testFileName);
}

void TagLibWriterTest::testImageDelete_data()
{
    testImageWrite_data();
}

void TagLibWriterTest::testImageDeleteInsert()
{
    QFETCH(QString, fileName);
    QFETCH(QString, mimeType);

    QString testFileName = testFilePath(QStringLiteral("writer_delinsert_") + fileName);

    QFile::copy(testFilePath(fileName), testFileName);
    WriteData data(testFileName, mimeType);

    QFile imgFile(testFilePath("test.jpg"));
    imgFile.open(QIODevice::ReadOnly);

    QMap<EmbeddedImageData::ImageType, QByteArray> delImages;
    QMap<EmbeddedImageData::ImageType, QByteArray> writeImages;
    QMap<EmbeddedImageData::ImageType, QByteArray> readImages;

    delImages.insert(EmbeddedImageData::ImageType::FrontCover, QByteArray());
    writeImages.insert(EmbeddedImageData::ImageType::FrontCover, m_coverImage);
    data.addImageData(delImages);
    
    TagLibWriter writerPlugin{this};
    writerPlugin.write(data);

    SimpleExtractionResult result(testFileName, mimeType, ExtractionResult::ExtractImageData);
    extractResult(mimeType, result);

    QVERIFY(!result.imageData().contains(EmbeddedImageData::FrontCover));

    data.addImageData(writeImages);
    writerPlugin.write(data);
    extractResult(mimeType, result);

    QCOMPARE(result.imageData().value(EmbeddedImageData::FrontCover), writeImages.value(EmbeddedImageData::FrontCover));

    QFile::remove(testFileName);
}

void TagLibWriterTest::testImageDeleteInsert_data()
{
    testImageWrite_data();
}

void TagLibWriterTest::testMultiImage()
{
    QFETCH(QString, fileName);
    QFETCH(QString, mimeType);
    QFETCH(EmbeddedImageData::ImageTypes, imageTypes);

    QString testFileName = testFilePath(QStringLiteral("writer_multiimage_") + fileName);

    QFile::copy(testFilePath(fileName), testFileName);

    QMap<EmbeddedImageData::ImageType, QByteArray> writeImages;

    SimpleExtractionResult result(testFileName, mimeType, ExtractionResult::ExtractImageData);
    extractResult(mimeType, result);
    QVERIFY(result.imageData().contains(EmbeddedImageData::FrontCover));

    using Image = EmbeddedImageData::ImageType;
    for (auto type : { Image::Other, Image::BackCover }) {
        if (type & imageTypes) {
            writeImages.insert(type, m_coverImage);
        }
    }
    WriteData data(testFileName, mimeType);
    data.addImageData(writeImages);
    TagLibWriter writerPlugin{this};
    writerPlugin.write(data);

    extractResult(mimeType, result);
    // Test if FrontCover still exists
    QVERIFY(result.imageData().contains(EmbeddedImageData::FrontCover));

    for (auto type : { Image::FrontCover, Image::Other, Image::BackCover }) {
        if (type & imageTypes) {
            QVERIFY2(result.imageData().contains(type), qPrintable(QString("has imagetype %1").arg(type)));
            QCOMPARE(result.imageData().value(type), m_coverImage);
        }
    }

    QFile::remove(testFileName);
}

void TagLibWriterTest::testMultiImage_data()
{
    using ImageTypes = EmbeddedImageData::ImageTypes;
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<QString>("mimeType");
    QTest::addColumn<ImageTypes>("imageTypes");

    using Image = EmbeddedImageData::ImageType;
    QTest::addRow("aiff") << QStringLiteral("test.aif") << QStringLiteral("audio/x-aiff") << ImageTypes{ Image::Other, Image::Artist };
    QTest::addRow("opus") << QStringLiteral("test.opus") << QStringLiteral("audio/x-opus+ogg") << ImageTypes{ Image::Other, Image::BackCover };
    QTest::addRow("flac") << QStringLiteral("test.flac") << QStringLiteral("audio/flac") << ImageTypes{ Image::Other, Image::Composer };
    QTest::addRow("wma")  << QStringLiteral("test.wma") << QStringLiteral("audio/x-ms-wma") << ImageTypes{ Image::Other, Image::Band };
}

QTEST_GUILESS_MAIN(TagLibWriterTest)

#include "taglibwritertest.moc"
