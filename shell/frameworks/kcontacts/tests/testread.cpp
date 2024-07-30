/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2007 KDE-PIM team <kde-pim@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <iostream>

#include <QFile>

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>

#include "converter/kcontacts/vcardconverter.h"
#include "vcard_p.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("vcard21"), QStringLiteral("vCard 2.1")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("+inputfile"), QStringLiteral("Input file")));

    parser.addVersionOption();
    parser.addHelpOption();
    parser.process(app);

    if (parser.positionalArguments().count() != 1) {
        std::cerr << "Missing argument" << std::endl;
        return 1;
    }

    QString inputFile(parser.positionalArguments().at(0));

    QFile file(inputFile);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug("Unable to open file '%s' for reading!", qPrintable(file.fileName()));
        return 1;
    }

    QByteArray text = file.readAll();
    file.close();

    KContacts::VCardConverter converter;
    KContacts::Addressee::List list = converter.parseVCards(text);

    if (parser.isSet(QStringLiteral("vcard21"))) {
        text = converter.createVCards(list, KContacts::VCardConverter::v2_1); // uses version 2.1
    } else {
        text = converter.createVCards(list); // uses version 3.0
    }

    std::cout << text.data();

    return 0;
}
