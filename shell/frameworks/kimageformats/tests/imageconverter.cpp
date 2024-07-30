/*
    SPDX-FileCopyrightText: 2013 Alex Merry <alex.merry@kdemail.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <stdio.h>

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QImageReader>
#include <QImageWriter>
#include <QTextStream>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCoreApplication::addLibraryPath(QStringLiteral(PLUGIN_DIR));
    QCoreApplication::setApplicationName(QStringLiteral("imageconverter"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1.01.01.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Converts images from one format to another"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("in"), QStringLiteral("input image file"));
    parser.addPositionalArgument(QStringLiteral("out"), QStringLiteral("output image file"));
    QCommandLineOption informat(QStringList() << QStringLiteral("i") << QStringLiteral("informat"),
                                QStringLiteral("Image format for input file"),
                                QStringLiteral("format"));
    parser.addOption(informat);
    QCommandLineOption outformat(QStringList() << QStringLiteral("o") << QStringLiteral("outformat"),
                                 QStringLiteral("Image format for output file"),
                                 QStringLiteral("format"));
    parser.addOption(outformat);
    QCommandLineOption listformats(QStringList() << QStringLiteral("l") << QStringLiteral("list"), QStringLiteral("List supported image formats"));
    parser.addOption(listformats);

    QCommandLineOption listmimes(QStringList() << QStringLiteral("m") << QStringLiteral("listmime"), QStringLiteral("List supported image mime formats"));
    parser.addOption(listmimes);

    parser.process(app);

    const QStringList files = parser.positionalArguments();

    if (parser.isSet(listformats)) {
        QTextStream out(stdout);
        out << "Input formats:\n";
        const auto lstReaderSupportedFormats = QImageReader::supportedImageFormats();
        for (const QByteArray &fmt : lstReaderSupportedFormats) {
            out << "  " << fmt << '\n';
        }
        out << "Output formats:\n";
        const auto lstWriterSupportedFormats = QImageWriter::supportedImageFormats();
        for (const QByteArray &fmt : lstWriterSupportedFormats) {
            out << "  " << fmt << '\n';
        }
        return 0;
    }

    if (parser.isSet(listmimes)) {
        QTextStream out(stdout);
        out << "Input mime formats:\n";
        const auto lstReaderSupportedMimes = QImageReader::supportedMimeTypes();
        for (const QByteArray &fmt : lstReaderSupportedMimes) {
            out << "  " << fmt << '\n';
        }
        out << "Output mime formats:\n";
        const auto lstWriterSupportedMimes = QImageWriter::supportedMimeTypes();
        for (const QByteArray &fmt : lstWriterSupportedMimes) {
            out << "  " << fmt << '\n';
        }
        return 0;
    }

    if (files.count() != 2) {
        QTextStream(stdout) << "Must provide exactly two files\n";
        parser.showHelp(1);
    }
    QImageReader reader(files.at(0), parser.value(informat).toLatin1());
    QImage img = reader.read();
    if (img.isNull()) {
        QTextStream(stdout) << "Could not read image: " << reader.errorString() << '\n';
        return 2;
    }

    QImageWriter writer(files.at(1), parser.value(outformat).toLatin1());
    if (!writer.write(img)) {
        QTextStream(stdout) << "Could not write image: " << writer.errorString() << '\n';
        return 3;
    }

    return 0;
}
