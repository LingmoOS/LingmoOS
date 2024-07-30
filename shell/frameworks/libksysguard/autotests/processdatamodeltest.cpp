/*
    SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QAbstractItemModelTester>
#include <QTest>

#include "process_data_model.h"

#define qs QStringLiteral

class ProcessDataModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testModel()
    {
        auto model = new KSysGuard::ProcessDataModel();
        auto tester = new QAbstractItemModelTester(model);

        model->setEnabledAttributes({qs("pid"), qs("name"), qs("usage"), qs("vmPSS")});

        QTest::qWait(100);

        QVERIFY(model->rowCount() > 0);
    }
};

QTEST_MAIN(ProcessDataModelTest);

#include "processdatamodeltest.moc"
