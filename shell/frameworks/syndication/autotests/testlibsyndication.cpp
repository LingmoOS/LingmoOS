/*
    This file is part of LibSyndication.
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "documentsource.h"
#include "feed.h"
#include "parsercollection.h"
#include "specificdocument.h"

#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QString>

#include <iostream>

using namespace Syndication;

void printUsage(const QString &error)
{
    std::cerr << "testlibsyndication - (C) Frank Osterfeld 2006" << std::endl;
    std::cerr << std::endl;
    if (!error.isNull()) {
        std::cerr << error.toUtf8().data() << std::endl;
        std::cerr << std::endl;
    } else {
        std::cerr << "Prints the parsed content of a feed file to standard output." << std::endl;
        std::cerr << std::endl;
    }
    std::cerr << "Usage: testlibsyndication [--specific-format] <file> [--compare <expectedfile>]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "--specific-format: If set, the debug output for the specific" << std::endl;
    std::cerr << "feed format is printed to stdout, otherwise the debug output" << std::endl;
    std::cerr << "for the abstraction" << std::endl;
}

int main(int argc, char **argv)
{
    qputenv("LC_ALL", "C");

    int pcompare = 2;
    if (argc < 2) {
        printUsage(QStringLiteral("filename expected"));
        return 1;
    }

    QString filename(QFile::decodeName(argv[1]));

    bool specificformat = false;

    if (filename == QLatin1String("--specific-format")) {
        if (argc < 3) {
            printUsage(QStringLiteral("filename expected"));
            return 1;
        }
        filename = QFile::decodeName(argv[2]);
        specificformat = true;
        pcompare += 1;
    }

    QString expfname;

    if (argc >= (pcompare + 1) //
        && QString::fromLatin1(argv[pcompare]) == QLatin1String("--compare")) {
        expfname = QString::fromLatin1(argv[pcompare + 1]);
    }

    QFile f(filename);

    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        printUsage(QStringLiteral("Couldn't open file"));
        return 1;
    }

    DocumentSource src(f.readAll(), QStringLiteral("http://libsyndicationtest"));
    f.close();

    FeedPtr ptr(Syndication::parse(src));

    if (ptr == nullptr) {
        printUsage(QStringLiteral("Couldn't parse file: (%1)").arg(Syndication::parserCollection()->lastError()));
        return 1;
    }

    QString res = specificformat ? ptr->specificDocument()->debugInfo() : ptr->debugInfo();
#if 0
    QFile headerFile(QStringLiteral("/tmp/bb.txt"));
    headerFile.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream outHeaderStream(&headerFile);
    outHeaderStream << res.toLatin1().trimmed();
    headerFile.close();
#endif
    if (expfname.isNull()) {
        std::cout << res.toUtf8().data() << std::endl;
        return 0;
    } else {
        QFile expFile(expfname);
        expFile.open(QIODevice::ReadOnly | QIODevice::Text);
        QByteArray expected = expFile.readAll();
        expFile.close();
        if (expected.trimmed() != res.toUtf8().trimmed()) {
            qDebug() << "Obtained:\n" << res.toUtf8().trimmed();
            qDebug() << "Expected:\n" << expected.trimmed();
            return 1;
        }
        return 0;
    }

    return 0;
}
