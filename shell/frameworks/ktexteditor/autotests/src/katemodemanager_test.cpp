/*
    SPDX-FileCopyrightText: 2021 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katemodemanager_test.h"

#include <katemodemanager.h>

#include <QString>
#include <QTest>

void KateModeManagerTest::testWildcardsFind_data()
{
    wildcardsFindTestData();
}

void KateModeManagerTest::testWildcardsFind()
{
    QFETCH(QString, fileName);
    QFETCH(QString, fileTypeName);

    QCOMPARE(m_modeManager->wildcardsFind(fileName), fileTypeName);
}

void KateModeManagerTest::testMimeTypesFind_data()
{
    mimeTypesFindTestData();
}

void KateModeManagerTest::testMimeTypesFind()
{
    QFETCH(QString, mimeTypeName);
    QFETCH(QString, fileTypeName);

    QCOMPARE(m_modeManager->mimeTypesFind(mimeTypeName), fileTypeName);
}

QTEST_MAIN(KateModeManagerTest)

#include "moc_katemodemanager_test.cpp"
