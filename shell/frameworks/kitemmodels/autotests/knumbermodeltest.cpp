/*
    SPDX-FileCopyrightText: 2018 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QSignalSpy>
#include <QTest>

#include "test_model_helpers.h"
#include <knumbermodel.h>
using namespace TestModelHelpers;

Q_DECLARE_METATYPE(QModelIndex)

class tst_KNumberModel : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init()
    {
        QLocale::setDefault(QLocale::c());
    }

    void basicTest()
    {
        KNumberModel m;
        m.setMinimumValue(3);
        m.setMaximumValue(5);
        QCOMPARE(m.rowCount(), 3);
        QCOMPARE(m.data(m.index(0, 0), Qt::DisplayRole), QVariant("3"));
        QCOMPARE(m.data(m.index(1, 0), Qt::DisplayRole), QVariant("4"));
        QCOMPARE(m.data(m.index(2, 0), Qt::DisplayRole), QVariant("5"));
    }

    void testUpdates()
    {
        KNumberModel m;
        m.setMinimumValue(3);
        m.setMaximumValue(5);
        QSignalSpy resetSpy(&m, &QAbstractItemModel::modelReset);
        m.setMaximumValue(7);
        QVERIFY(resetSpy.count() == 1);
    }

    void testStep()
    {
        KNumberModel m;
        m.setMinimumValue(3);
        m.setMaximumValue(4);
        m.setStepSize(0.4);
        QCOMPARE(m.rowCount(), 3);
        QCOMPARE(m.data(m.index(2, 0), Qt::DisplayRole), QVariant("3.8"));
    }

    void testLocale()
    {
        KNumberModel m;
        m.setMinimumValue(1000);
        m.setMaximumValue(1000);
        QCOMPARE(m.rowCount(), 1);
        QCOMPARE(m.data(m.index(0, 0), Qt::DisplayRole), QVariant("1,000"));

        m.setFormattingOptions(QLocale::OmitGroupSeparator);
        QCOMPARE(m.data(m.index(0, 0), Qt::DisplayRole), QVariant("1000"));
    }

private:
};

QTEST_MAIN(tst_KNumberModel)

#include "knumbermodeltest.moc"
