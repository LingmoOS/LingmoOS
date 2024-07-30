/*
    SPDX-FileCopyrightText: 2021 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTEXTEDITOR_KATEMODEMANAGER_BENCHMARK_H
#define KTEXTEDITOR_KATEMODEMANAGER_BENCHMARK_H

#include "katemodemanager_test_base.h"

class KateModeManagerBenchmark : public KateModeManagerTestBase
{
    Q_OBJECT
private Q_SLOTS:
    void benchmarkWildcardsFind_data();
    void benchmarkWildcardsFind();
    void benchmarkMimeTypesFind_data();
    void benchmarkMimeTypesFind();
};

#endif // KTEXTEDITOR_KATEMODEMANAGER_BENCHMARK_H
