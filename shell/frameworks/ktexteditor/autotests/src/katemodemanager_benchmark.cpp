/*
    SPDX-FileCopyrightText: 2021 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katemodemanager_benchmark.h"

#include <katemodemanager.h>

#include <QString>
#include <QTest>

void KateModeManagerBenchmark::benchmarkWildcardsFind_data()
{
    wildcardsFindTestData();
}

void KateModeManagerBenchmark::benchmarkWildcardsFind()
{
    QFETCH(QString, fileName);
    QFETCH(QString, fileTypeName);

    // Warm up and check correctness.
    QCOMPARE(m_modeManager->wildcardsFind(fileName), fileTypeName);

    QBENCHMARK {
        m_modeManager->wildcardsFind(fileName);
    }
}

void KateModeManagerBenchmark::benchmarkMimeTypesFind_data()
{
    mimeTypesFindTestData();
}

void KateModeManagerBenchmark::benchmarkMimeTypesFind()
{
    QFETCH(QString, mimeTypeName);
    QFETCH(QString, fileTypeName);

    // Warm up and check correctness.
    QCOMPARE(m_modeManager->mimeTypesFind(mimeTypeName), fileTypeName);

    QBENCHMARK {
        m_modeManager->mimeTypesFind(mimeTypeName);
    }
}

QTEST_MAIN(KateModeManagerBenchmark)

#include "moc_katemodemanager_benchmark.cpp"
