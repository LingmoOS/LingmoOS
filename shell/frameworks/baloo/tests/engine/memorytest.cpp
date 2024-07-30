/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2014-2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QDebug>
#include <QUuid>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "document.h"

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addPositionalArgument(QStringLiteral("num"), QStringLiteral("The number of terms. Each term is of length 10"));
    parser.addOption(QCommandLineOption(QStringList () << QStringLiteral("p") << QStringLiteral("position"), QStringLiteral("Add positional information")));
    parser.addHelpOption();
    parser.process(app);

    QStringList args = parser.positionalArguments();
    if (args.size() != 1) {
        parser.showHelp(1);
    }

    Baloo::Document doc;
    int size = args.first().toInt();

    for (int i = 0; i < size; i++) {
        QByteArray term = QUuid::createUuid().toByteArray().mid(1, 10);

        if (parser.isSet(QStringLiteral("p"))) {
            doc.addPositionTerm(term, i);
        }
        else {
            doc.addTerm(term);
        }
    }

    qDebug() << "Added" << size << "terms";
    if (parser.isSet(QStringLiteral("p"))) {
        qDebug() << "With Positional Information";
    }
}
