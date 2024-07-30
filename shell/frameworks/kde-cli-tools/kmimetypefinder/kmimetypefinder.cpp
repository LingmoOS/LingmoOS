/*
 *  SPDX-FileCopyrightText: 2002 David Faure <faure@kde.org>
 *  SPDX-FileCopyrightText: 2008 Pino Toscano <pino@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <KAboutData>
#include <KLocalizedString>

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFile>
#include <QMimeDatabase>
#include <QMimeType>

#include <stdio.h>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    KLocalizedString::setApplicationDomain(QByteArrayLiteral("kmimetypefinder"));

    KAboutData aboutData(QLatin1String("kmimetypefinder"), i18n("MIME Type Finder"), QLatin1String(PROJECT_VERSION));
    aboutData.setShortDescription(i18n("Gives the MIME type for a given file"));
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    parser.addOption(
        QCommandLineOption(QStringList() << QLatin1String("c") << QLatin1String("content"), i18n("Use only the file content for determining the MIME type.")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("f") << QLatin1String("filename-only"),
                                        i18n("Whether use the file name only for determining the MIME type. Not used if -c is specified.")));
    parser.addPositionalArgument(QLatin1String("filename"), i18n("The filename to test. '-' to read from stdin."));

    parser.process(app);
    aboutData.processCommandLine(&parser);

    if (parser.positionalArguments().count() < 1) {
        printf("No filename specified\n");
        return 1;
    }
    const QString fileName = parser.positionalArguments().at(0);
    QMimeDatabase db;
    QMimeType mime;
    if (fileName == QLatin1String("-")) {
        QFile qstdin;
        qstdin.open(stdin, QIODevice::ReadOnly);
        const QByteArray data = qstdin.readAll();
        mime = db.mimeTypeForData(data);
    } else if (parser.isSet(QStringLiteral("c"))) {
        mime = db.mimeTypeForFile(fileName, QMimeDatabase::MatchContent);
    } else if (parser.isSet(QStringLiteral("f"))) {
        mime = db.mimeTypeForFile(fileName, QMimeDatabase::MatchExtension);
    } else {
        mime = db.mimeTypeForFile(fileName);
    }
    if (!mime.isDefault()) {
        printf("%s\n", mime.name().toLatin1().constData());
    } else {
        return 1; // error
    }

    return 0;
}
