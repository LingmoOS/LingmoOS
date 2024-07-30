/*
    SPDX-FileCopyrightText: 2015 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: David Faure <david.faure@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QAbstractItemModelTester>
#include <QDebug>
#include <QSignalSpy>
#include <QStandardItemModel>
#include <QTest>

#include "test_model_helpers.h"
#include <krearrangecolumnsproxymodel.h>
using namespace TestModelHelpers;

Q_DECLARE_METATYPE(QModelIndex)

class tst_KRearrangeColumnsProxyModel : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void initTestCase()
    {
        qRegisterMetaType<QModelIndex>();
    }

    void init()
    {
        // Prepare the source model to use later on
        mod.clear();
        mod.appendRow(makeStandardItems(QStringList() << QStringLiteral("A") << QStringLiteral("B") << QStringLiteral("C") << QStringLiteral("D")
                                                      << QStringLiteral("E")));
        mod.item(0, 0)->appendRow(makeStandardItems(QStringList() << QStringLiteral("m") << QStringLiteral("n") << QStringLiteral("o") << QStringLiteral("p")
                                                                  << QStringLiteral("-")));
        mod.item(0, 0)->appendRow(makeStandardItems(QStringList() << QStringLiteral("q") << QStringLiteral("r") << QStringLiteral("s") << QStringLiteral("t")
                                                                  << QStringLiteral("-")));
        mod.appendRow(makeStandardItems(QStringList() << QStringLiteral("E") << QStringLiteral("F") << QStringLiteral("G") << QStringLiteral("H")
                                                      << QStringLiteral("I")));
        mod.item(1, 0)->appendRow(makeStandardItems(QStringList() << QStringLiteral("x") << QStringLiteral("y") << QStringLiteral("z") << QStringLiteral(".")
                                                                  << QStringLiteral("-")));
        mod.setHorizontalHeaderLabels(QStringList() << QStringLiteral("H1") << QStringLiteral("H2") << QStringLiteral("H3") << QStringLiteral("H4")
                                                    << QStringLiteral("H5"));

        QCOMPARE(extractRowTexts(&mod, 0), QStringLiteral("ABCDE"));
        QCOMPARE(extractRowTexts(&mod, 0, mod.index(0, 0)), QStringLiteral("mnop-"));
        QCOMPARE(extractRowTexts(&mod, 1, mod.index(0, 0)), QStringLiteral("qrst-"));
        QCOMPARE(extractRowTexts(&mod, 1), QStringLiteral("EFGHI"));
        QCOMPARE(extractRowTexts(&mod, 0, mod.index(1, 0)), QStringLiteral("xyz.-"));
        QCOMPARE(extractHorizontalHeaderTexts(&mod), QStringLiteral("H1H2H3H4H5"));

        // test code to see the model
        // showModel(&mod);
    }

    void shouldShowNothingIfNoSourceModel()
    {
        // Given a rearrange-columns proxy with no source model
        KRearrangeColumnsProxyModel pm;

        // Then the proxy should show nothing (no columns selected)
        QCOMPARE(pm.rowCount(), 0);
        QCOMPARE(pm.columnCount(), 0);
        QVERIFY(!pm.index(0, 0).isValid());
    }

    void shouldShowNothingIfNoColumnSelection()
    {
        // Given a rearrange-columns proxy
        KRearrangeColumnsProxyModel pm;

        // When setting it to a source model
        pm.setSourceModel(&mod);

        // Then the proxy should show nothing (no columns selected)
        QCOMPARE(pm.rowCount(), mod.rowCount());
        QCOMPARE(pm.columnCount(), 0);
        QVERIFY(!pm.hasChildren());
    }

    void shouldMapColumns()
    {
        // Given a rearrange-columns proxy
        KRearrangeColumnsProxyModel pm;
        pm.setSourceColumns(QList<int>() << 3 << 1 << 0);

        // When using that proxy on top of an empty source model
        QStandardItemModel sourceModel;
        sourceModel.setColumnCount(4);
        pm.setSourceModel(&sourceModel);

        // Then the mapping methods should work
        QCOMPARE(pm.proxyColumnForSourceColumn(0), 2);
        QCOMPARE(pm.proxyColumnForSourceColumn(1), 1);
        QCOMPARE(pm.proxyColumnForSourceColumn(2), -1);
        QCOMPARE(pm.proxyColumnForSourceColumn(3), 0);
        QCOMPARE(pm.sourceColumnForProxyColumn(0), 3);
        QCOMPARE(pm.sourceColumnForProxyColumn(1), 1);
        QCOMPARE(pm.sourceColumnForProxyColumn(2), 0);

        // And mapFromSource should return invalid for unmapped cells
        QVERIFY(!pm.mapFromSource(sourceModel.index(0, 2)).isValid());
    }

    void shouldShowNothingIfNoRows()
    {
        // Given a rearrange-columns proxy
        KRearrangeColumnsProxyModel pm;
        pm.setSourceColumns(QList<int>() << 2 << 3 << 1 << 0);

        // When using that proxy on top of an empty source model
        QStandardItemModel sourceModel;
        sourceModel.setColumnCount(4);
        pm.setSourceModel(&sourceModel);

        // Then the proxy should show nothing
        QCOMPARE(pm.rowCount(), 0);
        QCOMPARE(pm.columnCount(), 4);
        QCOMPARE(pm.index(0, 0), pm.index(0, 0)); // like QAbstractItemView::setModel does in a Q_ASSERT_X
    }

    void shouldRearrangeColumns()
    {
        // Given a rearrange-columns proxy
        KRearrangeColumnsProxyModel pm;
        new QAbstractItemModelTester(&pm, &pm);

        // When setting it to a source model, with columns rearranged
        setup(pm);

        // Then the proxy should show columns reordered
        QCOMPARE(pm.rowCount(), 2);

        // (verify that the mapFromSource(mapToSource(x)) == x roundtrip works)
        for (int row = 0; row < pm.rowCount(); ++row) {
            for (int col = 0; col < pm.columnCount(); ++col) {
                // qDebug() << "row" << row << "col" << col;
                QCOMPARE(pm.mapFromSource(pm.mapToSource(pm.index(row, col))), pm.index(row, col));
            }
        }
        QCOMPARE(indexRowCol(pm.index(0, 0)), QStringLiteral("0,0"));

        QCOMPARE(pm.rowCount(pm.index(0, 0)), 2);
        QCOMPARE(pm.index(0, 0).parent(), QModelIndex());

        QCOMPARE(pm.mapToSource(pm.index(0, 0)).column(), 2); // column 0 points to C
        QCOMPARE(pm.mapToSource(pm.index(0, 1)).column(), 3); // column 1 points to D

        QCOMPARE(pm.sibling(0, 1, pm.index(0, 0)).column(), 1);
        QCOMPARE(pm.sibling(0, 0, pm.index(0, 1)).column(), 0);

        QCOMPARE(extractRowTexts(&pm, 0), QStringLiteral("CDBA"));
        QCOMPARE(extractRowTexts(&pm, 0, pm.index(0, 0)), QStringLiteral("opnm"));
        QCOMPARE(extractRowTexts(&pm, 1, pm.index(0, 0)), QStringLiteral("strq"));
        QCOMPARE(extractRowTexts(&pm, 1), QStringLiteral("GHFE"));
        QCOMPARE(extractRowTexts(&pm, 0, pm.index(1, 0)), QStringLiteral("z.yx"));
        QCOMPARE(extractHorizontalHeaderTexts(&pm), QStringLiteral("H3H4H2H1"));

        // Verify tree structure of proxy
        const QModelIndex secondParent = pm.index(1, 0);
        QVERIFY(!secondParent.parent().isValid());
        QCOMPARE(indexToText(pm.index(0, 0, secondParent).parent()), indexToText(secondParent));
        QCOMPARE(indexToText(pm.index(0, 3, secondParent).parent()), indexToText(secondParent));

        QVERIFY(!pm.canFetchMore(QModelIndex()));
    }

    void shouldHandleDataChanged()
    {
        // Given a rearrange-columns proxy
        KRearrangeColumnsProxyModel pm;
        setup(pm);

        QSignalSpy dataChangedSpy(&pm, SIGNAL(dataChanged(QModelIndex, QModelIndex)));

        // When a cell in a source model changes
        mod.item(0, 2)->setData("c", Qt::EditRole);
        mod.item(0, 3)->setData("d", Qt::EditRole);

        // Then the change should be notified to the proxy
        QCOMPARE(dataChangedSpy.count(), 2);
        QCOMPARE(indexToText(dataChangedSpy.at(0).at(0).toModelIndex()), indexToText(pm.index(0, 0)));
        QCOMPARE(indexToText(dataChangedSpy.at(1).at(0).toModelIndex()), indexToText(pm.index(0, 1)));
        QCOMPARE(extractRowTexts(&pm, 0), QStringLiteral("cdBA"));
    }

    void shouldHandleDataChangedInChild()
    {
        // Given a rearrange-columns proxy
        KRearrangeColumnsProxyModel pm;
        setup(pm);

        QSignalSpy dataChangedSpy(&pm, SIGNAL(dataChanged(QModelIndex, QModelIndex)));

        // When a cell in a source model changes
        mod.item(1, 0)->child(0, 3)->setData(",", Qt::EditRole);

        // Then the change should be notified to the proxy
        QCOMPARE(dataChangedSpy.count(), 1);
        QCOMPARE(indexToText(dataChangedSpy.at(0).at(0).toModelIndex()), indexToText(pm.index(0, 1, pm.index(1, 0))));
        QCOMPARE(extractRowTexts(&pm, 0, pm.index(1, 0)), QStringLiteral("z,yx"));
    }

    void shouldSupportSetData()
    {
        // Given a rearrange-columns proxy
        KRearrangeColumnsProxyModel pm;
        setup(pm);

        QSignalSpy dataChangedSpy(&pm, SIGNAL(dataChanged(QModelIndex, QModelIndex)));

        // When changing data via the proxy
        const QModelIndex idx = pm.index(0, 2);
        QCOMPARE(idx.data().toString(), QStringLiteral("B"));
        pm.setData(idx, QStringLiteral("Z"));
        QCOMPARE(idx.data().toString(), QStringLiteral("Z"));
        QCOMPARE(extractRowTexts(&pm, 0), QStringLiteral("CDZA"));
        QCOMPARE(extractRowTexts(&mod, 0), QStringLiteral("AZCDE"));
    }

private:
    // setup proxy
    void setup(KRearrangeColumnsProxyModel &pm)
    {
        pm.setSourceColumns(QList<int>() << 2 << 3 << 1 << 0);
        pm.setSourceModel(&mod);
        pm.sort(0); // don't forget this!
    }

    static QString indexRowCol(const QModelIndex &index)
    {
        if (!index.isValid()) {
            return QStringLiteral("invalid");
        }
        return QString::number(index.row()) + "," + QString::number(index.column());
    }

    static QString indexToText(const QModelIndex &index)
    {
        if (!index.isValid()) {
            return QStringLiteral("invalid");
        }
        return QString::number(index.row()) + "," + QString::number(index.column()) + ","
            + QString::number(reinterpret_cast<qulonglong>(index.internalPointer()), 16) + " in "
            + QString::number(reinterpret_cast<qulonglong>(index.model()), 16);
    }

    QStandardItemModel mod;
};

QTEST_MAIN(tst_KRearrangeColumnsProxyModel)

#include "krearrangecolumnsproxymodeltest.moc"
