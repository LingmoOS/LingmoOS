/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "unindexedfileiterator.h"
#include "fileindexerconfig.h"

#include "database.h"
#include "transaction.h"

#include <QCoreApplication>
#include <QTemporaryDir>
#include <QElapsedTimer>

using namespace Baloo;

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    QTemporaryDir tempDir;
    qDebug() << tempDir.path();

    Database db(tempDir.path());
    db.open(Baloo::Database::CreateDatabase);

    Transaction tr(db, Transaction::ReadWrite);

    FileIndexerConfig config;

    QElapsedTimer timer;
    timer.start();

    UnIndexedFileIterator it(&config, &tr, QDir::homePath());
    while (!it.next().isEmpty()) {
        ;
    }

    qDebug() << "Done" << timer.elapsed() << "msecs";
    return 0;
}
