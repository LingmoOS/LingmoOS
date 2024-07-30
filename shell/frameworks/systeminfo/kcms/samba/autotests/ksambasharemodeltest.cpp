/*
    SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include <QAbstractItemModelTester>
#include <QTest>

#include "../ksambasharemodel.h"

class KSambaShareModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testModel()
    {
        QAbstractItemModelTester tester(new KSambaShareModel, QAbstractItemModelTester::FailureReportingMode::QtTest);
    }
};

QTEST_GUILESS_MAIN(KSambaShareModelTest)

#include "ksambasharemodeltest.moc"
