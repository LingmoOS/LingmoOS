/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QElapsedTimer>
#include <iostream>

#include "filtereddiriterator.h"
#include "fileindexerconfig.h"
#include "util.h"

using namespace Baloo;

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addPositionalArgument(QStringLiteral("folder"), QStringLiteral("Folder to test on"), QStringLiteral("folderName"));
    parser.process(app);

    std::unique_ptr<FileIndexerConfig> config;

    QStringList includeFolders;
    if (!parser.positionalArguments().isEmpty()) {
        QString folder = parser.positionalArguments().first();
        includeFolders << QFileInfo(folder).absoluteFilePath();
    } else {
        config.reset(new FileIndexerConfig);
        includeFolders = config->includeFolders();
    }
    QElapsedTimer timer;
    timer.start();

    int num = 0;
    for (const QString& dir : includeFolders) {
        FilteredDirIterator it(config.get(), dir);
        while (!it.next().isEmpty()) {
            num++;
        }
    }

    std::cout << "Num Files: " << num << std::endl;
    std::cout << "Elapsed: " << timer.elapsed() << std::endl;
    printIOUsage();

    return 0;
}
