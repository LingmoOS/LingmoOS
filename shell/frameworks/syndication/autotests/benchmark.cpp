/*
    This file is part of LibSyndication.
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "documentsource.h"
#include "feed.h"
#include "parsercollection.h"
#include "specificdocument.h"

#include <QFile>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QTime>

#include <cstdio>
#include <iostream>

using namespace Syndication;

int main(int argc, char **argv)
{
    QString filename(argv[1]);
    QString arg1(argv[1]);

    /*
    if (filename == "--specific-format")
    {
        if (argc < 3)
        {
            printUsage("filename expected");
            return 1;
        }
        filename = QString(argv[2]);
        specificformat = true;
    }
    */

    int totalLength = 0;
    int numberOfFiles = 0;

    QStringList filenames;

    QFile input(arg1);
    input.open(QIODevice::ReadOnly);
    QTextStream stream(&input);

    while (!stream.atEnd()) {
        filenames += stream.readLine();
    }
    input.close();

    QTime time;
    time.start();

    for (const QString &filename : std::as_const(filenames)) {
        QFile f(filename);

        if (!f.open(QIODevice::ReadOnly)) {
            continue;
        }

        DocumentSource src(f.readAll(), "http://libsyndicationtest");
        f.close();

        FeedPtr ptr(Syndication::parse(src));
        if (ptr) {
            totalLength += src.size();
            ++numberOfFiles;
            QString dbg = ptr->debugInfo();
        }

        // std::cerr << "size of debug output: " << dbg.length() << std::endl;
    }

    int timeSpent = time.elapsed();

    std::cout << "total number of files: " << numberOfFiles << std::endl;
    std::cout << "total length of source (bytes): " << totalLength << std::endl;
    std::cout << "avg length (bytes): " << (double)totalLength / numberOfFiles << std::endl;
    std::cout << "time needed in total (ms): " << timeSpent << std::endl;
    std::cout << "source bytes per second: " << (totalLength * 1000.0) / timeSpent << std::endl;
    std::cout << "files per second: " << ((double)numberOfFiles * 1000) / timeSpent << std::endl;
    return 0;
}
