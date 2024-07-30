/*
    Tests for KConfig Compiler

    SPDX-FileCopyrightText: 2005 Duncan Mac-Vicar <duncan@kde.org>
    SPDX-FileCopyrightText: 2009 Pino Toscano <pino@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONFIGCOMPILER_TEST_H
#define KCONFIGCOMPILER_TEST_H

#include <QFile>
#include <QObject>

class QString;

class KConfigCompiler_Test : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testBaselineComparison_data();
    void testBaselineComparison();
    void testRunning_data();
    void testRunning();

private:
    void appendFileDiff(const QString &oldFile, const QString &newFile);

    QString m_diffExe;
};

#endif
