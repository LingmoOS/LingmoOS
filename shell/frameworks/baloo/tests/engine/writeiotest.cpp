/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2014-2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QDebug>
#include <QTemporaryDir>
#include <QUuid>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "database.h"
#include "document.h"
#include "transaction.h"
#include "tests/file/util.h"

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

    QTemporaryDir tempDir;
    tempDir.setAutoRemove(false);

    Baloo::Database db(tempDir.path());
    db.open(Baloo::Database::CreateDatabase);

    qDebug() << tempDir.path();
    printIOUsage();
    qDebug() << "Creating the document";

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
    doc.setParentId(1);
    doc.setId(2);

    Baloo::Transaction tr(db, Baloo::Transaction::ReadWrite);
    tr.addDocument(doc);
    tr.commit();

    printIOUsage();

    int dbSize = 0;
    QDir dbDir(tempDir.path());
    for (const QFileInfo& file : dbDir.entryInfoList(QDir::Files)) {
        qDebug() << file.fileName() << file.size() / 1024 << "kb";
        dbSize += file.size();

    }
    qDebug() << "Database Size:" << dbSize / 1024 << "kb";
}
