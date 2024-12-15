// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DTextEncoding>

#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>

DCORE_USE_NAMESPACE

void convertFileEncoding(const QString &fromFile,
                         const QString &toFile,
                         const QByteArray &toEncoding,
                         const QByteArray &fromEncoding = QByteArray())
{
    QByteArray contentEncoding = fromEncoding;
    if (contentEncoding.isEmpty()) {
        bool isOk = false;
        contentEncoding = DTextEncoding::detectFileEncoding(fromFile, &isOk);
        if (!isOk) {
            qInfo().noquote() << QString("Detect file %1 encoding failed").arg(fromFile);
            return;
        }
    }

    QString errString;
    if (!DTextEncoding::convertFileEncodingTo(fromFile, toFile, toEncoding, contentEncoding, &errString)) {
        qInfo().noquote() << QString("Convert file %1 encoding from %2 to %3 failed. error: %4")
                                 .arg(fromFile)
                                 .arg(QString::fromUtf8(contentEncoding))
                                 .arg(QString::fromUtf8(toEncoding))
                                 .arg(errString);
    } else {
        qInfo().noquote() << QString("Convert file %1 encoding from %2 to %3 successed.")
                                 .arg(fromFile)
                                 .arg(QString::fromUtf8(contentEncoding))
                                 .arg(QString::fromUtf8(toEncoding));
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("Text codec");

    QCommandLineOption toEncodingOption({"t", "toEncoding"}, "Convert file encoding to specified encoding.", "encoding");
    QCommandLineOption fromEncodingOption({"f", "fromEncoding"}, "Convert file encoding from specified encoding.", "encoding");
    QCommandLineOption outputOption(
        {"o", "output"}, "Save converted text with file path, only supported when opening a single file.", "path");

    QCommandLineParser parser;
    parser.setApplicationDescription("Text codec, provide encoding detection and encoding conversion.");
    parser.addHelpOption();
    parser.addOption(toEncodingOption);
    parser.addOption(fromEncodingOption);
    parser.addOption(outputOption);
    parser.addPositionalArgument("file", "Open file.", "[file...]");
    parser.process(app);

    const QStringList args = parser.positionalArguments();
    if (args.isEmpty()) {
        parser.showHelp();
    }

    const QStringList fileArgs = parser.positionalArguments();
    if (fileArgs.isEmpty()) {
        qInfo().noquote() << "Not set open file.";
        return 0;
    }

    if (parser.isSet(outputOption)) {
        if (fileArgs.size() > 1) {
            qInfo().noquote() << "Output file path only supported when opening a single file.";
            return 0;
        } else if (!parser.isSet(toEncodingOption)) {
            qInfo().noquote() << "Convert file with not set convert encoding.";
        } else {
            QString fromFile = fileArgs.first();
            QString toFile = parser.value(outputOption);
            QByteArray toEncoding = parser.value(toEncodingOption).toUtf8();

            convertFileEncoding(fromFile, toFile, toEncoding, parser.value(fromEncodingOption).toUtf8());
            return 0;
        }
    }

    QByteArray toEncoding = parser.value(toEncodingOption).toUtf8();
    for (QString fileName : fileArgs) {
        if (toEncoding.isEmpty()) {
            // Only display file encoding.
            qInfo().noquote() << fileName << DTextEncoding::detectFileEncoding(fileName);
        } else {
            // Convert file encoding.
            convertFileEncoding(fileName, fileName, toEncoding, parser.value(fromEncodingOption).toUtf8());
        }
    }

    return 0;
}
