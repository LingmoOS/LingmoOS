/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fileindexerconfigutils.h"
#include "filtereddiriterator.h"
#include "fileindexerconfig.h"

#include <QTest>

class UnIndexedFileIterator : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test();
};

using namespace Baloo;

void UnIndexedFileIterator::test()
{
    // Bah!!
    // Testing this is complex!
    // FIXME: How in the world should I test this?
}

QTEST_GUILESS_MAIN(UnIndexedFileIterator)

#include "unindexedfileiteratortest.moc"
