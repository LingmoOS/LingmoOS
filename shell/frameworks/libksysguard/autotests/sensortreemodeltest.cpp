/*
    SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QAbstractItemModelTester>
#include <QTest>

#include <QDBusInterface>

#include "SensorTreeModel.h"

class SensorTreeModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        QDBusInterface interface{QStringLiteral("org.kde.ksystemstats"), QStringLiteral("/")};
        if (!interface.isValid()) {
            QSKIP("KSystemStats Daemon is not running");
        }
    }

    void testModel()
    {
        KSysGuard::SensorTreeModel model;
        QAbstractItemModelTester tester(&model);
        Q_UNUSED(tester)

        QVERIFY(model.rowCount() == 0);

        QTRY_VERIFY(model.rowCount() > 0);
    }
};

QTEST_MAIN(SensorTreeModelTest);

#include "sensortreemodeltest.moc"
