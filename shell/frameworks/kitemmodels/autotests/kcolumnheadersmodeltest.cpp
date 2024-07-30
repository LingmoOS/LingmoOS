/*
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QAbstractItemModelTester>
#include <QSignalSpy>
#include <QStandardItemModel>
#include <QTest>

#include <KColumnHeadersModel>

class KColumnHeadersModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testStatic()
    {
        auto model = new KColumnHeadersModel{this};

        auto sourceModel = new QStandardItemModel{this};
        sourceModel->setHorizontalHeaderLabels(
            {QStringLiteral("Test 1"), QStringLiteral("Test 2"), QStringLiteral("Test 3"), QStringLiteral("Test 4"), QStringLiteral("Test 5")});

        model->setSourceModel(sourceModel);

        new QAbstractItemModelTester(model, this);

        QCOMPARE(model->rowCount(), 5);
        QCOMPARE(model->data(model->index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("Test 1"));
        QCOMPARE(model->data(model->index(1, 0), Qt::DisplayRole).toString(), QStringLiteral("Test 2"));
        QCOMPARE(model->data(model->index(2, 0), Qt::DisplayRole).toString(), QStringLiteral("Test 3"));
        QCOMPARE(model->data(model->index(3, 0), Qt::DisplayRole).toString(), QStringLiteral("Test 4"));
        QCOMPARE(model->data(model->index(4, 0), Qt::DisplayRole).toString(), QStringLiteral("Test 5"));

        QSignalSpy spy{model, &QAbstractItemModel::dataChanged};
        QVERIFY(spy.isValid());

        sourceModel->setHorizontalHeaderLabels(
            {QStringLiteral("Test 5"), QStringLiteral("Test 4"), QStringLiteral("Test 3"), QStringLiteral("Test 2"), QStringLiteral("Test 1")});

        QCOMPARE(spy.count(), 4);
    }

    void testAddColumns()
    {
        auto model = new KColumnHeadersModel{this};
        auto sourceModel = new QStandardItemModel{this};
        sourceModel->setHorizontalHeaderLabels({QStringLiteral("Test 1"), QStringLiteral("Test 2")});
        model->setSourceModel(sourceModel);

        new QAbstractItemModelTester(model, this);

        QSignalSpy spy{model, &QAbstractItemModel::rowsInserted};
        QVERIFY(spy.isValid());

        QCOMPARE(model->rowCount(), 2);
        QCOMPARE(model->data(model->index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("Test 1"));
        QCOMPARE(model->data(model->index(1, 0), Qt::DisplayRole).toString(), QStringLiteral("Test 2"));

        sourceModel->setHorizontalHeaderLabels({QStringLiteral("Test 1"), QStringLiteral("Test 2"), QStringLiteral("Test 3")});

        QCOMPARE(spy.count(), 1);
        QCOMPARE(model->rowCount(), 3);
        QCOMPARE(model->data(model->index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("Test 1"));
        QCOMPARE(model->data(model->index(1, 0), Qt::DisplayRole).toString(), QStringLiteral("Test 2"));
        QCOMPARE(model->data(model->index(2, 0), Qt::DisplayRole).toString(), QStringLiteral("Test 3"));

        sourceModel->setHorizontalHeaderLabels(
            {QStringLiteral("Test 1"), QStringLiteral("Test 2"), QStringLiteral("Test 3"), QStringLiteral("Test 4"), QStringLiteral("Test 5")});

        QCOMPARE(spy.count(), 2);
        QCOMPARE(model->rowCount(), 5);
        QCOMPARE(model->data(model->index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("Test 1"));
        QCOMPARE(model->data(model->index(1, 0), Qt::DisplayRole).toString(), QStringLiteral("Test 2"));
        QCOMPARE(model->data(model->index(2, 0), Qt::DisplayRole).toString(), QStringLiteral("Test 3"));
        QCOMPARE(model->data(model->index(3, 0), Qt::DisplayRole).toString(), QStringLiteral("Test 4"));
        QCOMPARE(model->data(model->index(4, 0), Qt::DisplayRole).toString(), QStringLiteral("Test 5"));

        sourceModel->setHorizontalHeaderLabels({QStringLiteral("Test 1"),
                                                QStringLiteral("Test 2"),
                                                QStringLiteral("Test 6"),
                                                QStringLiteral("Test 3"),
                                                QStringLiteral("Test 4"),
                                                QStringLiteral("Test 5")});

        QCOMPARE(spy.count(), 3);
        QCOMPARE(model->rowCount(), 6);
        QCOMPARE(model->data(model->index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("Test 1"));
        QCOMPARE(model->data(model->index(1, 0), Qt::DisplayRole).toString(), QStringLiteral("Test 2"));
        QCOMPARE(model->data(model->index(2, 0), Qt::DisplayRole).toString(), QStringLiteral("Test 6"));
        QCOMPARE(model->data(model->index(3, 0), Qt::DisplayRole).toString(), QStringLiteral("Test 3"));
        QCOMPARE(model->data(model->index(4, 0), Qt::DisplayRole).toString(), QStringLiteral("Test 4"));
        QCOMPARE(model->data(model->index(5, 0), Qt::DisplayRole).toString(), QStringLiteral("Test 5"));
    }

    void testRemoveColumns()
    {
        auto model = new KColumnHeadersModel{this};

        auto sourceModel = new QStandardItemModel{this};
        sourceModel->setHorizontalHeaderLabels(
            {QStringLiteral("Test 1"), QStringLiteral("Test 2"), QStringLiteral("Test 3"), QStringLiteral("Test 4"), QStringLiteral("Test 5")});

        model->setSourceModel(sourceModel);

        new QAbstractItemModelTester(model, this);

        QSignalSpy spy{model, &QAbstractItemModel::rowsRemoved};
        QVERIFY(spy.isValid());

        QCOMPARE(model->rowCount(), 5);
        QCOMPARE(model->data(model->index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("Test 1"));
        QCOMPARE(model->data(model->index(1, 0), Qt::DisplayRole).toString(), QStringLiteral("Test 2"));
        QCOMPARE(model->data(model->index(2, 0), Qt::DisplayRole).toString(), QStringLiteral("Test 3"));
        QCOMPARE(model->data(model->index(3, 0), Qt::DisplayRole).toString(), QStringLiteral("Test 4"));
        QCOMPARE(model->data(model->index(4, 0), Qt::DisplayRole).toString(), QStringLiteral("Test 5"));

        sourceModel->takeColumn(4);

        QCOMPARE(spy.count(), 1);

        QCOMPARE(model->rowCount(), 4);
        QCOMPARE(model->data(model->index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("Test 1"));
        QCOMPARE(model->data(model->index(1, 0), Qt::DisplayRole).toString(), QStringLiteral("Test 2"));
        QCOMPARE(model->data(model->index(2, 0), Qt::DisplayRole).toString(), QStringLiteral("Test 3"));
        QCOMPARE(model->data(model->index(3, 0), Qt::DisplayRole).toString(), QStringLiteral("Test 4"));

        sourceModel->takeColumn(1);

        QCOMPARE(spy.count(), 2);

        QCOMPARE(model->rowCount(), 3);
        QCOMPARE(model->data(model->index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("Test 1"));
        QCOMPARE(model->data(model->index(1, 0), Qt::DisplayRole).toString(), QStringLiteral("Test 3"));
        QCOMPARE(model->data(model->index(2, 0), Qt::DisplayRole).toString(), QStringLiteral("Test 4"));
    }
};

QTEST_MAIN(KColumnHeadersModelTest)

#include "kcolumnheadersmodeltest.moc"
