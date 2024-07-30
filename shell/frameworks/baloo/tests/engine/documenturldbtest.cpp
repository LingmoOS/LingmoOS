/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QDebug>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QElapsedTimer>
#include <functional>

#include "documenturldb.h"
#include "idutils.h"

using namespace Baloo;

int main(int /* argc */, char** /* argv */)
{
    MDB_env* env;
    MDB_txn* txn;

    QTemporaryDir tempDir;
    qDebug() << tempDir.path();

    mdb_env_create(&env);
    mdb_env_set_maxdbs(env, 2);
    mdb_env_set_mapsize(env, 1048576000);

    // The directory needs to be created before opening the environment
    QByteArray path = QFile::encodeName(tempDir.path());
    mdb_env_open(env, path.constData(), MDB_NOMEMINIT | MDB_WRITEMAP, 0664);
    mdb_txn_begin(env, nullptr, 0, &txn);

    {
        DocumentUrlDB db(IdTreeDB::create(txn), IdFilenameDB::create(txn), txn);
        QElapsedTimer timer;
        timer.start();

        uint num = 0;
        std::function<void (const QDir&, quint64)> recurse = [&db, &num, &recurse](const QDir& dir, quint64 dirId) -> void {
            const auto entryInfos = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries);
            for (const auto& entryInfo : entryInfos) {
                QByteArray path = QFile::encodeName(entryInfo.absoluteFilePath());
                auto entryId = filePathToId(path);
                db.put(entryId, dirId, path);
                num++;

                if ((num % 10000) == 0) {
                    qDebug() << num;
                }

                if (entryInfo.isSymLink()) {
                    // qDebug() << "skip" << entryInfo;
                } else if (entryInfo.isDir()) {
                    // qDebug() << "recurse" << entryInfo;
                    recurse(entryInfo.absoluteFilePath(), entryId);
                }
            }
        };

        QByteArray homePath = QFile::encodeName(QDir::homePath());
        recurse(QDir::home(), filePathToId(homePath));

        mdb_txn_commit(txn);

        qDebug() << "Done" << timer.elapsed() << "msecs";
    }

    // Cleanup
    //mdb_txn_abort(txn);
    mdb_env_close(env);

    return 0;
}
