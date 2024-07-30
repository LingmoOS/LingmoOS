/*
    This file is part of the KDE Frameworks

    SPDX-FileCopyrightText: 2022 Mirco Miranda

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kmemoryinfotest.h"

#include <QTest>

#include "kmemoryinfo.h"

QTEST_GUILESS_MAIN(KMemoryInfoTest)

KMemoryInfoTest::KMemoryInfoTest(QObject *parent)
    : QObject(parent)
{
}

void KMemoryInfoTest::isNull()
{
    KMemoryInfo m;
    QVERIFY(!m.isNull());
}

void KMemoryInfoTest::operators()
{
    KMemoryInfo m;
    auto m1 = m;
    QVERIFY(m == m1);

    // paranoia check
    QVERIFY(m.totalPhysical() != 0);
    QCOMPARE(m.totalPhysical(), m1.totalPhysical());
}

#include "moc_kmemoryinfotest.cpp"
