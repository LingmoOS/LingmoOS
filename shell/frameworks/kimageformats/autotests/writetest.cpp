/*
    SPDX-FileCopyrightText: 2014 Alex Merry <alex.merry@kdemail.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <stdio.h>

#include <QBuffer>
#include <QColorSpace>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QImageWriter>
#include <QTextStream>

#include "fuzzyeq.cpp"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCoreApplication::removeLibraryPath(QStringLiteral(PLUGIN_DIR));
    QCoreApplication::addLibraryPath(QStringLiteral(PLUGIN_DIR));
    QCoreApplication::setApplicationName(QStringLiteral("writetest"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1.1.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Performs basic image conversion checking."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("format"), QStringLiteral("format to test."));
    QCommandLineOption lossless(QStringList() << QStringLiteral("l") << QStringLiteral("lossless"),
                                QStringLiteral("Check that reading back the data gives the same image."));
    QCommandLineOption ignoreDataCheck({QStringLiteral("no-data-check")}, QStringLiteral("Don't check that write data is exactly the same."));
    QCommandLineOption fuzz(QStringList() << QStringLiteral("f") << QStringLiteral("fuzz"),
                            QStringLiteral("Allow for some deviation in ARGB data."),
                            QStringLiteral("max"));
    parser.addOption(lossless);
    parser.addOption(ignoreDataCheck);
    parser.addOption(fuzz);

    parser.process(app);

    const QStringList args = parser.positionalArguments();
    if (args.count() < 1) {
        QTextStream(stderr) << "Must provide a format\n";
        parser.showHelp(1);
    } else if (args.count() > 1) {
        QTextStream(stderr) << "Too many arguments\n";
        parser.showHelp(1);
    }

    uchar fuzziness = 0;
    if (parser.isSet(fuzz)) {
        bool ok;
        uint fuzzarg = parser.value(fuzz).toUInt(&ok);
        if (!ok || fuzzarg > 255) {
            QTextStream(stderr) << "Error: max fuzz argument must be a number between 0 and 255\n";
            parser.showHelp(1);
        }
        fuzziness = uchar(fuzzarg);
    }

    QString suffix = args.at(0);
    QByteArray format = suffix.toLatin1();

    QDir imgdir(QStringLiteral(IMAGEDIR));
    if (parser.isSet(ignoreDataCheck)) {
        imgdir.setNameFilters({QLatin1String("*.png")});
    } else {
        imgdir.setNameFilters(QStringList(QLatin1String("*.") + suffix));
    }
    imgdir.setFilter(QDir::Files);

    int passed = 0;
    int failed = 0;

    QTextStream(stdout) << "********* "
                        << "Starting basic write tests for " << suffix << " images *********\n";
    const QFileInfoList lstImgDir = imgdir.entryInfoList();
    for (const QFileInfo &fi : lstImgDir) {
        QString pngfile;
        if (parser.isSet(ignoreDataCheck)) {
            pngfile = fi.filePath();
        } else {
            int suffixPos = fi.filePath().size() - suffix.size();
            pngfile = fi.filePath().replace(suffixPos, suffix.size(), QStringLiteral("png"));
        }
        QString pngfilename = QFileInfo(pngfile).fileName();

        QImageReader pngReader(pngfile, "png");
        QImage pngImage;
        if (!pngReader.read(&pngImage)) {
            QTextStream(stdout) << "ERROR: " << fi.fileName() << ": could not load " << pngfilename << ": " << pngReader.errorString() << "\n";
            ++failed;
            continue;
        }

        QByteArray writtenData;
        {
            QBuffer buffer(&writtenData);
            QImageWriter imgWriter(&buffer, format.constData());
            if (parser.isSet(lossless)) {
                imgWriter.setQuality(100);
            }
            if (!imgWriter.write(pngImage)) {
                QTextStream(stdout) << "FAIL : " << fi.fileName() << ": failed to write image data\n";
                ++failed;
                continue;
            }
        }

        if (!parser.isSet(ignoreDataCheck)) {
            QFile expFile(fi.filePath());
            if (!expFile.open(QIODevice::ReadOnly)) {
                QTextStream(stdout) << "ERROR: " << fi.fileName() << ": could not open " << fi.fileName() << ": " << expFile.errorString() << "\n";
                ++failed;
                continue;
            }
            QByteArray expData = expFile.readAll();
            if (expData.isEmpty()) {
                // check if there was actually anything to read
                expFile.reset();
                char buf[1];
                qint64 result = expFile.read(buf, 1);
                if (result < 0) {
                    QTextStream(stdout) << "ERROR: " << fi.fileName() << ": could not load " << fi.fileName() << ": " << expFile.errorString() << "\n";
                    ++failed;
                    continue;
                }
            }

            if (expData != writtenData) {
                QTextStream(stdout) << "FAIL : " << fi.fileName() << ": written data differs from " << fi.fileName() << "\n";
                ++failed;
                continue;
            }
        }

        QImage reReadImage;
        {
            QBuffer buffer(&writtenData);
            QImageReader imgReader(&buffer, format.constData());
            if (!imgReader.read(&reReadImage)) {
                QTextStream(stdout) << "FAIL : " << fi.fileName() << ": could not read back the written data\n";
                ++failed;
                continue;
            }
            if (reReadImage.colorSpace().isValid()) {
                QColorSpace toColorSpace;
                if (pngImage.colorSpace().isValid()) {
                    reReadImage.convertToColorSpace(pngImage.colorSpace());
                } else {
                    reReadImage.convertToColorSpace(QColorSpace(QColorSpace::SRgb));
                }
            }
            reReadImage = reReadImage.convertToFormat(pngImage.format());
        }

        if (parser.isSet(lossless)) {
            if (!fuzzyeq(pngImage, reReadImage, fuzziness)) {
                QTextStream(stdout) << "FAIL : " << fi.fileName() << ": re-reading the data resulted in a different image\n";
                if (pngImage.size() == reReadImage.size()) {
                    for (int i = 0; i < pngImage.width(); ++i) {
                        for (int j = 0; j < pngImage.height(); ++j) {
                            if (pngImage.pixel(i, j) != reReadImage.pixel(i, j)) {
                                QTextStream(stdout) << "Pixel is different " << i << ',' << j << ' ' << pngImage.pixel(i, j) << ' ' << reReadImage.pixel(i, j)
                                                    << '\n';
                            }
                        }
                    }
                }
                ++failed;
                continue;
            }
        }

        QTextStream(stdout) << "PASS : " << fi.fileName() << "\n";
        ++passed;
    }

    QTextStream(stdout) << "Totals: " << passed << " passed, " << failed << " failed\n";
    QTextStream(stdout) << "********* "
                        << "Finished basic write tests for " << suffix << " images *********\n";

    return failed == 0 ? 0 : 1;
}
