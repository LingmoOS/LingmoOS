/*
    SPDX-FileCopyrightText: 2014 Alex Merry <alex.merry@kdemail.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <stdio.h>

#include <QBuffer>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QImageWriter>
#include <QTest>
#include <QUuid>

Q_DECLARE_METATYPE(QImage::Format)

class PicTests : public QObject
{
    Q_OBJECT

private:
    void common_data()
    {
        QTest::addColumn<QString>("picfile");
        QTest::addColumn<QString>("pngfile");
        QTest::addColumn<QString>("comment");
        // whether the pic file has/should have an alpha channel
        QTest::addColumn<bool>("alpha");
        // the format to convert the png file to before writing
        // or comparing to the read image; this can be used to
        // induce loss of data (eg: make the image monochrome)
        QTest::addColumn<QImage::Format>("pngformat");
        QTest::addColumn<bool>("compress");

        QTest::newRow("4x4 no alpha RLE") << QFINDTESTDATA("pic/4x4-simple-color.pic") << QFINDTESTDATA("pic/4x4-simple-color.png") << QString() << false
                                          << QImage::Format_RGB32 << true;

        QTest::newRow("4x4 no alpha raw") << QFINDTESTDATA("pic/4x4-simple-color-uncompressed.pic") << QFINDTESTDATA("pic/4x4-simple-color.png") << QString()
                                          << false << QImage::Format_RGB32 << false;

        QTest::newRow("Short comment") << QFINDTESTDATA("pic/short-comment.pic") << QFINDTESTDATA("pic/4x4-simple-color.png")
                                       << QStringLiteral("Test comment value") << false << QImage::Format_RGB32 << true;

        QTest::newRow("Long comment") << QFINDTESTDATA("pic/long-comment.pic") << QFINDTESTDATA("pic/4x4-simple-color.png")
                                      << QStringLiteral("Test comment value that goes right up to the end of the comment field and has no") << false
                                      << QImage::Format_RGB32 << true;

        QTest::newRow("Long run-lengths") << QFINDTESTDATA("pic/long-runs.pic") << QFINDTESTDATA("pic/long-runs.png") << QString() << false
                                          << QImage::Format_RGB32 << true;

        QTest::newRow("4x4 with alpha RLE") << QFINDTESTDATA("pic/4x4-alpha.pic") << QFINDTESTDATA("pic/4x4-alpha.png") << QString() << true
                                            << QImage::Format_ARGB32 << true;

        QTest::newRow("4x4 with alpha raw") << QFINDTESTDATA("pic/4x4-alpha-uncompressed.pic") << QFINDTESTDATA("pic/4x4-alpha.png") << QString() << true
                                            << QImage::Format_ARGB32 << false;
    }

private Q_SLOTS:
    void initTestCase()
    {
        QCoreApplication::addLibraryPath(QStringLiteral(PLUGIN_DIR));
    }

    void testWrite_data()
    {
        common_data();

        // NB: 4x4-simple-color only uses solid red, blue, green and white,
        //     so there is no actual data loss in converting to RGB16.
        //     This just tests that the pic plugin can deal with different
        //     input formats.
        QTest::newRow("altered format") << QFINDTESTDATA("pic/4x4-simple-color.pic") << QFINDTESTDATA("pic/4x4-simple-color.png") << QString() << false
                                        << QImage::Format_RGB16 << true;
    }

    void testRead_data()
    {
        common_data();

        // TODO: test reading files with unusual channel setups
        //       (eg: one channel for each component)
    }

    void testWrite()
    {
        QFETCH(QString, picfile);
        QFETCH(QString, pngfile);
        QFETCH(QString, comment);
        QFETCH(QImage::Format, pngformat);
        QFETCH(bool, compress);

        QImageReader pngReader(pngfile, "png");
        QImage pngImage;
        QVERIFY2(pngReader.read(&pngImage), qPrintable(pngReader.errorString()));
        pngImage = pngImage.convertToFormat(pngformat);

        QFile expFile(picfile);
        QVERIFY2(expFile.open(QIODevice::ReadOnly), qPrintable(expFile.errorString()));
        QByteArray expData = expFile.readAll();

        QByteArray picData;
        QBuffer buffer(&picData);
        QImageWriter imgWriter(&buffer, "pic");
        imgWriter.setText(QStringLiteral("Description"), comment);
        imgWriter.setCompression(compress);
        imgWriter.write(pngImage);

        if (expData != picData) {
            QString fileNameBase = QUuid::createUuid().toString().remove(QLatin1Char('{')).remove(QLatin1Char('}'));
            QFile dumpFile(fileNameBase + QStringLiteral(".pic"));
            QVERIFY2(dumpFile.open(QIODevice::WriteOnly), qPrintable(dumpFile.errorString()));
            dumpFile.write(picData);
            QString msg =
                QStringLiteral("Written data (") + dumpFile.fileName() + QStringLiteral(") differed from expected data (") + picfile + QLatin1Char(')');
            QFAIL(qPrintable(msg));
        }
    }

    void testRead()
    {
        QFETCH(QString, picfile);
        QFETCH(QString, pngfile);
        QFETCH(bool, alpha);
        QFETCH(QImage::Format, pngformat);

        QImageReader inputReader(picfile, "pic");
        QImageReader expReader(pngfile, "png");

        QImage inputImage;
        QImage expImage;

        QVERIFY2(expReader.read(&expImage), qPrintable(expReader.errorString()));
        QVERIFY2(inputReader.read(&inputImage), qPrintable(inputReader.errorString()));

        QCOMPARE(inputImage.width(), expImage.width());
        QCOMPARE(inputImage.height(), expImage.height());
        QCOMPARE(inputImage.hasAlphaChannel(), alpha);
        QCOMPARE(inputImage.format(), alpha ? QImage::Format_ARGB32 : QImage::Format_RGB32);

        expImage = expImage.convertToFormat(pngformat);
        expImage = expImage.convertToFormat(alpha ? QImage::Format_ARGB32 : QImage::Format_RGB32);
        if (inputImage != expImage) {
            QString fileNameBase = QUuid::createUuid().toString().remove(QLatin1Char('{')).remove(QLatin1Char('}'));
            QFile picDumpFile(fileNameBase + QStringLiteral("-expected.data"));
            QVERIFY2(picDumpFile.open(QIODevice::WriteOnly), qPrintable(picDumpFile.errorString()));
            picDumpFile.write(reinterpret_cast<const char *>(inputImage.bits()), inputImage.sizeInBytes());
            QFile pngDumpFile(fileNameBase + QStringLiteral("-actual.data"));
            QVERIFY2(pngDumpFile.open(QIODevice::WriteOnly), qPrintable(pngDumpFile.errorString()));
            pngDumpFile.write(reinterpret_cast<const char *>(expImage.bits()), expImage.sizeInBytes());
            QString msg = QStringLiteral("Read image (") + picDumpFile.fileName() + QStringLiteral(") differed from expected image (") + pngDumpFile.fileName()
                + QLatin1Char(')');
            QFAIL(qPrintable(msg));
        }
    }

    void testPreReadComment_data()
    {
        testRead_data();
    }

    void testPreReadComment()
    {
        QFETCH(QString, picfile);
        QFETCH(QString, comment);

        QImageReader inputReader(picfile, "pic");

        QCOMPARE(inputReader.text(QStringLiteral("Description")), comment);
    }

    void testPreReadSize_data()
    {
        testRead_data();
    }

    void testPreReadSize()
    {
        QFETCH(QString, picfile);
        QFETCH(QString, pngfile);

        QImageReader inputReader(picfile, "pic");
        QImageReader expReader(pngfile, "png");

        QCOMPARE(inputReader.size(), expReader.size());
    }

    void testPreReadImageFormat_data()
    {
        testRead_data();
    }

    void testPreReadImageFormat()
    {
        QFETCH(QString, picfile);
        QFETCH(bool, alpha);

        QImageReader inputReader(picfile, "pic");

        QCOMPARE(inputReader.imageFormat(), alpha ? QImage::Format_ARGB32 : QImage::Format_RGB32);
    }
};

QTEST_MAIN(PicTests)

#include "pictest.moc"
