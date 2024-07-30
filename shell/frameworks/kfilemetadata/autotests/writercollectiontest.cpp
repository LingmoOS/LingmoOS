/*
    This file is part of the KDE KFileMetaData project
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2017  Igor Poboiko <igor.poboiko@gmail.com>
    SPDX-FileCopyrightText: 2019  Alexander Stippich <a.stippich@gmx.net>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <QObject>
#include <QTest>
#include <QDebug>

#include "writercollection.h"

namespace KFileMetaData {

class WriterCollectionTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testFetchWriters()
    {
        QCoreApplication::setLibraryPaths({QCoreApplication::applicationDirPath()});
        WriterCollection collection;
        QVERIFY(collection.fetchWriters(QStringLiteral("unknown/mimetype")).isEmpty());
        QVERIFY(!collection.fetchWriters(QStringLiteral("audio/mpeg")).isEmpty());
    }

};

}

QTEST_GUILESS_MAIN(KFileMetaData::WriterCollectionTest)

#include "writercollectiontest.moc"
