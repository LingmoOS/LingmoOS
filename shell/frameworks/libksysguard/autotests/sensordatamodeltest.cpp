/*
    SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QAbstractItemModelTester>
#include <QTest>
#include <QTransposeProxyModel>

#include <QDBusInterface>

#include "SensorDataModel.h"
#include "Unit.h"

#define qs QStringLiteral

class SensorDataModelTest : public QObject
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
        KSysGuard::SensorDataModel model;
        QAbstractItemModelTester tester{&model};
        Q_UNUSED(tester)

        QVERIFY(model.columnCount() == 0);

        model.setSensors({qs("cpu/all/usage"), qs("memory/physical/used"), qs("network/all/download"), qs("disk/all/used")});

        QTRY_VERIFY(model.isReady());

        QCOMPARE(model.columnCount(), 4);

        auto id = KSysGuard::SensorDataModel::SensorId;
        auto unit = KSysGuard::SensorDataModel::Unit;

        QCOMPARE(model.headerData(0, Qt::Horizontal, id).toString(), qs("cpu/all/usage"));
        QCOMPARE(model.headerData(1, Qt::Horizontal, id).toString(), qs("memory/physical/used"));
        QCOMPARE(model.headerData(2, Qt::Horizontal, id).toString(), qs("network/all/download"));
        QCOMPARE(model.headerData(3, Qt::Horizontal, id).toString(), qs("disk/all/used"));

        // Verify that metadata is also loaded correctly. Not using names to sidestep translation issues.
        QCOMPARE(model.headerData(0, Qt::Horizontal, unit).value<KSysGuard::Unit>(), KSysGuard::UnitPercent);
        QCOMPARE(model.headerData(1, Qt::Horizontal, unit).value<KSysGuard::Unit>(), KSysGuard::UnitByte);
        QCOMPARE(model.headerData(2, Qt::Horizontal, unit).value<KSysGuard::Unit>(), KSysGuard::UnitByteRate);
        QCOMPARE(model.headerData(3, Qt::Horizontal, unit).value<KSysGuard::Unit>(), KSysGuard::UnitByte);

        model.setSensors({qs("disk/all/used"), qs("network/all/download"), qs("cpu/all/usage")});

        QVERIFY(!model.isReady());

        QTRY_VERIFY(model.isReady());

        QCOMPARE(model.columnCount(), 3);

        QCOMPARE(model.headerData(0, Qt::Horizontal, id).toString(), qs("disk/all/used"));
        QCOMPARE(model.headerData(1, Qt::Horizontal, id).toString(), qs("network/all/download"));
        QCOMPARE(model.headerData(2, Qt::Horizontal, id).toString(), qs("cpu/all/usage"));

        // Verify that metadata is also loaded correctly. Not using names to sidestep translation issues.
        QCOMPARE(model.headerData(0, Qt::Horizontal, unit).value<KSysGuard::Unit>(), KSysGuard::UnitByte);
        QCOMPARE(model.headerData(1, Qt::Horizontal, unit).value<KSysGuard::Unit>(), KSysGuard::UnitByteRate);
        QCOMPARE(model.headerData(2, Qt::Horizontal, unit).value<KSysGuard::Unit>(), KSysGuard::UnitPercent);
    }

    void testTransposeProxy()
    {
        KSysGuard::SensorDataModel model;
        QAbstractItemModelTester tester{&model};
        Q_UNUSED(tester)

        QTransposeProxyModel transpose;
        transpose.setSourceModel(&model);

        model.setSensors({qs("cpu/all/usage"), qs("memory/physical/used"), qs("network/all/download"), qs("disk/all/used")});

        QTRY_VERIFY(model.isReady());

        QCOMPARE(model.columnCount(), transpose.rowCount());
        QCOMPARE(model.rowCount(), transpose.columnCount());
    }
};

QTEST_MAIN(SensorDataModelTest);

#include "sensordatamodeltest.moc"
