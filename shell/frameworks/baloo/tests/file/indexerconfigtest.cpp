/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fileindexerconfig.h"

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QMimeDatabase>

#include <iostream>

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addPositionalArgument(QStringLiteral("file"), QStringLiteral("The file url"));
    parser.process(app);

    if (parser.positionalArguments().isEmpty()) {
        parser.showHelp(1);
    }

    Baloo::FileIndexerConfig config;

    const QString arg = parser.positionalArguments().first();
    const QString url = QFileInfo(arg).absoluteFilePath();

    bool shouldIndex = config.shouldBeIndexed(url);

    QMimeDatabase m_mimeDb;
    QString mimetype = m_mimeDb.mimeTypeForFile(url).name();
    QString fastMimetype = m_mimeDb.mimeTypeForFile(url, QMimeDatabase::MatchExtension).name();

    bool shouldIndexMimetype = config.shouldMimeTypeBeIndexed(fastMimetype);
    std::cout << url.toUtf8().constData() << "\n"
              << "Should Index: " << std::boolalpha << shouldIndex << "\n"
              << "Should Index Mimetype: " << std::boolalpha << shouldIndexMimetype << "\n"
              << "Fast Mimetype: " << fastMimetype.toUtf8().constData() << std::endl
              << "Slow Mimetype: " << mimetype.toUtf8().constData() << std::endl;

    return 0; //app.exec();
}
