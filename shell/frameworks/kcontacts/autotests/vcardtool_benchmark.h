/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2016 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef VCARDTOOL_BENCHMARK_H
#define VCARDTOOL_BENCHMARK_H

#include <QObject>

class PerformanceTest : public QObject
{
    Q_OBJECT
public:
    explicit PerformanceTest(QObject *parent = nullptr);
    ~PerformanceTest() override;
private Q_SLOTS:
    void testParserPerformance();
    void testExportPerformance();
};

#endif
